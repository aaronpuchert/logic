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

#pragma once
#include <map>
#include <string>
#include <exception>
#include <sstream>
#include <memory>
#include "type.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Nested namespaces.
	 */
	class Namespace {
	public:
		typedef std::shared_ptr<Node> node_ptr;
		Namespace(const Namespace *parent) : parent(parent) {};
		void add(node_ptr object);
		node_ptr get(const std::string& name) const;

	private:
		std::map<std::string, node_ptr> map;
		const Namespace *parent;
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
	};
}	// End of namespace Core
