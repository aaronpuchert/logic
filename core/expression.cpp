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
#include <algorithm>
#include <sstream>

using namespace Core;

/**
 * Get type of an atomic expression.
 * @method AtomicExpr::getType
 * @return Type of the expression.
 */
const_Type_ptr AtomicExpr::getType() const
{
	return node->getType();
}

/**
 * Construct predicate (call) expression.
 * @method PredicateExpr::PredicateExpr
 * @param node Predicate to call.
 * @param args Vector of argument expressions.
 */
PredicateExpr::PredicateExpr(const_Node_ptr node, std::vector<Expr_ptr> &&args)
	: node(node), args(std::move(args))
{
	// Is node a predicate?
	std::shared_ptr<const LambdaType> pred_type =
		std::dynamic_pointer_cast<const LambdaType>(node->getType());

	if (!pred_type)
		throw TypeException(node->getType(), "lambda expression");

	if (pred_type->getReturnType() != BuiltInType::statement)
		throw TypeException(pred_type->getReturnType(), BuiltInType::statement, "return type");

	// Do the arguments have the right type?
	TypeComparator compare;
	auto mismatch = std::mismatch(pred_type->begin(), pred_type->end(), this->args.begin(),
		[&compare] (const_Type_ptr a, Expr_ptr b) -> bool
		{return compare(a.get(), b->getType().get());}
	);

	if (mismatch.first != pred_type->end() || mismatch.second != this->args.end()) {
		std::ostringstream str;
		str << "argument number " << mismatch.second - this->args.begin() + 1;
		throw TypeException((*mismatch.second)->getType(), *mismatch.first, str.str());
	}
}

const_Type_ptr PredicateExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Begin iterator for iterating through the arguments.
 * @method PredicateExpr::begin
 * @return Begin iterator.
 */
PredicateExpr::const_iterator PredicateExpr::begin() const
{
	return args.begin();
}

/**
 * End iterator for iterating through the arguments.
 * @method PredicateExpr::end
 * @return End iterator.
 */
PredicateExpr::const_iterator PredicateExpr::end() const
{
	return args.end();
}

/**
 * Construct a negation expression.
 * @method NegationExpr::NegationExpr
 * @param expr Statement expression to be negated.
 */
NegationExpr::NegationExpr(Expr_ptr expr) : expr(expr)
{
	const_Type_ptr type = expr->getType();
	if (type != BuiltInType::statement)
		throw TypeException(type, BuiltInType::statement);
}

const_Type_ptr NegationExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct connective expression.
 * @method ConnectiveExpr::ConnectiveExpr
 * @param variant One of ConnectiveExpr::{AND|OR|IMPL|EQUIV}
 * @param first First operand of connective
 * @param seond Second operand of connective
 */
ConnectiveExpr::ConnectiveExpr(Variant variant, Expr_ptr first, Expr_ptr second)
	: variant(variant), expr{first, second}
{
	const_Type_ptr first_type = first->getType(), second_type = second->getType();
	if (first_type != BuiltInType::statement)
		throw TypeException(first_type, BuiltInType::statement, "first operand");
	if (second_type != BuiltInType::statement)
		throw TypeException(second_type, BuiltInType::statement, "second operand");
}

const_Type_ptr ConnectiveExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct a quantifier expression.
 * @method QuantifierExpr::QuantifierExpr
 * @param variant One of QuantifierExpr::{EXISTS|FORALL}
 * @param predicate Lambda expression containing the predicate
 */
QuantifierExpr::QuantifierExpr(Variant variant, const_Expr_ptr predicate)
	: variant(variant), predicate(predicate)
{
	const_Type_ptr type = predicate->getType();
	std::shared_ptr<const LambdaType> pred_type =
		std::dynamic_pointer_cast<const LambdaType>(type);
	if (pred_type) {
		const_Type_ptr return_type = pred_type->getReturnType();
		if (return_type != BuiltInType::statement)
			throw TypeException(return_type, BuiltInType::statement, "return value");
	}
	else
		throw TypeException(type, "lambda expression");
}

const_Type_ptr QuantifierExpr::getType() const
{
	return BuiltInType::statement;
}

/**
 * Construct predicate lambda expression.
 * @method PredicateLambda::PredicateLambda
 * @param params Parameters to the lambda expression.
 * @param expression Statement expression.
 */
PredicateLambda::PredicateLambda(Theory &&params, const_Expr_ptr expression)
	: params(std::move(params)), expression(expression)
{
	if (expression->getType() != BuiltInType::statement)
		throw TypeException(expression->getType(), BuiltInType::statement);
}

/**
 * Set definition expression for predicate lambda.
 * @method PredicateLambda::setDefinition
 * @param  expression Expression defining the predicate.
 */
void PredicateLambda::setDefinition(const_Expr_ptr new_expression)
{
	if (new_expression->getType() == BuiltInType::statement)
		expression = new_expression;
	else
		throw TypeException(new_expression->getType(), BuiltInType::statement);
}

/**
 * Get type of Lambda expression.
 * @method PredicateLambda::getType
 * @return Type of Lambda expression.
 */
const_Type_ptr PredicateLambda::getType() const
{
	// Build type if required
	if (!type) {
		// transform can't write to an empty vector, hence initialize with dummy
		std::vector<const_Type_ptr> types{const_Type_ptr()};
		std::transform(params.begin(), params.end(), types.begin(),
			[] (const_Node_ptr node) -> const_Type_ptr {return node->getType();});
		type = std::make_shared<LambdaType>(std::move(types));
	}

	return type;
}

/**
 * Begin iterator for iterating through the paramenters.
 * @method PredicateLambda::begin
 * @return Begin iterator.
 */
Theory::const_iterator PredicateLambda::begin() const
{
	return params.begin();
}

/**
 * End iterator for iterating through the parameters.
 * @method PredicateLambda::end
 * @return End iterator.
 */
Theory::const_iterator PredicateLambda::end() const
{
	return params.end();
}

void PredicateLambda::accept(Core::Visitor *visitor) const
{
	visitor->visit(this);
}
