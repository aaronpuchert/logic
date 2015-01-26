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
#include "debug.hpp"
#include "expression.hpp"
#include <algorithm>
using namespace Core;

/**
 * Construct standard type.
 * @method BuiltInType::BuiltInType
 * @param variant Can be any of Type::{UNDEFINED|TYPE|STATEMENT|RULE}.
 */
BuiltInType::BuiltInType(Variant variant)
	: Expression(Expression::BUILTINTYPE), variant(variant) {}

/**
 * Get type of a type expression.
 * @method BuiltInType::getType
 * @return Type of a type, which is type.
 */
const_Expr_ptr BuiltInType::getType() const
{
	return BuiltInType::type;
}

void BuiltInType::accept(Visitor *visitor) const
{
	visitor->visit(this);
}

// GLOBAL standard types.
const const_Expr_ptr
	BuiltInType::type = std::make_shared<BuiltInType>(BuiltInType::TYPE),
	BuiltInType::statement = std::make_shared<BuiltInType>(BuiltInType::STATEMENT),
	BuiltInType::rule = std::make_shared<BuiltInType>(BuiltInType::RULE),
	BuiltInType::undefined = std::make_shared<BuiltInType>(BuiltInType::UNDEFINED);

/**
 * Construct a LambdaType.
 * @method LambdaType::LambdaType
 * @param args Vector of argument types.
 * @param return_type Type of return value, defaults to statement.
 */
LambdaType::LambdaType(std::vector<const_Expr_ptr> &&args, const_Expr_ptr return_type)
	: Expression(Expression::LAMBDATYPE), return_type(return_type), args(std::move(args))
{
	// Is the return type a type?
	if (return_type->getType() != BuiltInType::type)
		throw TypeException(return_type->getType(), BuiltInType::type);

	// Are the arguments types?
	auto find = std::find_if(this->args.begin(), this->args.end(),
		[] (const_Expr_ptr arg) -> bool
		{return (arg->getType() != BuiltInType::type);}
	);

	if (find != this->args.end()) {
		std::ostringstream str;
		str << "argument " << find - this->args.begin() + 1;
		throw TypeException((*find)->getType(), BuiltInType::type, str.str());
	}
}

/**
 * Get type of a type expression.
 * @method LambdaType::getType
 * @return Type of a type, which is type.
 */
const_Expr_ptr LambdaType::getType() const
{
	return BuiltInType::type;
}

/**
 * Get the return type of the lambda.
 * @method LambdaType::getReturnType
 * @return Pointer to the return type.
 */
const_Expr_ptr LambdaType::getReturnType() const
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
 * Compare function for types
 * @method TypeComparator::operator()
 * @return True, if a==b.
 */
bool TypeComparator::operator()(const Expression *a, const Expression *b)
{
	// Check if we have types at all
	if (a->getType() != BuiltInType::type || b->getType() != BuiltInType::type)
		throw std::logic_error("Trying to compare non-types in TypeComparator");

	// If pointers agree, the types must be equal.
	if (a == b)
		return true;

	// Otherwise, write descriptions ...
	description[0].resize(0);
	description[1].resize(0);
	yours = 0;	a->accept(this);
	yours = 1;	b->accept(this);

	// ... and compare them
	return (description[0] == description[1]);
}

void TypeComparator::visit(const BuiltInType *type)
{
	// The following assumes that small values will never be used as adresses.
	description[yours].push_back(reinterpret_cast<void *>(type->variant));
}

void TypeComparator::visit(const LambdaType *type)
{
	// The following assumes that 0xff..ff and 0xff..fe will never be used as adresses.
	description[yours].push_back(reinterpret_cast<void *>(-1));
	type->getReturnType()->accept(this);
	for (const_Expr_ptr arg_type : *type)
		arg_type->accept(this);
	description[yours].push_back(reinterpret_cast<void *>(-2));
}

void TypeComparator::visit(const AtomicExpr *type)
{
	// If there is a definition, look at that.
	const_Node_ptr node = type->getAtom();

	Context::const_iterator find;
	if (context && (find = context->find(node)) != context->end())
		find->second->accept(this);
	else
		description[yours].push_back(node.get());
}

/**
 * Construct a node.
 * @param type Type of the node
 * @param name Name or identifier of a node.
 */
Node::Node(const_Expr_ptr type, const std::string &name)
	: type(type), name(name)
{
	if (type->getType() != BuiltInType::type)
		throw TypeException(type->getType(), BuiltInType::type);
}

/**
 * Clone node object
 * @method Node::clone
 * @return Pointer to new node object.
 */
Node_ptr Node::clone() const
{
	return std::make_shared<Node>(*this);
}

/**
 * Set definition for a node.
 * @method Node::setDefinition
 * @param  new_expression Expression to serve as new definition.
 */
void Node::setDefinition(Expr_ptr new_expression)
{
	TypeComparator compare;
	if (compare(type.get(), new_expression->getType().get()))
		expression = new_expression;
	else
		throw TypeException(new_expression->getType(), type);
}

void Node::accept(Visitor *visitor) const
{
	visitor->visit(this);
}
