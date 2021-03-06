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
#include "tree.hpp"
#include <vector>
#include <string>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract base class for logical rules
	 */
	class Rule : public Object {
	public:
		const std::vector<Node_ptr>& getParams() const
			{return params;}
		bool validate(const Context &context,
			const std::vector<Reference> &statements, const_Expr_ptr statement) const;

	protected:
		/**
		 * Construct a rule.
		 *
		 * @param name Name of the rule.
		 * @param params Parameter list of the rule.
		 */
		Rule(const std::string& name, std::vector<Node_ptr> &&params)
			: Object(BuiltInType::rule, name), params(std::move(params)) {}

	private:
		virtual bool validate_pass(const Context &context,
			const std::vector<Reference> &statements, const_Expr_ptr statement) const = 0;

		const std::vector<Node_ptr> params;
	};

	/**
	 * Tautology rule.
	 */
	class Tautology : public Rule {
	public:
		Tautology(const std::string& name, std::vector<Node_ptr> &&params, Expr_ptr tautology);
		Object_ptr clone() const;

		/**
		 * Get statement that we can assume to be true by this rule.
		 *
		 * @return Tautological statement expression.
		 */
		const_Expr_ptr getStatement() const
			{return subst.getExpr();}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		bool validate_pass(const Context &context,
			const std::vector<Reference> &statements, const_Expr_ptr statement) const;

		mutable Substitution subst;
	};

	/**
	 * Equivalence rule.
	 */
	class EquivalenceRule : public Rule {
	public:
		EquivalenceRule(const std::string& name, std::vector<Node_ptr> &&params,
			Expr_ptr statement1, Expr_ptr statement2);
		Object_ptr clone() const;

		/**
		 * Get first statement of the equivalence.
		 *
		 * @return Statement expression.
		 */
		const_Expr_ptr getStatement1() const
			{return subst1.getExpr();}

		/**
		 * Get second statement of the equivalence.
		 *
		 * @return Statement expression.
		 */
		const_Expr_ptr getStatement2() const
			{return subst2.getExpr();}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		bool validate_pass(const Context &context,
			const std::vector<Reference> &statements, const_Expr_ptr statement) const;

		mutable Substitution subst1, subst2;
	};

	/**
	 * Deduction rule.
	 */
	class DeductionRule : public Rule {
	public:
		DeductionRule(const std::string& name, std::vector<Node_ptr> &&params,
			const std::vector<Expr_ptr> &premisses, Expr_ptr conclusion);
		Object_ptr clone() const;

		const std::vector<const_Expr_ptr>& getPremisses() const;
		/**
		 * Get conclusion of the deduction rule.
		 *
		 * @return Conclusion expression.
		 */
		const_Expr_ptr getConclusion() const
			{return subst_conclusion.getExpr();}

		void accept(Visitor *visitor) const
			{visitor->visit(this);}

	private:
		bool validate_pass(const Context &context,
			const std::vector<Reference> &statements, const_Expr_ptr statement) const;

		std::vector<Expr_ptr> premisses;
		mutable std::vector<Substitution> subst_premisses;
		mutable Substitution subst_conclusion;
	};
}	// End of namespace Core

#endif
