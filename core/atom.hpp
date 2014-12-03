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
#include <memory>
#include <string>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract base class for atoms.
	 */
	class Atom {
	public:
		Atom(Type_ptr type, const std::string &name)
			: type(type), name(name) {}
		const_Type_ptr getType() const
			{return type;}
		const std::string &getName() const
			{return name;}

		virtual void accept(Visitor *visitor) const = 0;

	protected:
		Type_ptr type;
		std::string name;
	};

	typedef std::shared_ptr<Atom> Atom_ptr;
	typedef std::shared_ptr<const Atom> const_Atom_ptr;

	/**
	 * Class for variable atoms.
	 */
	class Variable : public Atom {
	public:
		Variable(Type_ptr type, const std::string &name)
			: Atom(type, name) {}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
	};
}	// End of namespace Core
