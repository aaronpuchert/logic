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
#include "base.hpp"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <initializer_list>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Theories are collections of objects like nodes, statements and rules.
	 *
	 * The objects are arranged in a list. To add an object, one has to have
	 * an iterator into that list after which to insert. Theories are
	 * searchable by name.
	 */
	class Theory {
	public:
		// Types for iterating through theories
		using iterator = std::list<Object_ptr>::iterator;
		using const_iterator = std::list<Object_ptr>::const_iterator;

		Theory(Theory *parent = nullptr, iterator parent_object = iterator());
		Theory(std::initializer_list<Object_ptr> objects);
		Theory(const Theory &theory);
		Theory(Theory &&theory) = default;

		// Iterate through objects
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		// Add and get objects: declarations, definitions, and statements.
		iterator add(Object_ptr object, iterator after);
		const_iterator get(const std::string& reference) const;

		// Miscellaneous
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
		bool verify() const;

		const Theory *parent;
		const iterator parent_object;

	private:
		// Dependencies?
		std::list<Object_ptr> objects;
		struct NodeCompare : public std::binary_function<Object *, Object *, bool> {
			bool operator ()(const Object *, const Object *) const;
		};
		std::map<Object *, iterator, NodeCompare> name_space;
	};

	/**
	 * Class for statements.
	 */
	class Statement : public Node {
	public:
		Statement(const std::string &name, Expr_ptr expr);
		Object_ptr clone() const;

		/**
		 * Does the statement have a proof?
		 *
		 * @return True, if it has a proof.
		 */
		bool hasProof() const
			{return (bool)proof;}

		/**
		 * Get proof of the statement, not necessarily valid.
		 *
		 * @return Proof of the statement.
		 */
		const_Proof_ptr getProof() const
			{return proof;}

		void addProof(Proof_ptr proof);

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		Proof_ptr proof;
	};

	/**
	 * References to objects.
	 * (They should be used for statements only, shouldn't they?)
	 */
	class Reference {
	public:
		Reference(const Theory *theory, Theory::const_iterator it);
		Reference(const Theory *this_theory, Theory::const_iterator this_it,
			const std::string &description);
		std::string getDescription(const Theory *this_theory,
			Theory::const_iterator this_it) const;

		/**
		 * Resolve reference.
		 *
		 * @return Statement the reference points to.
		 */
		const_Object_ptr operator *() const
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
		virtual ~Proof() {}

		/**
		 * Does the proof prove a statement?
		 *
		 * @param statement Statement to verify.
		 * @return True, if the statement could be verified by the proof.
		 */
		virtual bool proves(const Statement &statement) const = 0;

		/**
		 * Accept a Visitor.
		 *
		 * @param visitor Visitor object on which to call the visit() method.
		 */
		virtual void accept(Visitor *visitor) const = 0;
	};

	/**
	 * Class for a single proof step
	 */
	class ProofStep : public Proof {
	public:
		ProofStep(const_Rule_ptr rule,
			const std::vector<Expr_ptr> &var_list,
			std::vector<Reference> &&statement_list);

		/**
		 * Get the rule used in this proof step.
		 *
		 * @return Rule used in the proof step.
		 */
		const_Rule_ptr getRule() const
			{return rule;}

		const_Expr_ptr operator[](const_Node_ptr node) const;

		/**
		 * Get a vector of the references used.
		 *
		 * @return References used in application of the rule.
		 */
		const std::vector<Reference>& getReferences() const
			{return ref_statement_list;}

		bool proves(const Statement &statement) const;
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const_Rule_ptr rule;
		Context subst;
		std::vector<Reference> ref_statement_list;
	};
}	// End of namespace Core

#endif
