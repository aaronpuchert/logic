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
#include <map>

/**
 * Namespace for logic core
 */
namespace Core {
	// base.hpp
	class Expression;
	typedef std::shared_ptr<Expression> Expr_ptr;
	typedef std::shared_ptr<const Expression> const_Expr_ptr;
	class BuiltInType;
	class LambdaType;
	class Node;
	typedef std::shared_ptr<Node> Node_ptr;
	typedef std::shared_ptr<const Node> const_Node_ptr;
	class SearchNode;
	typedef std::map<const_Node_ptr, const_Expr_ptr> Context;

	// expression.hpp
	class AtomicExpr;
	class LambdaCallExpr;
	class NegationExpr;
	class ConnectiveExpr;
	class QuantifierExpr;
	class LambdaExpr;

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
