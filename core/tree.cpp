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

/**
 * The Substitution class takes three inputs: two expressions and a list of
 * parameters. The parameters are then substituted in one of the expressions
 * and the result is compared to the target.
 *
 * Since we have to traverse the syntax tree to compare the two expressions,
 * we derived the class from the visitor class. The visiting functions traverse
 * the target expression, while we keep track of where we are in the other
 * expression or substitute expressions via a stack.
 *
 * The visit functions only have to compare the nodes on the highest level and
 * then push the kids of the stack expressions on the stack, while at the same
 * time letting their on kids (in the target) accept the visitor.
 *
 * The real work of substituting is then done when we push an expression on the
 * stack: if an expression refers to a node, such as an atomic expression, and
 * we have a definition for that node in the parameter list, then we push that
 * definition instead of the atomic expression or whatever it is.
 */

#include "tree.hpp"
#include "expression.hpp"
#include <algorithm>
using namespace Core;

/**
 * Initialize Substitution with an expression and a parameter theory.
 * @method Substitution::Substitution
 * @param expr Expression to substitute in.
 * @param params Pointer to a theory containing nodes with definition for each
 *               atom to be substituted.
 */
Substitution::Substitution(const_Expr_ptr expr, const Theory *params)
	: expr(expr), theory(params) {}

/**
 * Check if substituting certain expressions for variables in the expression
 * gives the target expression.
 * @method Substitution::check
 * @param target Target expression.
 * @return Return true, if the target matches.
 */
bool Substitution::check(const Expression *target)
{
	offender.first.reset();

	// Get substitutions
	for (const_Node_ptr node : *theory)
		add(node, node->getDefinition());
	theory_stack.push(theory);

	// Traverse expression target, and compare with expr
	push(expr);
	target->accept(this);
	pop();

	return !offender.first;
}

/**
 * Get mismatch.
 * @method Substitution::getMismatch
 * @return Return a pair of mismatching expressions or a pair with first pointer reset.
 */
Substitution::match Substitution::getMismatch() const
{
	return offender;
}

/**
 * Compare atomic expression in target. Of course we want to know if they refer
 * to the same node
 * @method Substitution::visit
 * @param expression Atomic expression in target
 */
void Substitution::visit(const AtomicExpr *expression)
{
	// Then we really should have an atomic expression on the other side.
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::ATOMIC) {
		auto expr_atomic = std::static_pointer_cast<const AtomicExpr>(expr);

		if (expr_atomic->getAtom() == expression->getAtom())
			return;
	}

	mismatch(expr, expression);
}

/**
 * Compare a lambda call expression: for this we have to know if the same lambda
 * was called and if the parameters compare.
 * @method Substitution::visit
 * @param expression Lambda call expression in target
 */
void Substitution::visit(const LambdaCallExpr *expression)
{
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::LAMBDACALL) {
		auto expr_call = std::static_pointer_cast<const LambdaCallExpr>(expr);

		// Compare the lambda node
		if (expr_call->getLambda() == expression->getLambda()) {
			// Compare the arguments
			for (LambdaCallExpr::const_iterator it_target = expression->begin(),
				it_expr = expr_call->begin(); it_target != expression->end() &&
				it_expr != expr_call->end(); ++it_expr, ++it_target
			) {
				push(*it_expr);
				(*it_target)->accept(this);
				pop();
			}

			return;
		}
	}

	mismatch(expr, expression);
}

/**
 * Compare a negation expression.
 * @method Substitution::visit
 * @param expression Negation expression in target
 */
void Substitution::visit(const NegationExpr *expression)
{
	// Do we have a negation in the expr?
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::NEGATION) {
		auto expr_neg = std::static_pointer_cast<const NegationExpr>(expr);

		// Push and go on
		push(expr_neg->getExpr());
		expression->getExpr()->accept(this);
		pop();
	}
	else
		mismatch(expr, expression);
}

/**
 * Compare a connective expression. We need the same variant and then to compare
 * the two operands.
 * @method Substitution::visit
 * @param expression Connective expression in target
 */
void Substitution::visit(const ConnectiveExpr *expression)
{
	// Do we have a connective in the expr?
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::CONNECTIVE) {
		auto expr_con = std::static_pointer_cast<const ConnectiveExpr>(expr);

		// Do we have the same variant?
		if (expr_con->getVariant() == expression->getVariant()) {
			// Check first operand
			push(expr_con->getFirstExpr());
			expression->getFirstExpr()->accept(this);
			pop();

			// Check second operand
			push(expr_con->getSecondExpr());
			expression->getSecondExpr()->accept(this);
			pop();

			return;
		}
	}

	mismatch(expr, expression);
}

/**
 * Compare a quantifier expression. We compare the variant and the contained
 * predicate lambda.
 * @method Substitution::visit
 * @param expression Quantifier expression in target
 */
void Substitution::visit(const QuantifierExpr *expression)
{
	// Do we have a quantifier expression?
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::QUANTIFIER) {
		auto expr_quant = std::static_pointer_cast<const QuantifierExpr>(expr);

		if (expr_quant->getVariant() == expression->getVariant()) {
			// Push and go on
			push(expr_quant->getPredicate());
			expression->getPredicate()->accept(this);
			pop();

			return;
		}
	}

	mismatch(expr, expression);
}

/**
 * Compare a lambda expression.
 * First we have to compare their type signature. Then to match the contained
 * expressions, we have to create a theory translating the parameter names.
 * @method Substitution::visit
 * @param expression Lambda expression in target
 */
void Substitution::visit(const LambdaExpr *expression)
{
	// Do we have a lambda expression on the other side?
	const_Expr_ptr expr = stack.top();
	if (expr->cls == Expression::LAMBDA) {
		auto expr_lambda = std::static_pointer_cast<const LambdaExpr>(expr);

		// Do the type signatures match?
		TypeComparator compare;
		if (compare(expression->getType().get(), expr_lambda->getType().get())) {
			// Try to match theories
			theory_stack.push(&expr_lambda->getParams());

			auto param_it = expr_lambda->getParams().begin();
			auto subst_it = expression->getParams().begin();
			for (; param_it != expr_lambda->getParams().end(); ++param_it, ++subst_it)
				add(*param_it, std::make_shared<AtomicExpr>(*subst_it));

			// Compare the definition
			push(expr_lambda->getDefinition());
			expression->getDefinition()->accept(this);
			pop();

			return;
		}
	}

	mismatch(expr, expression);
}

/**
 * Push an expression to the stack and do substitutions, if necessary.
 * @method Substitution::push
 * @param expr Expression to push to stack
 */
void Substitution::push(const_Expr_ptr expr)
{
	switch (expr->cls) {
	case Expression::ATOMIC: {        // For atomics, when we can: resolve
		auto atomic = std::static_pointer_cast<const AtomicExpr>(expr);
		const_Expr_ptr def = have(atomic->getAtom());
		if (def) {
			// Push definition on stack
			// A null pointer on the theory stack serves as a placeholder.
			theory_stack.push(nullptr);
			stack.push(def);
			return;
		}
		}	// case end
		break;
	case Expression::LAMBDACALL: {
		auto call = std::static_pointer_cast<const LambdaCallExpr>(expr);
		const_Expr_ptr lambda_def = have(call->getLambda());
		if (lambda_def) {
			// Is it atomic? Then just substitute.
			if (lambda_def->cls == Expression::ATOMIC) {
				auto atomic = std::static_pointer_cast<const AtomicExpr>(lambda_def);
				// TODO
				throw -1;
			}
			else {
				// Otherwise it is a lambda. Then we have to plug in.
				auto lambda = std::static_pointer_cast<const LambdaExpr>(lambda_def);

				// Write substitutions.
				theory_stack.push(&lambda->getParams());
				auto param_it = lambda->getParams().begin();
				auto arg_it = call->begin();
				for (; param_it != lambda->getParams().end(); ++param_it, ++arg_it)
					add(*param_it, *arg_it);

				// Recursively push definition
				push(lambda->getDefinition());
			}

			return;
		}
		}	// case end
		break;
	}

	// If there's nothing to substitute: just push the expression.
	// A null pointer on the theory stack serves as a placeholder.
	theory_stack.push(nullptr);
	stack.push(expr);
}

/**
 * Pop an expression from the stack.
 * @method Substitution::pop
 */
void Substitution::pop()
{
	// Pop null theory
	if (theory_stack.top())
		throw std::logic_error("Non-null theory pointer on substitutions stack in pop");
	theory_stack.pop();

	// Pop all theories until the next null theory
	while (theory_stack.size() && theory_stack.top())
		pop_theory();

	stack.pop();
}

/**
 * Add substitution to map.
 * @method Substitution::add
 * @param node Node to substitute.
 * @param expr Expression to substitute with.
 */
void Substitution::add(const_Node_ptr node, const_Expr_ptr expr)
{
	// If expr is atomic, look it up. This is to create shortcuts.
	if (expr->cls == Expression::ATOMIC) {
		auto atomic = std::static_pointer_cast<const AtomicExpr>(expr);
		auto find = substitutions.find(atomic->getAtom());
		if (find != substitutions.end())
			expr = find->second;
	}

	substitutions.insert({node, expr});
}

/**
 * Pop theory from stack and forget about the substitutions.
 * @method Substitution::pop_theory
 */
void Substitution::pop_theory()
{
	const Theory *theory = theory_stack.top();
	for (const_Node_ptr node : *theory)
		substitutions.erase(node);
	theory_stack.pop();
}

/**
 * Get our variant of a certain node.
 * @method Substitution::have
 * @param node Node referred to in the expression.
 * @return Node from our theory stack with the right definition.
 */
const_Expr_ptr Substitution::have(const_Node_ptr node)
{
	auto find = substitutions.find(node);
	if (find != substitutions.end())
		return find->second;
	else
		return Expr_ptr();
}

/**
 * Report mismatch of expressions on the top level.
 * @method Substitution::mismatch
 * @param expr Subexpression to compared against.
 * @param target_expr Subexpression of target.
 */
void Substitution::mismatch(const_Expr_ptr expr, const Expression *target_expr)
{
	offender = match(expr, target_expr);
}
