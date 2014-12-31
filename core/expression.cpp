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

using namespace Core;

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
 * Construct predicate lambda expression.
 * @method PredicateLambda::PredicateLambda
 * @param params Parameters to the lambda expression.
 */
PredicateLambda::PredicateLambda(Theory &&params)
	: params(std::move(params)) {}

/**
 * Set definition expression for predicate lambda.
 * @method PredicateLambda::setDefinition
 * @param  expression Expression defining the predicate.
 */
void PredicateLambda::setDefinition(const_Expr_ptr new_expression)
{
	// TODO: type check.
	expression = new_expression;
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
