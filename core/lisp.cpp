/*
 *   Classes concerning the textual represantation of our data.
 *   Copyright (C) 2014 Aaron Puchert
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "lisp.hpp"
#include "logic.hpp"
#include "expression.hpp"
#include <limits>
#include <stdexcept>
using namespace Core;

const std::string& LispToken::getContent() const
{
	if (type == WORD)
		return content;
	else
		throw std::logic_error("Only word tokens have content.");
}


///////////////////////////
// Parser helper classes //
///////////////////////////

Lexer::Lexer(std::istream &input)
	: input(input), last(' '), line_number(1), column_number(0) {}

LispToken Lexer::getToken()
{
	std::string str;

	// Skip any whitespace.
	while (std::isspace(last))
		nextChar();

	// Single-line comment
	if (last == '#') {
		skipLine();
		return getToken();
	}

	// Check for end of file.
	if (input.eof())
		return LispToken(LispToken::ENDOFFILE);

	// Word tokens: [^ \t\r\n()]*. Since we got here, we can assume
	// that we are not at eof and last is neither a space nor '#'.
	if (last != '(' && last != ')') {
		std::string token;
		bool valid;
		do {
			token += last;
			nextChar();
			valid = !std::isspace(last) && last != -1 && last != '('
				&& last != ')' && last != '#';
		} while (valid);

		return LispToken(LispToken::WORD, std::move(token));
	}

	// Otherwise, we will have parantheses
	LispToken::Type type;
	switch (last) {
	case '(':
		type = LispToken::OPENING;
		break;
	case ')':
		type = LispToken::CLOSING;
		break;
	}

	nextChar();
	return LispToken(type);
}

void Lexer::nextChar()
{
	last = input.get();
	if (last == '\n') {
		++line_number;
		column_number = 0;
	}
	else
		++column_number;
}

void Lexer::skipLine()
{
	input.ignore(std::numeric_limits<int>::max(), '\n');
	++line_number;
	column_number = 0;
	nextChar();
}

/**
 * Construct a parser error handler.
 * @param lexer Lexer reference to deliver line and column numbers.
 * @param output Output stream for errors, warnings and notes.
 * @param descriptor String that denotes the input stream, e.g. a file name.
 */
ParserErrorHandler::ParserErrorHandler(const Lexer &lexer, std::ostream &output, const std::string &descriptor)
	: lexer(lexer), descriptor(descriptor), error_count(0), warning_count(0), output(output), writer(output) {}

/**
 * Desctruct ParserErrorHandler and write error and warning count.
 */
ParserErrorHandler::~ParserErrorHandler()
{
	output << "\n\n*** " << descriptor << ": "
		<< error_count << " errors, "
		<< warning_count << " warnings.\n";
}

/**
 * Write new error, warning or note.
 * @param level One of ParserErrorHandler::{ERROR|WARNING|NOTE}.
 */
ParserErrorHandler& ParserErrorHandler::operator <<(Level level)
{
	output << std::endl << descriptor << ':' << lexer.getLine()
		<< ':' << lexer.getColumn() << ':';

	switch(level) {
	case ERROR:
		output << " error: ";
		++error_count;
		break;
	case WARNING:
		output << " warning: ";
		++warning_count;
		break;
	case NOTE:
		output << " note: ";
		break;
	}

	return *this;
}

/**
 * Write string.
 * @param str String.
 */
ParserErrorHandler& ParserErrorHandler::operator <<(const std::string &str)
{
	output << str;

	return *this;
}

/**
 * Write token type.
 * @param type Token type.
 */
ParserErrorHandler& ParserErrorHandler::operator <<(LispToken::Type type)
{
	switch(type) {
	case LispToken::WORD:
		output << "word";
		break;
	case LispToken::OPENING:
		output << "opening paranthesis";
		break;
	case LispToken::CLOSING:
		output << "closing paranthesis";
		break;
	case LispToken::ENDOFFILE:
		output << "end of file";
		break;
	}

	return *this;
}

/**
 * Write type signature.
 * @param type Pointer to type object.
 */
ParserErrorHandler& ParserErrorHandler::operator <<(const_Expr_ptr type)
{
	TypeWriter writer(output);
	writer.write(type.get());

	return *this;
}

//////////////////////////////////
// Implementation of the Parser //
//////////////////////////////////

/**
 * Construct a parser.
 * @param input Stream to read from.
 * @param error_output ParserErrorHandler object to tell problems occured.
 */
Parser::Parser(std::istream& input, std::ostream &output, const std::string &descriptor)
	: lexer(input), error_output(lexer, output, descriptor), token(lexer.getToken()) {}

void Parser::nextToken()
{
	token = lexer.getToken();
}

/**
 * Check if the current token has a certain type.
 * Write an error message if it doesn't.
 * @param type Token type expected.
 * @return True, if the current token is of the expected type.
 */
bool Parser::expect(LispToken::Type type)
{
	if (token.getType() == type)
		return true;
	else {
		error_output << ParserErrorHandler::ERROR << "expected " << type
			<< ", but read " << token.getType();
		return false;
	}
}

/**
 * Add node to current theory.
 * @param node Pointer to node object
 * @return Iterator to the newly inserted node.
 */
Theory::iterator Parser::addNode(Node_ptr node)
{
	Theory::iterator it = iterator_stack.top();
	it = theory_stack.top()->add(node, it);
	iterator_stack.top() = it;
	return it;
}

/**
 * Get the node denoted by the current token.
 * @return Node from a theory or undefined_node, if nothing was found.
 */
const_Node_ptr Parser::getNode()
{
	Theory *theory = theory_stack.top();
	Theory::const_iterator it = theory->get(token.getContent());

	if (it == theory->end()) {
		error_output << ParserErrorHandler::ERROR << "undeclared identifier "
			<< token.getContent();
		return undefined_node;
	}
	else
		return *it;
}

/**
 * Try to recover after an error: skip everything until the next ')'.
 */
void Parser::recover()
{
	while (token.getType() != LispToken::CLOSING)
		nextToken();
	error_output << ParserErrorHandler::NOTE << "ignored everything until ')'";
}

/**
 * Dummy objects used when errors occured.
 */
const Node_ptr Parser::undefined_node = std::make_shared<Node>(BuiltInType::undefined, "");
const Expr_ptr Parser::undefined_expr = std::make_shared<AtomicExpr>(undefined_node);

/**
 * Dispatch table for nodes.
 */
typedef void (Parser::*NodeParser)();
const std::map<std::string, NodeParser> Parser::node_dispatch = {
	{"axiom", &Parser::parseStatement},
	{"lemma", &Parser::parseStatement},
	{"tautology", &Parser::parseTautology},
	{"equivrule", &Parser::parseEquivalenceRule},
	{"deductionrule", &Parser::parseDeductionRule}
};

/**
 * Parses a node and add it to top theory.
 * @method Parser::parseNode
 * @pre The current token is the beginning of a node, the enclosing theory is
 *      top on the stack.
 * @post The current token is the token right after the closing paranthesis.
 */
void Parser::parseNode()
{
	// parse (
	if (!expect(LispToken::OPENING))
		return;
	nextToken();

	// Dispatch
	std::map<std::string, NodeParser>::const_iterator parse_function;
	if (token.getType() == LispToken::WORD &&
		(parse_function = node_dispatch.find(token.getContent()))
			!= node_dispatch.end()) {
		(this->*(parse_function->second))();
	}
	else {
		Node_ptr node;

		// Get Type
		const_Expr_ptr type = parseType();

		// Name
		if (expect(LispToken::WORD))
			node = std::make_shared<Node>(type, token.getContent());
		else
			return;

		// parse Definition, if there is one
		nextToken();
		if (token.getType() != LispToken::CLOSING) {
			Expr_ptr def = parseExpression();
			node->setDefinition(def);
		}

		addNode(node);
	}

	// parse )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();
}

/**
 * Parse a type expression.
 * @method Parser::parseType
 * @return Pointer to a type object.
 * @pre The current token is the first token of a type expression.
 * @post The current token is the token right after the type expression.
 */
const_Expr_ptr Parser::parseType()
{
	const_Expr_ptr type;

	if (token.getType() == LispToken::WORD) {
		// First token = word -> built-in type or variable type
		if (token.getContent() == "type")
			type = BuiltInType::type;
		else if (token.getContent() == "statement")
			type = BuiltInType::statement;
		else {
			const_Node_ptr node = getNode();
			type = std::make_shared<AtomicExpr>(node);
		}

		nextToken();
	}
	else if (token.getType() == LispToken::OPENING) {
		// First token = ( -> lambda-type, i.e. recursive descent
		type = parseLambdaType();
	}
	else {
		error_output << ParserErrorHandler::ERROR
			<< "expected beginning of type expression";
		type = BuiltInType::undefined;
	}

	return type;
}

/**
 * Parse a lambda type expression.
 * @method Parser::parseLambdaType
 * @return Pointer to a type object.
 * @pre The current token is an opening paranthesis.
 * @post The current token is the token right after the type expression.
 */
const_Expr_ptr Parser::parseLambdaType()
{
	nextToken();
	if (token.getType() != LispToken::WORD || token.getContent() != "lambda-type")
		error_output << ParserErrorHandler::ERROR << "expected 'lambda-type'";
	nextToken();

	// Read return type
	const_Expr_ptr return_type = parseType();

	// Read parameter list
	std::vector<const_Expr_ptr> argument_types;

	// '('
	if (expect(LispToken::OPENING)) {
		nextToken();

		// 'list'
		if (expect(LispToken::WORD) && token.getContent() == "list")
			nextToken();

		while (token.getType() != LispToken::CLOSING)
			argument_types.push_back(parseType());

		// ')'
		nextToken();
	}
	else {
		// Try to recover
		recover();
	}

	// ')'
	if (expect(LispToken::CLOSING))
		nextToken();

	// Build type
	return std::make_shared<LambdaType>(std::move(argument_types), return_type);
}

/**
 * Dispatch table for expressions.
 */
const std::map<std::string, Expr_ptr (Parser::*)()> Parser::expr_dispatch = {
	{"not", (&Parser::parseNegationExpr)},
	{"and", (&Parser::parseConnectiveExpr)},
	{"or", (&Parser::parseConnectiveExpr)},
	{"impl", (&Parser::parseConnectiveExpr)},
	{"equiv", (&Parser::parseConnectiveExpr)},
	{"forall", (&Parser::parseQuantifierExpr)},
	{"exists", (&Parser::parseQuantifierExpr)},
	{"lambda", (&Parser::parseLambda)}
};

/**
 * Dispatcher for expressions.
 * @method Parser::parseExpression
 * @return Pointer to an expression.
 * @pre The current token is the beginning of an expression.
 * @post The current token is the token right after the expression.
 */
Expr_ptr Parser::parseExpression()
{
	Expr_ptr expr;

	// Does it start with a paranthesis?
	if (token.getType() == LispToken::OPENING) {
		// Then it is a compound expression
		nextToken();

		if (expect(LispToken::WORD)) {
			// Dispatch
			auto parse_function = expr_dispatch.find(token.getContent());
			if (parse_function != expr_dispatch.end())
				expr = (this->*(parse_function->second))();
			else
				expr = parseLambdaCallExpr();
		}
		else {
			// Try to recover
			recover();
			expr = undefined_expr;
		}
	}
	else if (token.getType() == LispToken::WORD) {
		expr = parseAtomicExpr();
	}
	else {
		error_output << ParserErrorHandler::ERROR
			<< "expected beginning of expression";
		expr = undefined_expr;
	}

	return expr;
}

/**
 * Parse an atomic expression.
 * @method Parser::parseAtomicExpr
 * @return Pointer to an expression.
 * @pre The current token is an atomic expression.
 * @post The current token is the token right after the atomic expression.
 */
Expr_ptr Parser::parseAtomicExpr()
{
	const_Node_ptr node = getNode();
	nextToken();
	return std::make_shared<AtomicExpr>(node);
}

/**
 * Parse a lambda call expression.
 * @method Parser::parseLambdaCallExpr
 * @return Pointer to an expression.
 * @pre The current token is a word token (denoting a lambda node) at the
 *      beginning of a lambda call expression.
 * @post The current token is the token right after the closing paranthesis.
 */
Expr_ptr Parser::parseLambdaCallExpr()
{
	// Must ba a lambda call expression
	const_Node_ptr lambda_node = getNode();
	nextToken();

	// parse arguments
	std::vector<Expr_ptr> args;
	while (token.getType() != LispToken::CLOSING)
		args.push_back(parseExpression());

	// skip ')'
	nextToken();

	// build expression
	return std::make_shared<LambdaCallExpr>(lambda_node, std::move(args));
}

/**
 * Parse a negation expression.
 * @method Parser::parseNegationExpr
 * @return Pointer to an expression
 * @pre The current token is `not`, preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
Expr_ptr Parser::parseNegationExpr()
{
	// Parse next expression
	nextToken();
	Expr_ptr expr = parseExpression();

	// Hope for a )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();

	return std::make_shared<NegationExpr>(expr);
}

/**
 * Dispatch table for connective expressions.
 */
const std::map<std::string, ConnectiveExpr::Variant> Parser::connective_dispatch = {
	{"and", ConnectiveExpr::AND},
	{"or", ConnectiveExpr::OR},
	{"impl", ConnectiveExpr::IMPL},
	{"equiv", ConnectiveExpr::EQUIV},
};

/**
 * Parse a and, or, implication or equivalence expression.
 * @method Parser::parseConnectiveExpr
 * @return Pointer to an expression.
 * @pre The current token is a connective, preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
Expr_ptr Parser::parseConnectiveExpr()
{
	// Which kind of connective?
	auto connective = connective_dispatch.find(token.getContent());
	// By precondition, we have connective != connective_dispatch.end()
	nextToken();

	// Parse both guys
	Expr_ptr expr1, expr2;
	expr1 = parseExpression();
	expr2 = parseExpression();

	// Hope for a )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();

	// Build connective
	return std::make_shared<ConnectiveExpr>(connective->second, expr1, expr2);
}

/**
 * Parse a forall or exists quantifier expression.
 * @method Parser::parseQuantifierExpr
 * @return Pointer to an expression.
 * @pre The current token denotes the quantifier, i.e. either 'forall' or 'exists'.
 * @post The current token is the token right after the closing paranthesis.
 */
Expr_ptr Parser::parseQuantifierExpr()
{
	// Which quantifier?
	QuantifierExpr::Variant variant;
	if (token.getContent() == "forall")
		variant = QuantifierExpr::FORALL;
	else                // == "exists" by precondition
		variant = QuantifierExpr::EXISTS;
	nextToken();

	// Parse predicate
	Expr_ptr expr = parseExpression();

	// Hope for a )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();

	// Build expression
	return std::make_shared<QuantifierExpr>(variant, expr);
}

/**
 * Parse a lambda expression.
 * @method Parser::parseLam
 * @return Pointer to an expression.
 * @pre The current token is the word "lambda", preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
Expr_ptr Parser::parseLambda()
{
	// skip "lambda"
	nextToken();

	// parse parameter list
	if (expect(LispToken::OPENING) && (nextToken(), expect(LispToken::WORD))
			&& token.getContent() == "list")
		nextToken();
	else
		return undefined_expr;
	// TODO: the iterator won't be correct here, but we don't need it anyway.
	Theory params(parseTheory());
	// skip ')'
	nextToken();

	// parse expression
	theory_stack.push(&params);
	Expr_ptr expr = parseExpression();
	theory_stack.pop();

	// Hope for a )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();

	// build
	return std::make_shared<LambdaExpr>(std::move(params), expr);
}

/**
 * Parse a tautology rule and add it to the top theory.
 * @method Parser::parseTautology
 * @pre The current token is the word "tautology", preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
void Parser::parseTautology()
{
	// skip 'tautology'
	nextToken();

	// parse name
	if (!expect(LispToken::WORD)) {
		recover();
		return;
	}
	std::string name = token.getContent();
	nextToken();

	// parse parameters
	if (expect(LispToken::OPENING)) {
		nextToken();
		// next should be "list"
		if (expect(LispToken::WORD) && token.getContent() == "list")
			nextToken();
		else {
			recover();
			return;
		}
	}
	else
		return;

	Theory params(parseTheory(true));
	// skip ')'
	nextToken();

	// parse expression
	theory_stack.push(&params);
	Expr_ptr expr = parseExpression();
	theory_stack.pop();

	// build
	Node_ptr tautology = std::make_shared<Tautology>(name, std::move(params), expr);
	addNode(tautology);
}

/**
 * Parse a equivalence rule and add it to the top theory.
 * @method Parser::parseEquivalenceRule
 * @pre The current token is the word "equivrule", preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
void Parser::parseEquivalenceRule()
{
	// skip 'equivrule'
	nextToken();

	// parse name
	if (!expect(LispToken::WORD)) {
		recover();
		return;
	}
	std::string name = token.getContent();
	nextToken();

	// parse parameters
	if (expect(LispToken::OPENING)) {
		nextToken();
		// next should be "list"
		if (expect(LispToken::WORD) && token.getContent() == "list")
			nextToken();
		else {
			recover();
			return;
		}
	}
	else
		return;

	Theory params(parseTheory(true));
	// skip ')'
	nextToken();

	// parse expression
	theory_stack.push(&params);
	Expr_ptr expr1, expr2;
	expr1 = parseExpression();
	expr2 = parseExpression();
	theory_stack.pop();

	// build
	Node_ptr node = std::make_shared<EquivalenceRule>(name, std::move(params), expr1, expr2);
	addNode(node);
}

/**
 * Parse a deduction rule and add it to the top theory.
 * @method Parser::parseDeductionRule
 * @pre The current token is the word "deductionrule", preceded by an opening paranthesis.
 * @post The current token is the token right after the closing paranthesis.
 */
void Parser::parseDeductionRule()
{
	// skip 'deductionrule'
	nextToken();

	// parse name
	if (!expect(LispToken::WORD)) {
		recover();
		return;
	}
	std::string name = token.getContent();
	nextToken();

	// parse parameters
	if (expect(LispToken::OPENING)) {
		nextToken();
		// next should be "list"
		if (expect(LispToken::WORD) && token.getContent() == "list")
			nextToken();
		else {
			recover();
			return;
		}
	}
	else
		return;

	Theory params(parseTheory(true));
	// skip ')'
	nextToken();

	// parse premisses
	theory_stack.push(&params);
	std::vector<Expr_ptr> premisses;
	if (expect(LispToken::OPENING)) {
		nextToken();
		if (expect(LispToken::WORD) && token.getContent() == "list") {
			nextToken();
			while (token.getType() != LispToken::CLOSING)
				premisses.push_back(parseExpression());
			// skip ')'
			nextToken();
		}
		else
			recover();
	}
	// else: interpret as missing premisses list.

	// parse conclusion
	Expr_ptr conclusion = parseExpression();
	theory_stack.pop();

	// build
	Node_ptr node = std::make_shared<DeductionRule>(name, std::move(params), premisses, conclusion);
	addNode(node);
}

/**
 * Parse a statement and add it to the top theory.
 * @method Parser::parseStatement
 * @pre The current token is either "axiom" or "lemma", preceded by an opening
 *      paranthesis.
 * @post The current token is the token right after the last token of the content.
 */
void Parser::parseStatement()
{
	// Axiom or lemma?
	bool expect_proof = (token.getContent() != "axiom");
	nextToken();

	// Name
	std::string name;
	if (token.getType() == LispToken::WORD) {
		name = token.getContent();
		nextToken();
	}

	// Parse expression and build statement
	Expr_ptr expr = parseExpression();
	Statement_ptr stmt = std::make_shared<Statement>(name, expr);
	addNode(stmt);

	// Parse proof
	if (expect_proof) {
		Proof_ptr proof = parseProofStep();
		stmt->addProof(proof);
	}
}

/**
 * Parse a proof step.
 * @method Parser::parseProofStep
 * @return Pointer to proof
 * @pre The current token is the beginning of a proof step.
 * @post The current token is the token right after the closing paranthesis.
 */
Proof_ptr Parser::parseProofStep()
{
	// parse '('
	if (!expect(LispToken::OPENING))
		return Proof_ptr();
	nextToken();

	// get name of rule
	if (!expect(LispToken::WORD))
		return Proof_ptr();
	std::string rule_name = token.getContent();
	nextToken();

	// parse expression list
	std::vector<Expr_ptr> var_list;
	if (expect(LispToken::OPENING)) {
		nextToken();
		if (expect(LispToken::WORD) && token.getContent() == "list") {
			nextToken();
			while (token.getType() != LispToken::CLOSING)
				var_list.push_back(parseExpression());
			// skip ')'
			nextToken();
		}
		else
			recover();
	}
	// else: interpret as missing expression list.

	// parse reference list
	std::vector<Reference> references;
	if (expect(LispToken::OPENING)) {
		nextToken();
		if (expect(LispToken::WORD) && token.getContent() == "list") {
			nextToken();
			while (token.getType() != LispToken::CLOSING)
				references.push_back(parseReference());
			// skip ')'
			nextToken();
		}
		else
			recover();
	}
	// else: interpret as missing references list.

	// parse )
	if (expect(LispToken::CLOSING))
		nextToken();
	else
		recover();

	return std::make_shared<ProofStep>(rules, rule_name, std::move(var_list),
		std::move(references));
}

/**
 * Parse a reference.
 * @method Parser::parseReference
 * @return Reference object
 * @pre The current token is a reference token.
 * @post The current token is the token right after the reference.
 */
Reference Parser::parseReference()
{
	if (expect(LispToken::WORD)) {
		Reference ref(theory_stack.top(), iterator_stack.top(), token.getContent());
		nextToken();
		return ref;
	}
	else
		return Reference(nullptr, Theory::iterator());
}

/**
 * Parse a theory.
 * @method Parser::parseTheory
 * @param standalone If set to true, the theory won't point to it's parent.
 * @return Theory object.
 * @pre The current token is the beginning of the first node of the theory.
 * @post The current token is the token right after the last node of the theory.
 */
Theory Parser::parseTheory(bool standalone)
{
	Theory *parent = nullptr;
	Theory::iterator default_it;

	// If we have something on the stack, it's going to be our parent.
	if (!standalone && theory_stack.size() && iterator_stack.size()) {
		parent = theory_stack.top();
		default_it = iterator_stack.top();
	}

	Theory theory(parent, default_it);

	theory_stack.push(&theory);
	iterator_stack.push(theory.begin());

	while (token.getType() != LispToken::CLOSING &&
			token.getType() != LispToken::ENDOFFILE)
		parseNode();

	iterator_stack.pop();
	theory_stack.pop();

	return theory;
}


//////////////////////////////////
// Implementation of the Writer //
//////////////////////////////////

Writer::Writer(std::ostream &output, int line_length, int tab_size, bool tabs)
	: output(output), depth(0), max_line_length(line_length), line_length(0),
	  tab_size(tab_size), tabs(tabs), write_depth(0) {}

Writer::~Writer()
{
	// flush buffer?
	writeQueue();

	// We shouldn't throw an exception here, nevertheless check depth.
	if (depth != 0)
		output << "Error: unbalanced parantheses!\n";
}

void Writer::visit(const BuiltInType *type)
{
	switch (type->variant) {
	case BuiltInType::TYPE:
		addToken("type");
		break;
	case BuiltInType::STATEMENT:
		addToken("statement");
		break;
	case BuiltInType::RULE:
		addToken("rule");
		break;
	default:
		addToken("undefined");
		break;
	}
}

void Writer::visit(const LambdaType *type)
{
	addParanthesis(OPENING);
	addToken("lambda-type");
	type->getReturnType()->accept(this);
	addParanthesis(OPENING);
	addToken("list");
	for (const_Expr_ptr arg_type : *type)
		arg_type->accept(this);
	addParanthesis(CLOSING);
	addParanthesis(CLOSING);
}

void Writer::visit(const Node *node)
{
	addParanthesis(OPENING);
	node->getType()->accept(this);
	addToken(node->getName());
	if (const_Expr_ptr expr = node->getDefinition())
		expr->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const LambdaExpr *expression)
{
	addParanthesis(OPENING);
	addToken("lambda");
	// Declaration list
	addParanthesis(OPENING);
	addToken("list");
	expression->getParams().accept(this);
	addParanthesis(CLOSING);
	if (const_Expr_ptr expr = expression->getDefinition())
		expr->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const AtomicExpr *expression)
{
	addToken(expression->getAtom()->getName());
}

void Writer::visit(const LambdaCallExpr *expression)
{
	addParanthesis(OPENING);
	addToken(expression->getLambda()->getName());
	for (auto arg : *expression)
		arg->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const NegationExpr *expression)
{
	addParanthesis(OPENING);
	addToken("not");
	expression->getExpr()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const ConnectiveExpr *expression)
{
	addParanthesis(OPENING);
	switch (expression->getVariant()) {
		case ConnectiveExpr::AND:
			addToken("and");
			break;
		case ConnectiveExpr::OR:
			addToken("or");
			break;
		case ConnectiveExpr::IMPL:
			addToken("impl");
			break;
		case ConnectiveExpr::EQUIV:
			addToken("equiv");
			break;
	}
	expression->getFirstExpr()->accept(this);
	expression->getSecondExpr()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const QuantifierExpr *expression)
{
	addParanthesis(OPENING);
	switch (expression->getVariant()) {
	case QuantifierExpr::EXISTS:
		addToken("exists");
		break;
	case QuantifierExpr::FORALL:
		addToken("forall");
		break;
	}
	expression->getPredicate()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::write_varlist(const Rule* rule)
{
	addParanthesis(OPENING);
	addToken("list");
	rule->getVars()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const Tautology *rule)
{
	addParanthesis(OPENING);
	addToken("tautology");
	addToken(rule->getName());
	write_varlist(rule);
	rule->getStatement()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const EquivalenceRule *rule)
{
	addParanthesis(OPENING);
	addToken("equivrule");
	addToken(rule->getName());
	write_varlist(rule);
	rule->getStatement1()->accept(this);
	rule->getStatement2()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const DeductionRule *rule)
{
	addParanthesis(OPENING);
	addToken("deductionrule");
	addToken(rule->getName());
	write_varlist(rule);
	addParanthesis(OPENING);
	addToken("list");
	for (const_Expr_ptr expr : rule->getPremisses())
		expr->accept(this);
	addParanthesis(CLOSING);
	rule->getConclusion()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const Statement *statement)
{
	addParanthesis(OPENING);
	if (statement->hasProof())
		addToken("lemma");
	else
		addToken("axiom");
	statement->getDefinition()->accept(this);
	if (statement->hasProof())
		statement->getProof()->accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const Reference *reference)
{
	addToken(reference->getDescription(theory_stack.top(), iterator_stack.top()));
}

void Writer::visit(const ProofStep *proofstep)
{
	addParanthesis(OPENING);
	addToken(proofstep->getRule()->getName());
	addParanthesis(OPENING);
	addToken("list");
	for (Expr_ptr expr : proofstep->getVars())
		expr->accept(this);
	addParanthesis(CLOSING);
	addParanthesis(OPENING);
	addToken("list");
	for (const Reference &ref : proofstep->getReferences())
		ref.accept(this);
	addParanthesis(CLOSING);
	addParanthesis(CLOSING);
}

void Writer::visit(const Theory *theory)
{
	theory_stack.push(theory);

	for (Theory::const_iterator it = theory->begin(); it != theory->end(); ++it) {
		iterator_stack.push(it);
		(*it)->accept(this);
		iterator_stack.pop();
	}

	theory_stack.pop();
}

/**
 * Add paranthesis token.
 * @method Writer::addParanthesis
 * @param depth_change One of LispToken::{OPENING¦CLOSING}
 */
void Writer::addParanthesis(Change depth_change)
{
	depth += (int)depth_change;

	if (depth_change == OPENING)
		push(LispToken(LispToken::OPENING));
	else	// CLOSING
		push(LispToken(LispToken::CLOSING));

	// If we are at level 0 or have enough material, write something.
	if (depth == 0 || line_length > 2*max_line_length)
		writeQueue();
}

/**
 * Add a token string.
 * @method Writer::addToken
 * @param token Token string.
 */
void Writer::addToken(const std::string &token)
{
	push(LispToken(LispToken::WORD, token));
}

/**
 * Add a token string we can eat up.
 * @method Writer::addToken
 * @param token Rvalue reference to token.
 */
void Writer::addToken(std::string &&token)
{
	push(LispToken(LispToken::WORD, std::move(token)));
}

/**
 * Push a LispToken, called by addToken and addParanthesis.
 * @method Writer::push
 * @param token Rvalue reference to a LispToken
 */
void Writer::push(LispToken &&token)
{
	// Add token to queue
	token_queue.push_back(std::move(token));

	// Add length of preceding token. This means we haven't accounted for the
	// last item in queue. The last item will only be flushed if we go back to
	// level 0, so this is a closing paranthesis.
	// However, in writeLine we don't account for the last item as well, hence
	// it won't hurt our character count.
	if (token_queue.size() >= 2)
		line_length += tokenLength(token_queue.size() - 2);
}

/**
 * Write some tokens from the queue.
 * We will stop at level 0, when the queue is empty, or, when in between, when
 * there is material for just one line in it. Then we allow to refill, since we
 * always need at least a full line to decide where to put line breaks.
 * @method Writer::writeQueue
 */
void Writer::writeQueue()
{
	while ((depth == 0 && token_queue.size()) ||
			(depth != 0 && line_length > max_line_length)) {
		LispToken token(token_queue.front());

		// Current line length and index of next token
		int length = tab_size * write_depth;
		int index = 1;

		switch (token.getType()) {
		case LispToken::OPENING:
			// Then count characters until ')'
			for (int cur_depth = 1; cur_depth != 0 && length <= max_line_length; ++index) {
				LispToken::Type type = token_queue[index].getType();
				if (type == LispToken::OPENING)
					cur_depth += 1;
				else if (type == LispToken::CLOSING)
					cur_depth -= 1;

				length += tokenLength(index);
			}

			// Does it fit on the line? Then write.
			if (length <= max_line_length)
				writeLine(index);
			else {
				writeLine(token_queue[1].getType() == LispToken::OPENING ? 1 : 2);

				// The closing paranthesis will be on an extra line,
				// hence we don't have to care about it. (*)
				--line_length;
				++write_depth;
			}

			break;

		// Closing paranthesis? Decrease depth, write it.
		case LispToken::CLOSING:
			++line_length;	     // compensate for forgetting in (*)
			--write_depth;
			// no break;

		// Single word? Just write it.
		case LispToken::WORD:
			writeLine(1);
			break;
		}
	}
}

/**
 * Write a complete line, using a certain number of tokens from the queue.
 * The line is indented as write_depth says, and we write the tokens with
 * indices 0 to num_tokens-1.
 * @method Writer::writeLine
 * @param num_tokens Number of tokens to write
 */
void Writer::writeLine(int num_tokens)
{
	// Indent
	if (tabs)
		for (int indent = 0; indent < write_depth; ++indent)
			output << '\t';
	else
		for (int indent = 0; indent < tab_size * write_depth; ++indent)
			output << ' ';

	// Write tokens
	while (num_tokens--) {
		LispToken token(token_queue.front());

		// Write token and compensate in line length
		writeToken(token);
		// We don't compensate for the last token in the queue, see push.
		if (token_queue.size() > 1)
			line_length -= tokenLength(0);

		token_queue.pop_front();

		// Space after token if it isn't '(' and the next token isn't ')'.
		// Also, no space after the last token in a line.
		if (token.getType() != LispToken::OPENING && token_queue.size() &&
				token_queue.front().getType() != LispToken::CLOSING && num_tokens > 1)
			output << ' ';
	}

	// New line
	output << std::endl;
}

/**
 * Write a single token.
 * @method Writer::writeToken
 * @param token Token to write
 */
void Writer::writeToken(LispToken token)
{
	switch (token.getType()) {
	case LispToken::WORD:
		output << token.getContent();
		break;
	case LispToken::OPENING:
		output << '(';
		break;
	case LispToken::CLOSING:
		output << ')';
		break;
	}
}

/**
 * Compute the length of a token in the queue, accounting for space after it.
 * This assumes that all tokens will form a single line of output. We don't
 * concern ourselves with the case that a token might be at the end of a line.
 * We can only compute the length of tokens that are not at the end.
 * @method Writer::tokenLength
 * @param index Index of token in the queue
 * @return Length of token including a space after it.
 */
int Writer::tokenLength(int index)
{
	int length = 0;

	switch (token_queue[index].getType()) {
	case LispToken::WORD:
		length += token_queue[index].getContent().length();
		break;
	case LispToken::OPENING:
		length += 1;
		break;
	case LispToken::CLOSING:
		length += 1;
		break;
	}

	// Account for space after token.
	// assert(token_queue.size() > index+1);
	if (token_queue[index].getType() != LispToken::OPENING &&
			token_queue[index+1].getType() != LispToken::CLOSING)
		length += 1;

	return length;
}
