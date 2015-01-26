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

#include "logic.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>
#include "debug.hpp"
using namespace Core;

/**
 * Validate the application of a Rule.
 * @method Rule::validate
 * @param  context        Arguments for the parameters of the Expression.
 * @param  statements     References to the statements needed.
 * @param  statement      Statement that was deduced.
 * @return                True, if the statement can be deduced in this way.
 */
bool Rule::validate(const Context &context,
	const std::vector<Reference> &statements, const_Expr_ptr statement) const
{
	return validate_pass(context, statements, statement);
}

/**
 * Construct a tautology: such a rule states that a certain statement is always true.
 * @param name Name of the rule.
 * @param params Parameters of the rule.
 * @param statement Statement that is always true.
 */
Tautology::Tautology(const std::string& name, Theory &&params, Expr_ptr tautology)
	: Rule(name, std::move(params)), subst(tautology)
{
	if (tautology->getType() != BuiltInType::statement)
		throw TypeException(tautology->getType(), BuiltInType::statement);
}

/**
 * Clone tautology
 * @method Tautology::clone
 * @return Pointer to new tautology.
 */
Node_ptr Tautology::clone() const
{
	return std::make_shared<Tautology>(*this);
}

bool Tautology::validate_pass(const Context &context,
	const std::vector<Reference> &statements, const_Expr_ptr statement) const
{
	// Check the number of references, it should be 0
	if (statements.size() != 0)
		return false;

	// Check if the statement given is correct
	bool result = subst.check(statement.get(), context);
	return result;
}

/**
 * Construct an equivalence rule: such a rule states that two statements are equivalent.
 * @param name Name of the rule.
 * @param params Parameters of the rule.
 * @param statement1 First statement.
 * @param statement2 Second statement.
 */
EquivalenceRule::EquivalenceRule(const std::string& name, Theory &&params,
	Expr_ptr statement1, Expr_ptr statement2)
	: Rule(name, std::move(params)), subst1(statement1), subst2(statement2)
{
	if (statement1->getType() != BuiltInType::statement)
		throw TypeException(statement1->getType(), BuiltInType::statement, "first statement");
	if (statement2->getType() != BuiltInType::statement)
		throw TypeException(statement2->getType(), BuiltInType::statement, "second statement");
}

/**
 * Clone equivalence rule
 * @method EquivalenceRule::clone
 * @return Pointer to new equivalence rule.
 */
Node_ptr EquivalenceRule::clone() const
{
	return std::make_shared<EquivalenceRule>(*this);
}

bool EquivalenceRule::validate_pass(const Context &context,
	const std::vector<Reference> &statements, const_Expr_ptr statement) const
{
	// Check if we are given one reference
	if (statements.size() != 1)
		return false;

	// Resolve reference
	const_Expr_ptr alt_statement =
		std::static_pointer_cast<const Statement>(*statements[0])->getDefinition();

	// Try substitution both ways
	bool result =
		(subst1.check(alt_statement.get(), context)
			&& subst2.check(statement.get(), context))
		|| (subst1.check(statement.get(), context)
			&& subst2.check(alt_statement.get(), context));
	return result;
}

/**
 * Construct an deduction rule: such a rule states that, given all premisses are
 * true, the conclusion also holds.
 * @param name Name of the rule.
 * @param params Parameters of the rule.
 * @param premisses Vector of premisses.
 * @param statement2 Conclusion statement.
 */
DeductionRule::DeductionRule(const std::string& name, Theory &&params,
	const std::vector<Expr_ptr> &premisses, Expr_ptr conclusion)
	: Rule(name, std::move(params)), premisses(premisses), subst_conclusion(conclusion)
{
	auto find = std::find_if(premisses.begin(), premisses.end(),
		[] (Expr_ptr expr) -> bool {return (expr->getType() != BuiltInType::statement);}
	);

	if (find != premisses.end()) {
		std::ostringstream str;
		str << "premiss number " << find - premisses.begin() + 1;
		throw TypeException((*find)->getType(), BuiltInType::statement, str.str());
	}

	if (conclusion->getType() != BuiltInType::statement)
		throw TypeException(conclusion->getType(), BuiltInType::statement, "conclusion");

	// Build substitution vector
	for (Expr_ptr premiss : premisses)
		subst_premisses.push_back(Substitution(premiss));
}

/**
 * Clone deduction rule
 * @method DeductionRule::clone
 * @return Pointer to new deduction rule.
 */
Node_ptr DeductionRule::clone() const
{
	return std::make_shared<DeductionRule>(*this);
}

/**
 * Get a vector of the premisses
 * @method DeductionRule::getPremisses
 * @return Vector of expressions.
 */
const std::vector<const_Expr_ptr>& DeductionRule::getPremisses() const
{
	// Unfortunately, there is simply no other way.
	const std::vector<const_Expr_ptr> *ptr = reinterpret_cast<const std::vector<const_Expr_ptr> *>(&premisses);
	return *ptr;
}

bool DeductionRule::validate_pass(const Context &context,
	const std::vector<Reference> &statements, const_Expr_ptr statement) const
{
	// Check if we are given the right number of references
	if (statements.size() != subst_premisses.size())
		return false;

	// Check the premisses
	auto mismatch = std::mismatch(subst_premisses.begin(), subst_premisses.end(), statements.begin(),
		[this, context] (Substitution &subst, const Reference ref) -> bool {
			auto stmt = static_cast<const Statement *>((*ref).get());
			return subst.check(stmt->getDefinition().get(), context);
		}
	);

	// Check the conclusion, compute result
	bool result = (mismatch.first == subst_premisses.end())
		&& subst_conclusion.check(statement.get(), context);
	return result;
}
