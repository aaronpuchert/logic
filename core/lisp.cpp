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
#include <limits>
using namespace Core;

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

void Writer::visit(const Type *type)
{
	addParanthesis(OPENING);
	addToken("type");
	addToken(type->getName());
	addParanthesis(CLOSING);
}

void Writer::visit(const Variable *variable)
{
	addParanthesis(OPENING);
	addToken(variable->getType()->getName());
	addToken(variable->getName());
	addParanthesis(CLOSING);
}

void Writer::visit(const PredicateDecl *predicate)
{
	addParanthesis(OPENING);
	addToken("predicate");
	addToken(predicate->getName());
	addParanthesis(OPENING);
	addToken("list");
	for (int i=0; i<predicate->getValency(); ++i)
		addToken(predicate->getParameterType(i)->getName());
	addParanthesis(CLOSING);
	addParanthesis(CLOSING);
}

void Writer::visit(const PredicateLambda *predicate)
{
	// Declaration list
	addParanthesis(OPENING);
	addToken("list");
	for (int i=0; i<predicate->getValency(); ++i)
		predicate->getDeclaration(i).accept(this);
	addParanthesis(CLOSING);
	// Statement
	predicate->getDefinition()->accept(this);
}

void Writer::visit(const PredicateDef *predicate)
{
	addParanthesis(OPENING);
	addToken("predicate");
	predicate->getDefinition().accept(this);
	addParanthesis(CLOSING);
}

void Writer::visit(const AtomicExpr *expression)
{
	addToken(expression->getAtom()->getName());
}

void Writer::visit(const PredicateExpr *expression)
{
	addParanthesis(OPENING);
	addToken(expression->getPredicate()->getName());
	for (int i=0; i<expression->getPredicate()->getValency(); ++i)
		expression->getArg(i)->accept(this);
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
	switch (expression->getType()) {
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
	switch (expression->getType()) {
	case QuantifierExpr::EXISTS:
		addToken("exists");
		break;
	case QuantifierExpr::FORALL:
		addToken("forall");
		break;
	}
	expression->getPredicateLambda().accept(this);
	addParanthesis(CLOSING);
}

void Writer::write_varlist(const Rule* rule)
{
	addParanthesis(OPENING);
	addToken("list");
	for (const Variable &var: rule->getVars())
		var.accept(this);
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
	switch (statement->getType()) {
	case Statement::AXIOM:
		addToken("axiom");
		break;
	case Statement::STATEMENT:
		addToken("statement");
		break;
	};
	statement->getStatement()->accept(this);
	if (statement->getType() == Statement::STATEMENT)
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
		addToken("(");
	else	// CLOSING
		addToken(")");

	// if conditions are met, write contents
	if (depth == 0)
		writeQueue();
}

void Writer::addToken(const std::string &token)
{
	token_queue.push(token);
}

void Writer::addToken(std::string &&token)
{
	token_queue.push(token);
}

void Writer::writeQueue()
{
	while (token_queue.size()) {
		std::string token(token_queue.front());
		token_queue.pop();
		output << token;
		if (token != "(" && token_queue.size() && token_queue.front() != ")")
			output << " ";
	};

	output << std::endl;
}
