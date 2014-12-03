/*
 *   Visitor infrastructure for traversing the (syntax?) tree.
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

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Abstract class for a visitor traversing the syntax tree.
	 */
	class Visitor {
	public:
		virtual ~Visitor() {}
		virtual void visit(const Type *type);
		virtual void visit(const Variable *variable);
		virtual void visit(const Predicate *predicate);
		virtual void visit(const AtomicExpr *expression);
		virtual void visit(const PredicateExpr *expression);
		virtual void visit(const NegationExpr *expression);
		virtual void visit(const ConnectiveExpr *expression);
		virtual void visit(const QuantifierExpr *expression);
		virtual void visit(const Tautology *rule);
		virtual void visit(const EquivalenceRule *rule);
		virtual void visit(const DeductionRule *rule);
		virtual void visit(const Statement *statement);
		virtual void visit(const ProofStep *proofstep);
		virtual void visit(const LongProof *longproof);
		virtual void visit(const Theory *theory);
	};
}	// End of namespace Core
