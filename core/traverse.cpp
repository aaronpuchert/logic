/*
 *   Visitor infrastructure for traversing the (syntax?) tree.
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

#include "traverse.hpp"
using namespace Core;

/**
 * Default implementations for Visitor functions.
 */
void Visitor::visit(const Type *type)
{
	//
}

void Visitor::visit(const Variable *variable)
{
	//
}

void Visitor::visit(const PredicateDecl *predicate)
{
	//
}

void Visitor::visit(const PredicateLambda *predicate)
{
	//
}

void Visitor::visit(const PredicateDef *predicate)
{
	//
}

void Visitor::visit(const AtomicExpr *expression)
{
	//
}

void Visitor::visit(const PredicateExpr *expression)
{
	//
}

void Visitor::visit(const NegationExpr *expression)
{
	//
}

void Visitor::visit(const ConnectiveExpr *expression)
{
	//
}

void Visitor::visit(const QuantifierExpr *expression)
{
	//
}

void Visitor::visit(const Tautology *rule)
{
	//
}

void Visitor::visit(const EquivalenceRule *rule)
{
	//
}

void Visitor::visit(const DeductionRule *rule)
{
	//
}

void Visitor::visit(const Statement *statement)
{
	//
}

void Visitor::visit(const Reference *reference)
{
	//
}

void Visitor::visit(const ProofStep *proofstep)
{
	//
}

void Visitor::visit(const LongProof *longproof)
{
	//
}

void Visitor::visit(const Theory *theory)
{
	//
}
