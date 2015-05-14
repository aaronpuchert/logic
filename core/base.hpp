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
		enum Class {BUILTINTYPE, ATOMIC, LAMBDACALL, NEGATION, CONNECTIVE, QUANTIFIER,
			LAMBDATYPE, LAMBDA} const cls;

		virtual ~Expression() {}

		/**
		 * Accept a Visitor.
		 *
		 * @param visitor Visitor object on which to call the visit() method.
		 */
		virtual void accept(Visitor *visitor) const = 0;

		/**
		 * Get type of expression.
		 *
		 * @return Type of expression.
		 */
		virtual const_Expr_ptr getType() const = 0;

	protected:
		/**
		 * Construct expression of certain class.
		 */
		Expression(Class cls) : cls(cls) {}
	};

	/**
	 * Built-in standard types.
	 */
	class BuiltInType : public Expression {
	public:
		enum Variant {UNDEFINED, TYPE, STATEMENT, RULE}
			const variant;

		BuiltInType(Variant variant);
		const_Expr_ptr getType() const;
		void accept(Visitor *visitor) const;

		// Global standard types
		static const const_Expr_ptr
			type, statement, rule, undefined;
	};

	/**
	 * Lambda type.
	 */
	class LambdaType : public Expression {
	public:
		LambdaType(std::vector<const_Expr_ptr> &&args,
			const_Expr_ptr return_type = BuiltInType::statement);
		const_Expr_ptr getType() const;
		const_Expr_ptr getReturnType() const;

		// Iterate over argument types...
		typedef std::vector<const_Expr_ptr>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		void accept(Visitor *visitor) const;

	private:
		const_Expr_ptr return_type;
		std::vector<const_Expr_ptr> args;
	};

	/**
	 * Type comparison class, unfortunately not thread-safe.
	 */
	class TypeComparator : public Visitor {
	public:
		TypeComparator(const Context *context = nullptr) : context(context) {}
		bool operator()(const Expression *, const Expression *);
		void visit(const BuiltInType *type);
		void visit(const LambdaType *type);
		void visit(const AtomicExpr *type);

	private:
		const Context *context;
		int yours;
		std::vector<const void *> description[2];
	};

	/**
	 * Abstract base class for named entities in theories: types, variables,
	 * predicates, statements.
	 */
	class Node {
	public:
		Node(const_Expr_ptr type, const std::string &name);
		virtual ~Node() {}
		virtual Node_ptr clone() const;

		/**
		 * Get type of node.
		 *
		 * @return Type of node.
		 */
		const_Expr_ptr getType() const
			{return type;}

		/**
		 * Get name of node.
		 *
		 * @return Name of node.
		 */
		const std::string &getName() const
			{return name;}

		void setDefinition(Expr_ptr new_expression);

		/**
		 * Get definition expression of the node, if there is any.
		 *
		 * @return Definition expression, or an invalid pointer, if there is none.
		 */
		const_Expr_ptr getDefinition() const
			{return expression;}

		virtual void accept(Visitor *visitor) const;

	protected:
		const_Expr_ptr type;
		const std::string name;
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
