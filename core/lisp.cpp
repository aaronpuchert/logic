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

#include "theory.hpp"
#include "logic.hpp"
#include "lisp.hpp"
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

/**
 * Implementation of the writer
 */
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
		addToken("statement");   // TODO: that's actually not so easy.
		break;
	case BuiltInType::RULE:
		addToken("rule");        // TODO: not so easy as well.
		break;
	default:
		addToken("undefined");
		break;
	}
}

void Writer::visit(const VariableType *type)
{
	addToken(type->getName());
}

void Writer::visit(const LambdaType *type)
{
	addParanthesis(OPENING);
	addToken("lambda-type");
	type->getReturnType()->accept(this);
	addParanthesis(OPENING);
	addToken("list");
	for (const_Type_ptr arg_type : *type)
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

void Writer::visit(const LongProof *longproof)
{
	addParanthesis(OPENING);
	addToken("proof");
	longproof->subTheory.accept(this);
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
