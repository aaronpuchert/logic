/*
 *   Logical systems and their rules.
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

#ifndef CORE_LOGIC_HPP
#define CORE_LOGIC_HPP
#include "forward.hpp"
#include "theory.hpp"
#include <vector>
#include <string>
#include <memory>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract base class for logical rules
	 */
	class Rule : public Node {
	public:
		Rule(const std::string& name, Theory &&params)
			: Node(name, Node::RULE), params(std::move(params)) {}
		const Theory* getVars() const
			{return &params;}

		virtual bool validate(const std::vector<Expr_ptr> &substitutes,
			const std::vector<Expr_ptr> &statements) = 0;

	protected:
		Theory params;
	};

	/**
	 * Tautology rule.
	 */
	class Tautology : public Rule {
	public:
		Tautology(const std::string& name, Theory &&params, Expr_ptr statement)
			: Rule(name, std::move(params)), statement(statement) {}
		const_Expr_ptr getStatement() const
			{return statement;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		bool validate(const std::vector<Expr_ptr> &substitutes,
			const std::vector<Expr_ptr> &statements);

	protected:
		Expr_ptr statement;
	};

	/**
	 * Equivalence rule.
	 */
	class EquivalenceRule : public Rule {
	public:
		EquivalenceRule(const std::string& name, Theory &&params,
			Expr_ptr statement1, Expr_ptr statement2)
			: Rule(name, std::move(params)),
				statement1(statement1), statement2(statement2) {}
		const_Expr_ptr getStatement1() const
			{return statement1;}
		const_Expr_ptr getStatement2() const
			{return statement2;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		bool validate(const std::vector<Expr_ptr> &substitutes,
			const std::vector<Expr_ptr> &statements);

	protected:
		Expr_ptr statement1, statement2;
	};

	/**
	 * Deduction rule.
	 */
	class DeductionRule : public Rule {
	public:
		DeductionRule(const std::string& name, Theory &&params,
			const std::vector<Expr_ptr> &premisses, Expr_ptr conclusion)
			: Rule(name, std::move(params)), premisses(premisses),
				conclusion(conclusion) {}
		const std::vector<Expr_ptr> &prem() const
			{return premisses;}
		const_Expr_ptr getConclusion() const
			{return conclusion;}
		void accept(Visitor *visitor) const
			{visitor->visit(this);}

		bool validate(const std::vector<Expr_ptr> &substitutes,
			const std::vector<Expr_ptr> &statements);

	protected:
		std::vector<Expr_ptr> premisses;
		Expr_ptr conclusion;
	};
}	// End of namespace Core

#endif
