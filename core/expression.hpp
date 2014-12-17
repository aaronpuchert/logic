/*
 *   Data structures for expressions.
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
#include "atom.hpp"
#include <vector>
#include <memory>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract class for expressions.
	 */
	class Expression {
	public:
		virtual void accept(Visitor *visitor) const = 0;
	};

	/**
	 * Atomic expression.
	 */
	class AtomicExpr : public Expression {
	public:
		AtomicExpr(const_Var_ptr var)
			: var(var) {}
		const_Var_ptr getAtom() const
			{return var;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Var_ptr var;
	};

	/**
	 * Predicate expression
	 */
	class PredicateExpr : public Expression {
	public:
		PredicateExpr(const_Pred_ptr pred, std::vector<Expr_ptr> &&args)
			: pred(pred), args(std::move(args)) {}
		const_Pred_ptr getPredicate() const
			{return pred;}
		const_Expr_ptr getArg(int n) const
			{return args[n];}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Pred_ptr pred;
		std::vector<Expr_ptr> args;
	};

	/**
	 * Negation expression
	 */
	class NegationExpr : public Expression {
	public:
		NegationExpr(Expr_ptr expr) : expr(expr) {}
		Expr_ptr getExpr() const {return expr;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Expr_ptr expr;
	};

	/**
	 * Classical binary expressions: and, or, implication and equivalence
	 */
	class ConnectiveExpr : public Expression {
	public:
		enum Type {AND, OR, IMPL, EQUIV};
		ConnectiveExpr(Type type, Expr_ptr first,
			Expr_ptr second) : type(type), expr{first, second} {}
		Type getType() const {return type;}
		Expr_ptr getFirstExpr() const {return expr[0];}
		Expr_ptr getSecondExpr() const {return expr[1];}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Type type;
		Expr_ptr expr[2];
	};

	/**
	 * Quantifier expressions
	 */
	class QuantifierExpr : public Expression {
	public:
		enum Type {EXISTS, FORALL};
		QuantifierExpr(Type type, PredicateLambda &&pred)
			: type(type), pred(std::move(pred)) {}
		Type getType() const
			{return type;}
		const PredicateLambda& getPredicateLambda() const
			{return pred;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Type type;
		PredicateLambda pred;
	};

	/**
	 * Logical syntactical sugar
	 */
	// class LongImplication : public Expression {};
	// class LongConjunction : public Expression {};
}	// End of namespace Core

#endif
