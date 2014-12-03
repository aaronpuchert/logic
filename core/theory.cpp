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

#include "theory.hpp"
using namespace Core;

/**
 * Add a proof to a statement.
 */
bool Statement::addProof(Proof_ptr proof)
{
	//
}

/**
 * Add a statement to a theory at a specific position.
 */
void Theory::addStatement(const Statement &statement, unsigned index)
{
	//
}

void Theory::addStatement(Statement &&statement, unsigned index)
{
	//
}

/**
 * Get the statement at a specific position.
 */
const Statement& Theory::getStatement(unsigned index)
{
	//
}

/**
 * verify the theory.
 */
bool Theory::verify()
{
	//
}

/**
 * Initialize a proof step.
 */
ProofStep::ProofStep(LogicSystem System, const std::string &rule_name,
	const std::vector<Expr_ptr> var_list,
	const std::vector<const Statement*> statement_list)
{
	//
}

/**
 * Does the proof step prove a certain statement?
 */
bool ProofStep::proves(const Statement &statement)
{
	//
}

/**
 * Does this long proof prove a certain statement?
 */
bool LongProof::proves(const Statement &statement)
{
	//
}
