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
#include "logic.hpp"
#include <sstream>
#include <algorithm>
using namespace Core;

/**
 * Compare function object for nodes, similar to std::less.
 */
bool Theory::NodeCompare::operator ()(const Node *x, const Node *y) const
{
	return (x->getName() < y->getName());
}

/**
 * Construct a root theory.
 */
Theory::Theory() : parent(nullptr) {}

/**
 * Construct a theory.
 * @param parent The parent theory, if this is a subtheory, otherwise nullptr.
 * @param parent_node Iterator to the node referring to this theory.
 */
Theory::Theory(Theory* parent, iterator parent_node)
	: parent(parent), parent_node(parent_node) {}

/**
 * Construct a theory from a list of nodes.
 * @param nodes List of nodes.
 */
Theory::Theory(std::initializer_list<Node_ptr> nodes)
{
	iterator it = begin();
	for (Node_ptr node : nodes)
		it = add(node, it);
}

/**
 * Add node to theory.
 * @param object Object to add.
 * @param after Iterator pointing to the node after which to insert.
 * @return Iterator to the newly inserted object.
 * @throw Core::NamespaceException if an object of the same name already exists.
 */
Theory::iterator Theory::add(Node_ptr object, iterator after)
{
	auto entry = name_space.find(object.get());
	if (entry == name_space.end()) {
		iterator position = nodes.insert(++after, object);
		if (object->getName() != "")
			name_space[object.get()] = position;
		return position;
	}
	else
		throw NamespaceException(NamespaceException::DUPLICATE,
			object->getName());
}

/**
 * Get the object having a specific name.
 * @param reference Identifier to search for.
 * @return Iterator to the node or to the end, if no such node exists.
 */
Theory::const_iterator Theory::get(const std::string& reference) const
{
	// construct a dummy node
	SearchNode node(reference);

	auto entry = name_space.find(&node);
	if (entry != name_space.end())
		return entry->second;
	else if (parent != nullptr)
		return parent->get(reference);
	else
		return nodes.end();
}

Theory::iterator Theory::begin()
{
	return nodes.begin();
}

Theory::iterator Theory::end()
{
	return nodes.end();
}

Theory::const_iterator Theory::begin() const
{
	return nodes.begin();
}

Theory::const_iterator Theory::end() const
{
	return nodes.end();
}

/**
 * Verify the theory.
 * @return True, when the theory verifies, false if it doesn't.
 */
bool Theory::verify() const
{
	return std::all_of(nodes.begin(), nodes.end(), [] (Node_ptr node) -> bool {
		if (node->getType() == BuiltInType::statement) {
			auto stmt = std::dynamic_pointer_cast<Statement>(node);
			if (stmt && stmt->hasProof())
				return stmt->getProof()->proves(*stmt);
		}
		return true;
	});
}

NamespaceException::NamespaceException(Reason reason, const std::string &name)
	: reason(reason), name(name) {}

/**
 * Return exception description.
 */
const char* NamespaceException::what() const noexcept
{
	std::ostringstream str;

	switch (reason) {
	case NOTFOUND:
		str << "Did not find symbol: " << name;
		break;
	case DUPLICATE:
		str << "Duplicate symbol: " << name;
		break;
	}

	description = str.str();
	return description.c_str();
}

/**
 * Construct a statement node.
 * @method Statement::Statement
 * @param name Name of the statement.
 * @param expr Contents of the statement.
 */
Statement::Statement(const std::string &name, Expr_ptr expr)
	: Node(BuiltInType::statement, name)
{
	const_Type_ptr type = expr->getType();
	if (type == BuiltInType::statement)
		setDefinition(expr);
	else
		throw TypeException(type, BuiltInType::statement);
}

/**
 * Clone statement object
 * @method Statement::clone
 * @return Pointer to new statement object.
 */
Node_ptr Statement::clone() const
{
	return std::make_shared<Statement>(*this);
}

/**
 * Add a proof to a statement.
 * @method Statement::addProof
 * @param proof Pointer to the proof to be added.
 * @return Return if the proof works.
 */
bool Statement::addProof(Proof_ptr proof)
{
	this->proof = proof;
	return proof->proves(*this);
}

/**
 * Construct a reference.
 * @param theory Theory which contains the referenced node.
 * @param it Iterator pointing to the referenced node.
 */
Reference::Reference(const Theory *theory, Theory::const_iterator it)
	: theory(theory), ref(it) {}

/**
 * Create description of reference.
 * @param this_theory Theory which contains `this`.
 * @param this_it Iterator to `this`.
 */
std::string Reference::getDescription(const Theory *this_theory,
	Theory::const_iterator this_it) const
{
	// Does the referred statement have a name?
	if ((*ref)->getName() != "")
		return (*ref)->getName();

	// What about name~n?

	// Try to find the referenced statement
	Theory::const_iterator level_head(this_it);
	int level_val = 0;
	int diff;

	for (const Theory *level = this_theory; level; level = level->parent) {
		for (diff = 0; (level_head != level->end())
			&& (level_head != ref); ++diff) --level_head;

		// level up
		if (level->parent && level_head != ref) {
			level_head = level->parent_node;
			++level_val;
		}
		else
			break;
	}

	// What should we do if we don't find anything:
	// 1. Provide a fallback method or
	// 2. Use some kind of hash?

	// Found it? Return <base>~<diff>.
	std::ostringstream stream;
	if (level_val) {
		if (level_val > 1)
			stream << "parent^" << level_val;
		else
			stream << "parent";
	}
	else
		stream << "this";
	stream << '~' << diff;
	return stream.str();
}

/**
 * Wind back a reference.
 * @param diff Number of nodes to go back.
 */
Reference& Reference::operator -=(int diff)
{
	while (diff--)
		--ref;
	return *this;
}

/**
 * Get a reference to the node which is `back` before `a`.
 */
Reference Core::operator -(const Reference& a, int back)
{
	Reference res(a);
	res -= back;
	return res;
}

/**
 * Compute the positive distance between two references, assuming a<b.
 * @return The nonnegative difference or -1, if `a` doesn't precede `b` in this theory.
 */
int Core::operator -(const Reference& a, const Reference& b)
{
	if (a.theory != b.theory)
		return -1;

	int diff = 0;
	for (Theory::const_iterator it = a.ref; it != b.ref; ++it)
		++diff;
	return diff;
}


/**
 * Initialize a proof step.
 * @param system Theory containing the desired rule.
 * @param rule_name Name of the desired rule.
 * @param var_list List of expressions which substitute the rule variables.
 * @param statement_list List of statements referenced.
 */
ProofStep::ProofStep(Theory *system, const std::string &rule_name,
	std::vector<Expr_ptr> &&var_list,
	std::vector<Reference> &&statement_list)
	: var_list(std::move(var_list)), ref_statement_list(std::move(statement_list))
{
	Theory::const_iterator rule_it = system->get(rule_name);
	if (rule_it != system->end() && (*rule_it)->getType() == BuiltInType::rule)
		rule = std::static_pointer_cast<Rule>(*rule_it).get();
	else
		return; 	// TODO: exception
}

/**
 * Does the proof step prove a certain statement?
 * @method ProofStep::proves
 * @param statement Statement to prove
 * @return True, if the statement can be proven this way.
 */
bool ProofStep::proves(const Statement &statement) const
{
	return rule->validate(var_list, ref_statement_list, statement.getDefinition());
}

/**
 * Does this long proof prove a certain statement?
 */
bool LongProof::proves(const Statement &statement) const
{
	// Is the theory valid?
	if (!subTheory.verify())
		return false;

	// Is the last node a statement and the same as ours?
	// TODO: compare
	return true;
}
