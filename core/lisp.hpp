/*
 *   Classes concerning the textual represantation of our data.
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

#ifndef CORE_LISP_HPP
#define CORE_LISP_HPP
#include "forward.hpp"
#include "theory.hpp"
#include <string>
#include <stack>
#include <queue>
#include <sstream>
#include <memory>
#include "traverse.hpp"

/**
 * Namespace for logic core
 */
namespace Core {
	/**
	 * Lisp-syntax tokens.
	 */
	class LispToken {
	public:
		enum Type {WORD, OPENING, CLOSING, ENDOFFILE};
		LispToken(Type type) : type(type) {}
		LispToken(Type type, const std::string &content)
			: type(type), content(content) {}
		LispToken(Type type, std::string &&content)
			: type(type), content(std::move(content)) {}

		Type getType() const
			{return type;}
		std::string getContent() const;

	private:
		Type type;
		std::string content;
	};

	/**
	 * Lisp-syntax writer class
	 */
	class Writer : public Visitor {
	public:
		Writer(std::ostream &output);
		~Writer();
		void visit(const Type *type);
		void visit(const Variable *variable);
		void visit(const PredicateDecl *predicate);
		void visit(const PredicateLambda *predicate);
		void visit(const PredicateDef *predicate);
		void visit(const AtomicExpr *expression);
		void visit(const PredicateExpr *expression);
		void visit(const NegationExpr *expression);
		void visit(const ConnectiveExpr *expression);
		void visit(const QuantifierExpr *expression);
		void write_varlist(const Rule *rule);
		void visit(const Tautology *rule);
		void visit(const EquivalenceRule *rule);
		void visit(const DeductionRule *rule);
		void visit(const Statement *statement);
		void visit(const Reference *reference);
		void visit(const ProofStep *proofstep);
		void visit(const LongProof *longproof);
		void visit(const Theory *theory);

	private:
		enum Change {OPENING = +1, CLOSING = -1};
		void addParanthesis(Change depth_change);
		void addToken(const std::string &token);
		void addToken(std::string &&token);
		void writeQueue();

		std::ostream &output;
		std::queue<LispToken> token_queue;
		int depth;

		// Keeping track of where we are
		std::stack<const Theory *> theory_stack;
		std::stack<Theory::const_iterator> iterator_stack;
	};

}	// End of namespace Core

#endif
