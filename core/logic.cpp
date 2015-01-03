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
using namespace Core;

/**
 * Construct a tautology: such a rule states that a certain statement is always true.
 * @param name Name of the rule.
 * @param params Parameters of the rule.
 * @param statement Statement that is always true.
 */
Tautology::Tautology(const std::string& name, Theory &&params, Expr_ptr statement)
	: Rule(name, std::move(params)), statement(statement)
{
	if (statement->getType() != BuiltInType::statement)
		throw TypeException(statement->getType(), BuiltInType::statement);
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
	: Rule(name, std::move(params)), statement1(statement1), statement2(statement2)
{
	if (statement1->getType() != BuiltInType::statement)
		throw TypeException(statement1->getType(), BuiltInType::statement, "first statement");
	if (statement2->getType() != BuiltInType::statement)
		throw TypeException(statement2->getType(), BuiltInType::statement, "second statement");
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
	: Rule(name, std::move(params)), premisses(premisses),
		conclusion(conclusion)
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
}

bool Tautology::validate(const std::vector<Expr_ptr> &substitutes,
	const std::vector<Expr_ptr> &statements)
{
	//
}

bool EquivalenceRule::validate(const std::vector<Expr_ptr> &substitutes,
	const std::vector<Expr_ptr> &statements)
{
	//
}

bool DeductionRule::validate(const std::vector<Expr_ptr> &substitutes,
	const std::vector<Expr_ptr> &statements)
{
	//
}
