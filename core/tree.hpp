/*
 *   Handling the abstract syntax tree: substitutions etc.
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

#ifndef CORE_TREE_HPP
#define CORE_TREE_HPP
#include "forward.hpp"
#include "base.hpp"
#include "traverse.hpp"
#include <stack>
#include <map>

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Substitution algorithm:
	 * - we would like to check if a expression can be the result of a
	 *   substitution without producing it.
	 * - we would like to know if a certain pattern fits on an expression and
	 *   what the parameters would be.
	 */
	class Substitution : public Visitor {
	public:
		// Init with expression
		Substitution(const_Expr_ptr expr, const Theory *params);
		const_Expr_ptr getExpr() const
			{return expr;}

		// Test against other expression
		typedef std::pair<const_Expr_ptr, const Expression *> match;
		bool check(const Expression *target);
		match getMismatch() const;

		// Visitor functions
		void visit(const AtomicExpr *expression);
		void visit(const LambdaCallExpr *expression);
		void visit(const NegationExpr *expression);
		void visit(const ConnectiveExpr *expression);
		void visit(const QuantifierExpr *expression);
		void visit(const LambdaExpr *expression);

	private:
		void push(const_Expr_ptr expr);
		void pop();

		void add(const_Node_ptr node, const_Expr_ptr expr);
		void pop_theory();
		const_Expr_ptr have(const_Node_ptr node);

		void mismatch(const_Expr_ptr expr, const Expression *target_expr);

		const_Expr_ptr expr;
		const Theory *theory;
		std::map<const_Node_ptr, const_Expr_ptr> substitutions;

		std::stack<const_Expr_ptr> stack;
		std::stack<const Theory *> theory_stack;
		match offender;
	};
}

#endif
