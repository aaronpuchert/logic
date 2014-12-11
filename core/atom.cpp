/*
 *   Data structures for atomic objects such as constants, variables and
 *   predicates.
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

#include "atom.hpp"

using namespace Core;

void Variable::setDefinition(Expr_ptr new_expression)
{
	expression = new_expression;
}

int PredicateDecl::getValency() const
{
	return type_list.size();
}

void PredicateDecl::setParameterType(int n, Type_ptr type)
{
	type_list[n] = type;
}

const_Type_ptr PredicateDecl::getParameterType(int n) const
{
	return type_list[n];
}

PredicateLambda::PredicateLambda(std::vector<Variable> &&vars, Expr_ptr expression)
	: vars(std::move(vars)), expression(expression) {}

int PredicateLambda::getValency() const
{
	return vars.size();
}

const_Type_ptr PredicateLambda::getParameterType(int n) const
{
	return vars[n].getType();
}
