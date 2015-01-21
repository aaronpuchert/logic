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
#include "expression.hpp"
#include <string>
#include <stack>
#include <deque>
#include <sstream>
#include "debug.hpp"
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
	 * Lisp-syntax lexer class.
	 */
	class Lexer {
	public:
		Lexer(std::istream &input);
		LispToken getToken();
		int getLine() const {return line_number;}
		int getColumn() const {return column_number;}

	private:
		void nextChar();
		void skipLine();

		std::istream &input;

		// State
		int last;
		int line_number;
		int column_number;
	};

	/**
	 * Parser error-handling.
	 */
	class ParserErrorHandler {
	public:
		// Problem level
		enum Level {ERROR, WARNING, NOTE};

		ParserErrorHandler(const Lexer &lexer, std::ostream &output, const std::string &descriptor);
		~ParserErrorHandler();

		// Formatting operators
		ParserErrorHandler& operator <<(Level level);
		ParserErrorHandler& operator <<(const std::string &str);
		ParserErrorHandler& operator <<(LispToken::Type type);
		ParserErrorHandler& operator <<(const_Type_ptr type);

		// Get statistics
		int getErrors() const {return error_count;}
		int getWarnings() const {return warning_count;}

	private:
		const Lexer &lexer;
		std::string descriptor;
		int error_count;
		int warning_count;

		std::ostream &output;
		TypeWriter writer;
	};

	/**
	 * Lisp-Syntax parser class
	 */
	class Parser {
	public:
		Parser(std::istream &input, std::ostream &output,
			const std::string &descriptor);

		Node_ptr parseNode();
		const_Type_ptr parseType();
		Type_ptr parseLambdaType();

		Expr_ptr parseExpression();
		Expr_ptr parseAtomicExpr();
		Expr_ptr parseLambdaCallExpr();
		Expr_ptr parseNegationExpr();
		Expr_ptr parseConnectiveExpr();
		Expr_ptr parseQuantifierExpr();
		Expr_ptr parseLambda();

		Node_ptr parseTautology();
		Node_ptr parseEquivalenceRule();
		Node_ptr parseDeductionRule();

		Node_ptr parseStatement();
		// Reference, ProofStep, LongProof
		Theory parseTheory();

		// Get statistics
		int getErrors() const {return error_output.getErrors();}
		int getWarnings() const {return error_output.getWarnings();}

	private:
		void nextToken();
		bool expect(LispToken::Type type);
		const_Node_ptr getIdentifier();
		void recover();

		// Our lexer object
		Lexer lexer;
		// Where we write errors
		ParserErrorHandler error_output;

		// The current token and theory stack
		LispToken token;
		std::stack<Theory *> stack;

		// Dispatch tables
		static const std::map<std::string, Node_ptr (Parser::*)()> node_dispatch;
		static const std::map<std::string, Expr_ptr (Parser::*)()> expr_dispatch;
		static const std::map<std::string, ConnectiveExpr::Variant> connective_dispatch;

		// Dummy objects to use in the case of errors
		static const Node_ptr undefined_node;
		static const Expr_ptr undefined_expr;
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
