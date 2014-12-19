/*
 *   Data structures for theories.
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

#ifndef CORE_THEORY_HPP
#define CORE_THEORY_HPP
#include "forward.hpp"
#include "expression.hpp"
#include "logic.hpp"
#include <string>
#include <vector>
#include <list>
#include <map>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Theory class
	 */
	class Theory {
	public:
		// Types for iterating through theories
		using iterator = std::list<Node_ptr>::iterator;
		using const_iterator = std::list<Node_ptr>::const_iterator;

		Theory();
		Theory(Theory *parent, iterator parent_node);

		// Iterate through nodes
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		// Add and get nodes: declarations, definitions, and statements.
		iterator add(Node_ptr object, iterator after);
		iterator get(const std::string& reference) const;

		// Miscellaneous
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
		bool verify();

		const Theory *parent;
		const iterator parent_node;

	private:
		// Dependencies?
		std::list<Node_ptr> nodes;
		struct NodeCompare : public std::binary_function<Node *, Node *, bool> {
			bool operator ()(const Node *, const Node *) const;
		};
		std::map<Node *, iterator, NodeCompare> name_space;
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

	/**
	 * Class for statements.
	 */
	class Statement : public Node {
	public:
		enum StatementType {AXIOM, STATEMENT};

		Statement(const std::string &name, Expr_ptr expr)
			: Node(name, Node::STATEMENT), expr(expr) {}
		StatementType getType() const
			{return proof ? STATEMENT : AXIOM;}
		const_Expr_ptr getStatement() const
			{return expr;}

		const_Proof_ptr getProof() const
			{return proof;}
		bool addProof(Proof_ptr proof);

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Expr_ptr expr;
		Proof_ptr proof;
	};

	/**
	 * Class for references
	 */
	class Reference {
	public:
		Reference(const Theory *theory, Theory::const_iterator it);
		std::string getDescription(const Theory *this_theory,
			Theory::const_iterator this_it) const;

		// Resolve reference
		const_Node_ptr operator *() const
			{return *ref;}
		Reference& operator -=(int diff);

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		friend Reference operator -(const Reference&, int);
		friend int operator -(const Reference&, const Reference&);
		friend bool operator ==(const Reference& a, const Reference& b)
			{return (a-b) == 0;}

	private:
		// Where do we find the object?
		const Theory *theory;
		// Reference to a node in this theory
		Theory::const_iterator ref;
	};


	/**
	 * Abstract base class for proofs.
	 */
	class Proof {
	public:
		virtual bool proves(const Statement &statement) = 0;
		virtual void accept(Visitor *visitor) const = 0;
	};

	/**
	 * Class for a single proof step
	 */
	class ProofStep : public Proof {
	public:
		ProofStep(Theory *system, const std::string &rule_name,
			std::vector<Expr_ptr> &&var_list,
			std::vector<Reference> &&statement_list);
		const Rule *getRule() const
			{return rule;}
		const std::vector<Expr_ptr>& getVars() const
			{return var_list;}
		const std::vector<Reference> getReferences() const
			{return ref_statement_list;}
		bool proves(const Statement &statement);
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const Rule *rule;
		std::vector<Expr_ptr> var_list;
		std::vector<Reference> ref_statement_list;
	};

	/**
	 * Class for more difficult proofs.
	 */
	class LongProof : public Proof {
	public:
		LongProof(Theory *parent, Theory::iterator parent_node)
			: subTheory(parent, parent_node) {}

		bool proves(const Statement &statement);
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		Theory subTheory;
	};
}	// End of namespace Core

#endif
