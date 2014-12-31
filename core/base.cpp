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
 * @method Type::Type
 * @param variant Can be any of Type::{UNDEFINED|TYPE|PREDICATE|STATEMENT|RULE}.
 */
Type::Type(Variant variant)
	: variant(variant) {}

/**
 * Get name of standard type.
 * @method Type::getName
 * @return String containing the name.
 */
std::string Type::getName() const
{
	switch (variant) {
	case TYPE:
		return "type";
	case PREDICATE:
		return "predicate";
	case STATEMENT:
		return "statement";
	case RULE:
		return "rule";
	default:
		return "undefined";
	}
}

void Type::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

/**
 * Construct a VariableType.
 * @method VariableType::VariableType
 * @param node Pointer to a node of Type type.
 */
VariableType::VariableType(const_Node_ptr node)
	: Type(Type::VARIABLE), node(node)
{
	if (node->getType() != type_type)
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
std::string VariableType::getName() const
{
	return node->getName();
}

void VariableType::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

// GLOBAL standard types.
namespace Core {
const const_Type_ptr
	type_type = std::make_shared<Type>(Type::TYPE),
	statement_type = std::make_shared<Type>(Type::STATEMENT),
	predicate_type = std::make_shared<Type>(Type::PREDICATE),
	rule_type = std::make_shared<Type>(Type::RULE),
	undefined_type = std::make_shared<Type>(Type::UNDEFINED);
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
