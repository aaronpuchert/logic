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

#include "expression.hpp"
#include "debug.hpp"
#include <algorithm>
#include <sstream>

using namespace Core;

/**
 * Get type of an atomic expression.
 *
 * @return Type of the expression.
 */
const_Expr_ptr AtomicExpr::getType() const
{
	return node->getType();
}

/**
 * Construct predicate (call) expression.
 *
 * @param node Predicate to call.
 * @param args Vector of argument expressions.
 */
LambdaCallExpr::LambdaCallExpr(const_Node_ptr node, std::vector<Expr_ptr> &&args)
	: Expression(Expression::LAMBDACALL), node(node), args(std::move(args))
{
	// Is node a lambda?
	if (node->getType()->cls != Expression::LAMBDATYPE)
		throw TypeException(node->getType(), "lambda expression");

	auto pred_type = std::static_pointer_cast<const LambdaType>(node->getType());

	// Do the arguments have the right type?
	TypeComparator compare;
	auto mismatch = std::mismatch(pred_type->begin(), pred_type->end(), this->args.begin(),
		[&compare] (const_Expr_ptr a, Expr_ptr b) -> bool
		{return compare(a.get(), b->getType().get());}
	);

	if (mismatch.first != pred_type->end() || mismatch.second != this->args.end()) {
		std::ostringstream str;
		str << "argument " << mismatch.second - this->args.begin() + 1;
		throw TypeException((*mismatch.second)->getType(), *mismatch.first, str.str());
	}
}

const_Expr_ptr LambdaCallExpr::getType() const
{
	auto pred_type = std::static_pointer_cast<const LambdaType>(node->getType());
	return pred_type->getReturnType();
}

/**
 * Begin iterator for iterating through the arguments.
 *
 * @return Begin iterator.
 */
LambdaCallExpr::const_iterator LambdaCallExpr::begin() const
{
	return args.begin();
}

/**
 * End iterator for iterating through the arguments.
 *
 * @return End iterator.
 */
LambdaCallExpr::const_iterator LambdaCallExpr::end() const
{
	return args.end();
}

/**
 * Construct a negation expression.
 *
 * @param expr %Statement expression to be negated.
 */
NegationExpr::NegationExpr(Expr_ptr expr)
	: Expression(Expression::NEGATION), expr(expr)
{
	const_Expr_ptr type = expr->getType();
	if (type != BuiltInType::statement)
		throw TypeException(type, BuiltInType::statement);
}

const_Expr_ptr NegationExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct connective expression.
 *
 * @param variant One of ConnectiveExpr::{AND|OR|IMPL|EQUIV}
 * @param first First operand of connective
 * @param seond Second operand of connective
 */
ConnectiveExpr::ConnectiveExpr(Variant variant, Expr_ptr first, Expr_ptr second)
	: Expression(Expression::CONNECTIVE), variant(variant), expr{first, second}
{
	const_Expr_ptr first_type = first->getType(), second_type = second->getType();
	if (first_type != BuiltInType::statement)
		throw TypeException(first_type, BuiltInType::statement, "first operand");
	if (second_type != BuiltInType::statement)
		throw TypeException(second_type, BuiltInType::statement, "second operand");
}

const_Expr_ptr ConnectiveExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct a quantifier expression.
 *
 * @param variant One of QuantifierExpr::{EXISTS|FORALL}
 * @param predicate Lambda expression containing the predicate
 */
QuantifierExpr::QuantifierExpr(Variant variant, const_Expr_ptr predicate)
	: Expression(Expression::QUANTIFIER), variant(variant), predicate(predicate)
{
	const_Expr_ptr type = predicate->getType();
	if (type->cls == Expression::LAMBDATYPE) {
		auto pred_type = std::static_pointer_cast<const LambdaType>(type);
		const_Expr_ptr return_type = pred_type->getReturnType();
		if (return_type != BuiltInType::statement)
			throw TypeException(return_type, BuiltInType::statement, "return value");
	}
	else
		throw TypeException(type, "lambda expression");
}

const_Expr_ptr QuantifierExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct lambda expression.
 *
 * @param params Parameters to the lambda expression.
 * @param expression Lambda body.
 */
LambdaExpr::LambdaExpr(Theory &&params, const_Expr_ptr expression)
	: Expression(Expression::LAMBDA), params(std::move(params)), expression(expression) {}

/**
 * Set definition expression for lambda.
 *
 * @param  expression Expression defining the predicate.
 */
void LambdaExpr::setDefinition(const_Expr_ptr new_expression)
{
	// We shouldn't accept it if the return type changes.
	TypeComparator compare;
	if (!compare(expression->getType().get(), new_expression->getType().get()))
		throw TypeException(new_expression->getType(), expression->getType(), "return type");

	expression = new_expression;
}

/**
 * Get type of Lambda expression.
 *
 * @return Type of Lambda expression.
 */
const_Expr_ptr LambdaExpr::getType() const
{
	// Build type if required
	if (!type) {
		// transform can't write to an empty vector, hence initialize with dummy
		std::vector<const_Expr_ptr> types{const_Expr_ptr()};
		std::transform(params.begin(), params.end(), types.begin(),
			[] (const_Node_ptr node) -> const_Expr_ptr {return node->getType();});
		type = std::make_shared<LambdaType>(std::move(types), expression->getType());
	}

	return type;
}

/**
 * Begin iterator for iterating through the paramenters.
 *
 * @return Begin iterator.
 */
Theory::const_iterator LambdaExpr::begin() const
{
	return params.begin();
}

/**
 * End iterator for iterating through the parameters.
 *
 * @return End iterator.
 */
Theory::const_iterator LambdaExpr::end() const
{
	return params.end();
}

void LambdaExpr::accept(Core::Visitor *visitor) const
{
	visitor->visit(this);
}
