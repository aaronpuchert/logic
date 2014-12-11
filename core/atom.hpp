/*
 *   Data structures for constant and variable objects.
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

#pragma once
#include "forward.hpp"
#include "type.hpp"
#include <string>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Class for variable atoms.
	 */
	class Variable : public Node {
	public:
		Variable(Type_ptr type, const std::string &name)
			: Node(name), type(type) {}
		const_Type_ptr getType() const
			{return type;}
		void setDefinition(Expr_ptr new_expression);
		const_Expr_ptr getDefinition() const
			{return expression;}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Type_ptr type;
		Expr_ptr expression;
	};
}	// End of namespace Core
