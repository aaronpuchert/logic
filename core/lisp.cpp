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

std::string LispToken::getContent() const
{
	if (type == WORD)
		return content;
	else
		throw std::logic_error("Only word tokens have content.");
}

/**
 * Implementation of the writer
 */
Writer::Writer(std::ostream &output)
	: output(output), depth(0) {}

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
	addToken("lambda");
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

void Writer::visit(const PredicateLambda *predicate)
{
	// Declaration list
	addParanthesis(OPENING);
	addToken("list");
	predicate->getParams().accept(this);
	addParanthesis(CLOSING);
	if (const_Expr_ptr expr = predicate->getDefinition())
		expr->accept(this);
}

void Writer::visit(const AtomicExpr *expression)
{
	addToken(expression->getAtom()->getName());
}

void Writer::visit(const PredicateExpr *expression)
{
	addParanthesis(OPENING);
	addToken(expression->getPredicate()->getName());
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
	for (const_Expr_ptr expr : rule->prem())
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
	statement->getStatement()->accept(this);
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

void Writer::addParanthesis(Change depth_change)
{
	depth += (int)depth_change;

	if (depth_change == OPENING)
		token_queue.push(LispToken(LispToken::OPENING));
	else	// CLOSING
		token_queue.push(LispToken(LispToken::CLOSING));

	// if conditions are met, write contents
	if (depth == 0)
		writeQueue();
}

void Writer::addToken(const std::string &token)
{
	token_queue.push(LispToken(LispToken::WORD, token));
}

void Writer::addToken(std::string &&token)
{
	token_queue.push(LispToken(LispToken::WORD, std::move(token)));
}

void Writer::writeQueue()
{
	while (token_queue.size()) {
		LispToken token(token_queue.front());
		token_queue.pop();

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
		case LispToken::ENDOFFILE:		// To avoid warning.
			break;
		}

		if (token.getType() != LispToken::OPENING && token_queue.size() &&
				token_queue.front().getType() != LispToken::CLOSING)
			output << ' ';
	};

	output << std::endl;
}
