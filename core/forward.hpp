/*
 *   Forward declaration of several classes.
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

#ifndef CORE_FORWARD_HPP
#define CORE_FORWARD_HPP
#include <memory>
#include <utility>

/**
 * Namespace for logic core
 */
namespace Core {
	// base.hpp
	class Expression;
	typedef std::shared_ptr<Expression> Expr_ptr;
	typedef std::shared_ptr<const Expression> const_Expr_ptr;
	class Type;
	typedef std::shared_ptr<Type> Type_ptr;
	typedef std::shared_ptr<const Type> const_Type_ptr;
	class BuiltInType;
	class VariableType;
	class Node;
	typedef std::shared_ptr<Node> Node_ptr;
	typedef std::shared_ptr<const Node> const_Node_ptr;
	class SearchNode;

	// expression.hpp
	class AtomicExpr;
	class PredicateExpr;
	class NegationExpr;
	class ConnectiveExpr;
	class QuantifierExpr;
	class PredicateLambda;

	// logic.hpp
	class Rule;
	typedef std::shared_ptr<Rule> Rule_ptr;
	typedef std::shared_ptr<const Rule> const_Rule_ptr;
	class Tautology;
	class EquivalenceRule;
	class DeductionRule;

	// theory.hpp
	class Theory;
	class Statement;
	typedef std::shared_ptr<Statement> Statement_ptr;
	typedef std::shared_ptr<const Statement> const_Statement_ptr;
	class Reference;
	class Proof;
	typedef std::shared_ptr<Proof> Proof_ptr;
	typedef std::shared_ptr<const Proof> const_Proof_ptr;
	class ProofStep;
	class LongProof;

	// traverse.hpp
	class Visitor;
}	// End of namespace Core

#endif
