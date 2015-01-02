/*
 *   Base classes for Theory nodes and Expressions.
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

#include "base.hpp"
#include <stdexcept>
using namespace Core;

/**
 * Construct standard type.
 * @method BuiltInType::BuiltInType
 * @param variant Can be any of Type::{UNDEFINED|TYPE|STATEMENT|RULE}.
 */
BuiltInType::BuiltInType(Variant variant)
	: variant(variant) {}

void BuiltInType::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

// GLOBAL standard types.
const const_Type_ptr
	BuiltInType::type = std::make_shared<BuiltInType>(BuiltInType::TYPE),
	BuiltInType::statement = std::make_shared<BuiltInType>(BuiltInType::STATEMENT),
	BuiltInType::rule = std::make_shared<BuiltInType>(BuiltInType::RULE),
	BuiltInType::undefined = std::make_shared<BuiltInType>(BuiltInType::UNDEFINED);

/**
 * Construct a VariableType.
 * @method VariableType::VariableType
 * @param node Pointer to a node of Type type.
 */
VariableType::VariableType(const_Node_ptr node) : node(node)
{
	if (node->getType() != BuiltInType::type)
		std::runtime_error("Can't construct a VariableType from a non-type node.");
}

/**
 * Get the node which declares the type.
 * @method VariableType::getNode
 * @return Pointer to the node.
 */
const_Node_ptr VariableType::getNode() const
{
	return node;
}

/**
 * Get name of variable type.
 * @method VariableType::getName
 * @return String containing the name.
 */
const std::string& VariableType::getName() const
{
	return node->getName();
}

void VariableType::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

/**
 * Construct a LambdaType.
 * @method LambdaType::LambdaType
 * @param args Vector of argument types.
 * @param return_type Type of return value, defaults to statement.
 */
LambdaType::LambdaType(const std::vector<const_Type_ptr> &&args,
	const_Type_ptr return_type)
	: return_type(return_type), args(std::move(args)) {}

/**
 * Get the return type of the lambda.
 * @method LambdaType::getReturnType
 * @return Pointer to the return type.
 */
const_Type_ptr LambdaType::getReturnType() const
{
	return return_type;
}

/**
 * Return an iterator to the beginning of the argument list.
 * @method LambdaType::begin
 * @return Begin iterator.
 */
LambdaType::const_iterator LambdaType::begin() const
{
	return args.begin();
}

/**
 * Return an iterator to the beginning of the argument list.
 * @method LambdaType::end
 * @return End iterator.
 */
LambdaType::const_iterator LambdaType::end() const
{
	return args.end();
}

void LambdaType::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

/**
 * Set definition for a node.
 * @method Node::setDefinition
 * @param  new_expression Expression to serve as new definition.
 */
void Node::setDefinition(Expr_ptr new_expression)
{
	expression = new_expression;
}

void Node::accept(Visitor *visitor) const
{
	visitor->visit(this);
}