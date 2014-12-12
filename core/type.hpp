/*
 *   Type system.
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
#include <string>
#include <memory>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract base class for named entities in theories: types, variables,
	 * predicates, statements.
	 */
	class Node {
	public:
		enum NodeType {UNDEFINED, TYPE, VARIABLE, PREDICATE, STATEMENT}
			const node_type;

		Node(const std::string &name, NodeType node_type)
			: node_type(node_type), name(name) {}
		const std::string &getName() const
			{return name;}

		virtual void accept(Visitor *visitor) const = 0;

	protected:
		std::string name;
	};

	/**
	 * Base class for types.
	 */
	class Type : public Node {
	public:
		Type(const std::string &name)
			: Node(name, Node::TYPE) {}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
	};
}	// End of namespace Core
