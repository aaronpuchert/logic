/*
 *   Forward declaration of several classes.
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

/**
 * Namespace for logic core
 */
namespace Core {
	// type.hpp
	class Type;

	// atom.hpp
	class Atom;
	class Variable;

	// expression.hpp
	class Predicate;
	class Expression;
	class AtomicExpr;
	class PredicateExpr;
	class NegationExpr;
	class ConnectiveExpr;
	class QuantifierExpr;

	// logic.hpp
	class Rule;
	class Tautology;
	class EquivalenceRule;
	class DeductionRule;

	// theory.hpp
	class Statement;
	class Proof;
	class ProofStep;
	class LongProof;
	class Theory;

	// namespace.hpp
	template<typename T> class Namespace;
	typedef Namespace<Rule> LogicSystem;

	// traverse.hpp
	class Visitor;
}	// End of namespace Core
