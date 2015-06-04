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
		/**
		 * Construct atomic expression.
		 *
		 * @param node Node the expression should point to.
		 */
		AtomicExpr(const_Node_ptr node)
			: Expression(Expression::ATOMIC), node(node) {}

		/**
		 * Get corresponding node.
		 *
		 * @return Node the expression refers to.
		 */
		const_Node_ptr getAtom() const
			{return node;}

		const_Expr_ptr getType() const;
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Node_ptr node;
	};

	/**
	 * Lambda call expression
	 */
	class LambdaCallExpr : public Expression {
	public:
		LambdaCallExpr(const_Node_ptr node, std::vector<Expr_ptr> &&args);

		/**
		 * Get the lambda node that is called.
		 *
		 * @return Lambda node.
		 */
		const_Node_ptr getLambda() const
			{return node;}

		const_Expr_ptr getType() const;

		/**
		 * Iterator for parameters.
		 */
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

		/**
		 * Get negated expression.
		 *
		 * @return Negated expression.
		 */
		Expr_ptr getExpr() const {return expr;}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}
		const_Expr_ptr getType() const;

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

		/**
		 * Get variant of connective expression.
		 *
		 * @return One of ConnectiveExpr::{AND|OR|IMPL|EQUIV}.
		 */
		Variant getVariant() const {return variant;}

		/**
		 * Get first subexpression of the connective.
		 *
		 * @return First expression.
		 */
		Expr_ptr getFirstExpr() const {return expr[0];}

		/**
		 * Get second subexpression of the connective.
		 *
		 * @return Second expression.
		 */
		Expr_ptr getSecondExpr() const {return expr[1];}

		const_Expr_ptr getType() const;

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

		/**
		 * Is this an universal or existential quantification?
		 *
		 * @return One of QuantifierExpr::{EXISTS|FORALL}
		 */
		Variant getVariant() const
			{return variant;}

		/**
		 * Get predicate expression over which is quantified.
		 *
		 * @return Predicate expression.
		 */
		const_Expr_ptr getPredicate() const
			{return predicate;}

		const_Expr_ptr getType() const;
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Variant variant;
		const_Expr_ptr predicate;
	};

	/**
	 * Logical syntactical sugar
	 */
	// class LongConnective : public Expression {};

	/**
	 * Lambda expressions.
	 */
	class LambdaExpr : public Expression {
	public:
		LambdaExpr(std::vector<Node_ptr> &&params, const_Expr_ptr expression);
		const std::vector<Node_ptr>& getParams() const
			{return params;}
		const_Expr_ptr getDefinition() const
			{return expression;}
		void setDefinition(const_Expr_ptr new_expression);
		const_Expr_ptr getType() const;

		// Iterating through arguments
		std::vector<Node_ptr>::const_iterator begin() const;
		std::vector<Node_ptr>::const_iterator end() const;

		void accept(Visitor *visitor) const;

	private:
		std::vector<Node_ptr> params;
		mutable const_Expr_ptr type;
		const_Expr_ptr expression;
	};
}	// End of namespace Core

#endif
