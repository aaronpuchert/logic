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

#ifndef CORE_DEBUG_HPP
#define CORE_DEBUG_HPP
#include "forward.hpp"
#include <sstream>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Exception for mismatched types.
	 */
	class TypeException : public std::exception {
	public:
		TypeException(const_Type_ptr type, const_Type_ptr want,
			const std::string &where = "");
		TypeException(const_Type_ptr type, const std::string &want,
			const std::string &where = "");
		const char *what() const noexcept;

	private:
		std::string description;
	};

	/**
	 * Type writer
	 */
	class TypeWriter : public Visitor {
	public:
		TypeWriter(std::ostream &str) : str(str) {}
		void write(const Type *type);
		void visit(const BuiltInType *type);
		void visit(const VariableType *type);
		void visit(const LambdaType *type);

	private:
		std::ostream &str;
	};

	/**
	 * Exception for not finding entries
	 */
	class NamespaceException : public std::exception {
	public:
		enum Reason {NOTFOUND, DUPLICATE};
		NamespaceException(Reason reason, const std::string &name);
		const char* what() const noexcept;

	private:
		const Reason reason;
		const std::string name;
		mutable std::string description;
	};
}

#endif
