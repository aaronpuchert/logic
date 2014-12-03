/*
 *   Namespaces and the namespace stack.
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

#include "namespace.hpp"
using namespace Core;

NamespaceException::NamespaceException(Reason reason, const std::string &name)
	: reason(reason), name(name)
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
}

/**
 * Return exception description.
 */
const char* NamespaceException::what() const noexcept
{
	return description.c_str();
}
