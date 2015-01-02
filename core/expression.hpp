/*
 *   Data structures for expressions such as variables and predicates.
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

#ifndef CORE_EXPRESSION_HPP
#define CORE_EXPRESSION_HPP
#include "forward.hpp"
#include "base.hpp"
#include "theory.hpp"
#include <vector>
#include <memory>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Atomic expression.
	 */
	class AtomicExpr : public Expression {
	public:
		AtomicExpr(const_Node_ptr node)
			: node(node) {}
		const_Node_ptr getAtom() const
			{return node;}
		const_Type_ptr getType() const;
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Node_ptr node;
	};

	/**
	 * Predicate expression
	 */
	class PredicateExpr : public Expression {
	public:
		PredicateExpr(const_Node_ptr node, std::vector<Expr_ptr> &&args);
		const_Node_ptr getPredicate() const
			{return node;}
		const_Type_ptr getType() const;

		// Iteration
		typedef std::vector<Expr_ptr>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Node_ptr node;
		std::vector<Expr_ptr> args;
	};

	/**
	 * Negation expression
	 */
	class NegationExpr : public Expression {
	public:
		NegationExpr(Expr_ptr expr);
		Expr_ptr getExpr() const {return expr;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
		const_Type_ptr getType() const;

	private:
		Expr_ptr expr;
	};

	/**
	 * Classical binary expressions: and, or, implication and equivalence
	 */
	class ConnectiveExpr : public Expression {
	public:
		enum Variant {AND, OR, IMPL, EQUIV};
		ConnectiveExpr(Variant variant, Expr_ptr first, Expr_ptr second);
		Variant getVariant() const {return variant;}
		Expr_ptr getFirstExpr() const {return expr[0];}
		Expr_ptr getSecondExpr() const {return expr[1];}
		const_Type_ptr getType() const;

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Variant variant;
		Expr_ptr expr[2];
	};

	/**
	 * Quantifier expressions
	 */
	class QuantifierExpr : public Expression {
	public:
		enum Variant {EXISTS, FORALL};
		QuantifierExpr(Variant variant, const_Expr_ptr predicate);
		Variant getVariant() const
			{return variant;}
		const_Expr_ptr getPredicate() const
			{return predicate;}
		const_Type_ptr getType() const;

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Variant variant;
		const_Expr_ptr predicate;
	};

	/**
	 * Logical syntactical sugar
	 */
	// class LongImplication : public Expression {};
	// class LongConjunction : public Expression {};

	/**
	 * Predicate lambda expressions.
	 */
	class PredicateLambda : public Expression {
	public:
		PredicateLambda(Theory &&params, const_Expr_ptr expression);
		const Theory& getParams() const
			{return params;}
		const_Expr_ptr getDefinition() const
			{return expression;}
		void setDefinition(const_Expr_ptr new_expression);
		const_Type_ptr getType() const;

		// Iterating through arguments
		Theory::const_iterator begin() const;
		Theory::const_iterator end() const;

		void accept(Visitor *visitor) const;

	private:
		Theory params;
		mutable const_Type_ptr type;
		const_Expr_ptr expression;
	};

	/**
	 * Functions, relations?
	 */
}	// End of namespace Core

#endif
