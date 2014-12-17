/*
 *   Data structures for atomic objects such as constants, variables and
 *   predicates.
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

#ifndef CORE_ATOM_HPP
#define CORE_ATOM_HPP
#include "forward.hpp"
#include "type.hpp"
#include <string>
#include <vector>
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
			: Node(name, Node::VARIABLE), type(type) {}
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

	/**
	 * Abstract class for predicates.
	 */
	class Predicate : public Node {
	public:
		Predicate(const std::string& name)
			: Node(name, Node::PREDICATE) {}
		virtual int getValency() const = 0;
		virtual const_Type_ptr getParameterType(int n) const = 0;
	};

	/**
	 * Declaration of a predicate giving just the types of arguments.
	 */
	class PredicateDecl : public Predicate {
	public:
		PredicateDecl(const std::string& name, int valency)
			: Predicate(name), type_list(valency, Type_ptr()) {}
		int getValency() const;
		void setParameterType(int n, Type_ptr type);
		const_Type_ptr getParameterType(int n) const;

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		std::vector<Type_ptr> type_list;
	};

	/**
	 * Definition of an anonymous Predicate.
	 */
	class PredicateLambda {
	public:
		PredicateLambda(std::vector<Variable> &&vars, Expr_ptr expression);
		int getValency() const;
		const_Type_ptr getParameterType(int n) const;
		const Variable& getDeclaration(int n) const;
		const_Expr_ptr getDefinition() const
			{return expression;}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		std::vector<Variable> vars;
		Expr_ptr expression;
	};

	/**
	 * Definition of a named predicate.
	 */
	class PredicateDef : public Predicate {
	public:
		PredicateDef(const std::string& name, std::vector<Variable> &&vars, Expr_ptr expression)
			: Predicate(name), lambda(std::move(vars), expression) {}

		int getValency() const
			{return lambda.getValency();}
		const_Type_ptr getParameterType(int n) const
			{return lambda.getParameterType(n);}
		const PredicateLambda& getDefinition() const
			{return lambda;}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		PredicateLambda lambda;
	};

	/**
	 * Functions, relations?
	 */
}	// End of namespace Core

#endif
