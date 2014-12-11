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

#pragma once
#include "forward.hpp"
#include "expression.hpp"
#include "namespace.hpp"
#include "logic.hpp"
#include <string>
#include <vector>
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
		Theory(Theory_ptr parent) : types(parent->types),
			predicates(parent->predicates), vars(parent->vars), parent(parent) {}
		void addStatement(const Statement &statement, unsigned index = -1);
		void addStatement(Statement &&statement, unsigned index = -1);
		const Statement& getStatement(unsigned index = -1);
		void accept(Visitor *visitor) const
			{visitor->visit(this);}
		bool verify();

		// Declared names
		Namespace<Type> types;
		Namespace<Predicate> predicates;
		Namespace<Variable> vars;

	protected:
		Theory_ptr parent;
		// Dependencies?
		std::vector<Statement> statements;
	};

	/**
	 * Class for statements.
	 */
	class Statement {
	public:
		enum Type {AXIOM, STATEMENT};

		Statement(const std::string &name, Expr_ptr expr)
			: name(name), expr(expr) {}
		Type getType() const
			{return proof ? STATEMENT : AXIOM;}
		const_Expr_ptr getStatement() const
			{return expr;}
		const_Proof_ptr getProof() const
			{return proof;}
		bool addProof(Proof_ptr proof);

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		std::string name;
		Expr_ptr expr;
		Proof_ptr proof;
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
		ProofStep(LogicSystem System, const std::string &rule_name,
			const std::vector<Expr_ptr> var_list,
			const std::vector<const Statement *> statement_list);
		const Rule *getRule() const
			{return rule;}
		const std::vector<Expr_ptr>& getVars() const
			{return var_list;}
		const std::vector<const Statement*> getReferences() const
			{return ref_statement_list;}
		bool proves(const Statement &statement);
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		const Rule *rule;
		std::vector<Expr_ptr> var_list;
		std::vector<const Statement*> ref_statement_list;
	};

	/**
	 * Class for more difficult proofs.
	 */
	class LongProof : public Proof {
	public:
		LongProof(Theory_ptr parent) : subTheory(parent) {}

		bool proves(const Statement &statement);
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		Theory subTheory;
	};
}	// End of namespace Core
