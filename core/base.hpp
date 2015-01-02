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

#ifndef CORE_BASE_HPP
#define CORE_BASE_HPP
#include "forward.hpp"
#include <string>
#include <memory>
#include <vector>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract class for expressions.
	 */
	class Expression {
	public:
		virtual ~Expression() {}
		virtual void accept(Visitor *visitor) const = 0;
		virtual const_Type_ptr getType() const = 0;
	};

	/**
	 * Base class for types.
	 */
	class Type {
	public:
		virtual ~Type() {}
		virtual void accept(Visitor *visitor) const = 0;
	};

	/**
	 * Built-in standard types.
	 */
	class BuiltInType : public Type {
	public:
		enum Variant {UNDEFINED, TYPE, STATEMENT, RULE}
			const variant;

		BuiltInType(Variant variant);
		void accept(Visitor *visitor) const;

		// Global standard types
		static const const_Type_ptr
			type, statement, rule, undefined;
	};

	/**
	 * Variable type.
	 */
	class VariableType : public Type {
	public:
		VariableType(const_Node_ptr node);
		const_Node_ptr getNode() const;
		const std::string& getName() const;

		void accept(Visitor *visitor) const;

	private:
		const_Node_ptr node;
	};

	/**
	 * Lambda type.
	 */
	class LambdaType : public Type {
	public:
		LambdaType(const std::vector<const_Type_ptr> &&args,
			const_Type_ptr return_type = BuiltInType::statement);
		const_Type_ptr getReturnType() const;

		// Iterate over argument types...
		typedef std::vector<const_Type_ptr>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		void accept(Visitor *visitor) const;

	private:
		const_Type_ptr return_type;
		std::vector<const_Type_ptr> args;
	};

	/**
	 * Type comparison class, unfortunately not thread-safe.
	 */
	class TypeComparator : public Visitor {
	public:
		bool operator()(const Type *, const Type *);
		void visit(const BuiltInType *type);
		void visit(const VariableType *type);
		void visit(const LambdaType *type);

	private:
		int yours;
		std::vector<const void *> description[2];
	};

	/**
	 * Exception for mismatched types.
	 */
	class TypeException : std::exception {
	public:
		TypeException(const_Type_ptr type, const_Type_ptr want,
			const std::string &where = "");
		TypeException(const_Type_ptr type, const std::string &want,
			const std::string &where = "");
		const char *what() const noexcept;

	private:
		mutable std::string description;
	};

	/**
	 * Abstract base class for named entities in theories: types, variables,
	 * predicates, statements.
	 */
	class Node {
	public:
		Node(const_Type_ptr type, const std::string &name)
			: type(type), name(name) {}
		virtual ~Node() {}

		const std::string &getName() const
			{return name;}
		void setDefinition(Expr_ptr new_expression);
		const_Type_ptr getType() const
			{return type;}
		const_Expr_ptr getDefinition() const
			{return expression;}

		virtual void accept(Visitor *visitor) const;

	protected:
		const_Type_ptr type;
		std::string name;
		Expr_ptr expression;
	};

	/**
	 * Dummy node class for searching.
	 */
	class SearchNode : public Node {
	public:
		SearchNode(const std::string& name)
			: Node(BuiltInType::undefined, name) {}
		void accept(Visitor *visitor) const
			{}
	};
}	// End of namespace Core

#endif
