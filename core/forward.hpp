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

#pragma once
#include <memory>

/**
 * Namespace for logic core
 */
namespace Core {
	// type.hpp
	class Node;
	typedef std::shared_ptr<Node> Node_ptr;
	typedef std::shared_ptr<const Node> const_Node_ptr;
	class Type;
	typedef std::shared_ptr<Type> Type_ptr;
	typedef std::shared_ptr<const Type> const_Type_ptr;

	// atom.hpp
	class Variable;
	typedef std::shared_ptr<Variable> Var_ptr;
	typedef std::shared_ptr<const Variable> const_Var_ptr;

	// expression.hpp
	class Predicate;
	typedef std::shared_ptr<Predicate> Pred_ptr;
	typedef std::shared_ptr<const Predicate> const_Pred_ptr;
	class PredicateDecl;
	class PredicateLambda;
	class PredicateDef;
	class Expression;
	typedef std::shared_ptr<Expression> Expr_ptr;
	typedef std::shared_ptr<const Expression> const_Expr_ptr;
	class AtomicExpr;
	class PredicateExpr;
	class NegationExpr;
	class ConnectiveExpr;
	class QuantifierExpr;

	// logic.hpp
	class Rule;
	class Tautology;
	class EquivalenceRule;
	class DeductionRule;

	// theory.hpp
	class Theory;
	typedef std::shared_ptr<Theory> Theory_ptr;
	typedef std::shared_ptr<const Theory> const_Theory_ptr;
	class Statement;
	class Proof;
	typedef std::shared_ptr<Proof> Proof_ptr;
	typedef std::shared_ptr<const Proof> const_Proof_ptr;
	class ProofStep;
	class LongProof;

	// namespace.hpp
	template<typename T> class Namespace;
	typedef Namespace<Rule> LogicSystem;

	// traverse.hpp
	class Visitor;
}	// End of namespace Core
