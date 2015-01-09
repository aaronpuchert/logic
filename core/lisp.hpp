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
#include <deque>
#include <sstream>
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
		const std::string& getContent() const;

	private:
		Type type;
		std::string content;
	};

	/**
	 * Lisp-syntax writer class
	 */
	class Writer : public Visitor {
	public:
		Writer(std::ostream &output, int line_length = 80,
			int tab_size = 4, bool tabs = true);
		~Writer();
		void visit(const Node *node);
		void visit(const BuiltInType *type);
		void visit(const VariableType *type);
		void visit(const LambdaType *type);
		void visit(const AtomicExpr *expression);
		void visit(const LambdaCallExpr *expression);
		void visit(const NegationExpr *expression);
		void visit(const ConnectiveExpr *expression);
		void visit(const QuantifierExpr *expression);
		void visit(const LambdaExpr *expression);
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

		// Add tokens
		void addParanthesis(Change depth_change);
		void addToken(const std::string &token);
		void addToken(std::string &&token);
		void push(LispToken &&token);

		// Write to stream
		void writeQueue();
		void writeLine(int num_tokens);
		void writeToken(LispToken token);
		int tokenLength(int index);

		std::ostream &output;
		std::deque<LispToken> token_queue;
		int depth;

		// For pretty printing
		int max_line_length;
		int line_length;
		int tab_size;
		bool tabs;
		int write_depth;
		std::stack<int> length;

		// Keeping track of where we are
		std::stack<const Theory *> theory_stack;
		std::stack<Theory::const_iterator> iterator_stack;
	};

}	// End of namespace Core

#endif
