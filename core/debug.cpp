/*
 *   Logging and exceptions.
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

#include "debug.hpp"
#include "base.hpp"
#include <stdexcept>
using namespace Core;

TypeException::TypeException(const TypeException &other)
{
	str << other.str.str();
}

/**
 * Construct a TypeException.
 * @param type The type we got.
 * @param want The type we would like to see.
 * @param where Short (local) description where the problem occured.
 */
TypeException::TypeException(const_Type_ptr type, const_Type_ptr want, const std::string &where)
{
	str << "expected ";
	want->accept(this);
	str << ", but got ";
	type->accept(this);
	if (where != "")
		str << " in " << where;
}

/**
 * Construct a TypeException.
 * @param type The type we got.
 * @param want Description of what we'd like to see.
 * @param where Short (local) description where the problem occured.
 */
TypeException::TypeException(const_Type_ptr type, const std::string &want, const std::string &where)
{
	str << "expected " << want << ", but got ";
	type->accept(this);
	if (where != "")
		str << " in " << where;
}

void TypeException::visit(const BuiltInType *type)
{
	switch (type->variant) {
	case BuiltInType::UNDEFINED:
		str << "undefined";
		break;
	case BuiltInType::TYPE:
		str << "type";
		break;
	case BuiltInType::STATEMENT:
		str << "statement";
		break;
	case BuiltInType::RULE:
		str << "rule";
		break;
	}
}

void TypeException::visit(const VariableType *type)
{
	str << type->getName();
}

void TypeException::visit(const LambdaType *type)
{
	str << "(";
	bool first = true;
	for (const_Type_ptr arg_type : *type) {
		if (!first) {
			first = true;
			str << ' ';
		}
		arg_type->accept(this);
	}
	str << ")->";
	type->getReturnType()->accept(this);
}

/**
 * Get description of a type exception.
 * @return Description.
 */
const char *TypeException::what() const noexcept
{
	return str.str().c_str();
}


/**
 * Construct a NamespaceException.
 * @param reason One of NamespaceException::{NOTFOUND|DUPLICATE}.
 * @param name The name of the node this is about.
 */
NamespaceException::NamespaceException(Reason reason, const std::string &name)
	: reason(reason), name(name) {}

/**
 * Return exception description.
 * @return Description.
 */
const char* NamespaceException::what() const noexcept
{
	std::ostringstream str;

	switch (reason) {
	case NOTFOUND:
		str << "Did not find symbol: " << name;
		break;
	case DUPLICATE:
		str << "Duplicate symbol: " << name;
		break;
	}

	description = str.str();
	return description.c_str();
}
