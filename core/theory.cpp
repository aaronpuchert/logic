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
#include <algorithm>
#include "debug.hpp"
using namespace Core;

namespace {
	/**
	 * Dummy node class for searching.
	 */
	class SearchObject : public Object {
	public:
		SearchObject(const std::string& name)
			: Object(BuiltInType::undefined, name) {}
		Object_ptr clone() const
			{return std::make_shared<SearchObject>(*this);}
		void accept(Visitor *visitor) const
			{}
	};
}

/**
 * Compare function object for nodes, similar to std::less.
 */
bool Theory::NodeCompare::operator ()(const Object *x, const Object *y) const
{
	return (x->getName() < y->getName());
}

/**
 * Construct a theory.
 *
 * @param parent The parent theory, if this is a subtheory, otherwise nullptr.
 * @param parent_node Iterator to the object referring to this theory.
 */
Theory::Theory(Theory *parent, Theory::iterator parent_object)
	: parent(parent), parent_object(parent_object) {}

/**
 * Construct a theory from a list of nodes.
 *
 * @param nodes List of nodes.
 */
Theory::Theory(std::initializer_list<Object_ptr> objects) : parent(nullptr)
{
	iterator it = begin();
	for (Object_ptr node : objects)
		it = add(node, it);
}

/**
 * Copy construct a theory.
 *
 * @param theory Other theory.
 */
Theory::Theory(const Theory &theory) : parent(nullptr)
{
	iterator it = begin();
	for (const_Object_ptr node : theory)
		it = add(node->clone(), it);
}

/**
 * Add node to theory.
 *
 * @param object Object to add.
 * @param after Iterator pointing to the node after which to insert.
 * @return Iterator to the newly inserted object.
 * @throw Core::NamespaceException if an object of the same name already exists.
 */
Theory::iterator Theory::add(Object_ptr object, iterator after)
{
	auto entry = name_space.find(object.get());
	if (entry == name_space.end()) {
		iterator position = objects.insert(++after, object);
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
 *
 * @param reference Identifier to search for.
 * @return Iterator to the node or to the end, if no such node exists.
 */
Theory::const_iterator Theory::get(const std::string& reference) const
{
	// construct a dummy node
	SearchObject node(reference);

	auto entry = name_space.find(&node);
	if (entry != name_space.end())
		return entry->second;
	else if (parent != nullptr)
		return parent->get(reference);
	else
		return objects.end();
}

Theory::iterator Theory::begin()
{
	return objects.begin();
}

Theory::iterator Theory::end()
{
	return objects.end();
}

Theory::const_iterator Theory::begin() const
{
	return objects.begin();
}

Theory::const_iterator Theory::end() const
{
	return objects.end();
}

/**
 * Verify the theory.
 *
 * @return True, when the theory verifies, false if it doesn't.
 */
bool Theory::verify() const
{
	return std::all_of(objects.begin(), objects.end(), [] (Object_ptr object) -> bool {
		if (object->getType() == BuiltInType::statement) {
			auto stmt = std::dynamic_pointer_cast<Statement>(object);
			if (stmt && stmt->hasProof())
				return stmt->getProof()->proves(*stmt);
		}
		return true;
	});
}

/**
 * Construct a statement node.
 *
 * @param name Name of the statement.
 * @param expr Contents of the statement.
 */
Statement::Statement(const std::string &name, Expr_ptr expr)
	: Node(BuiltInType::statement, name)
{
	const_Expr_ptr type = expr->getType();
	if (type == BuiltInType::statement)
		setDefinition(expr);
	else
		throw TypeException(type, BuiltInType::statement);
}

/**
 * Clone statement object
 *
 * @return Pointer to new statement object.
 */
Object_ptr Statement::clone() const
{
	return std::make_shared<Statement>(*this);
}

/**
 * Add a proof to a statement.
 *
 * @param proof Pointer to the proof to be added.
 */
void Statement::addProof(Proof_ptr proof)
{
	this->proof = proof;
}

/**
 * Construct a reference.
 *
 * @param theory Theory which contains the referenced node.
 * @param it Iterator pointing to the referenced node.
 */
Reference::Reference(const Theory *theory, Theory::const_iterator it)
	: theory(theory), ref(it) {}

/**
 * Construct reference from description string.
 *
 * @param description Description string.
 */
Reference::Reference(const Theory *this_theory, Theory::const_iterator this_it,
	const std::string &description)
{
	std::string base = description;
	int diff = 0;

	// Absolute or relative?
	int pos_tilde = description.find('~');
	if (pos_tilde) {
		// Relative: Use the hierarchy or stack
		base = description.substr(0, pos_tilde);
		std::istringstream str(description.substr(pos_tilde+1));
		str >> diff;
	}

	// We have to find base. Following possibilities:
	// - this
	if (base == "this") {
		theory = this_theory;
		ref = this_it;
	}
	// - parent
	else if (base == "parent") {
		theory = this_theory->parent;
		ref = this_theory->parent_object;
	}
	// - parent^<n>
	else if (base.substr(0, 6) == "parent^") {
		int level;
		std::istringstream str(base.substr(7));
		str >> level;

		// get up
		theory = this_theory;
		ref = this_it;
		while (level--) {
			theory = theory->parent;
			ref = theory->parent_object;
		}
	}
	// - <name>
	else {
		// How do we get base_theory? We might not need it, but this is ugly.
		ref = this_theory->get(base);
	}

	// Now step back...
	while (diff--)
		--ref;
}

/**
 * Create description of reference.
 *
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
			level_head = level->parent_object;
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
 *
 * @param diff Number of nodes to go back.
 */
Reference& Reference::operator -=(int diff)
{
	while (diff--)
		--ref;
	return *this;
}

/**
 * Get a reference to the node which is @a back before @a a.
 *
 * @param a Start reference
 * @param back Number of nodes to go back.
 * @return Reference to node a~back.
 */
Reference Core::operator -(const Reference& a, int back)
{
	Reference res(a);
	res -= back;
	return res;
}

/**
 * Compute the positive distance between two references, assuming a < b.
 *
 * @return The nonnegative difference or -1, if @a a doesn't precede @a b in this theory.
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
 *
 * @param rule Pointer to the rule to be used.
 * @param var_list List of expressions which substitute the rule variables.
 * @param statement_list List of statements referenced.
 */
ProofStep::ProofStep(const_Rule_ptr rule, const std::vector<Expr_ptr>& var_list, std::vector< Reference >&& statement_list )
	: rule(rule), ref_statement_list(std::move(statement_list))
{
	// Do we have a rule?
	if (rule->getType() != BuiltInType::rule)
		throw TypeException(rule->getType(), BuiltInType::rule);

	TypeComparator compare(&subst);

	// Create context and check types
	auto param_it = rule->getParams().begin();
	auto sub_it = var_list.begin();
	for (;  param_it != rule->getParams().end(); ++param_it, ++sub_it) {
		if (!compare((*param_it)->getType().get(), (*sub_it)->getType().get()))
			throw TypeException((*param_it)->getType(), (*sub_it)->getType());
		subst.insert({*param_it, *sub_it});
	}
}

/**
 * Get the substitute of a certain node.
 *
 * @param node Node to look up.
 * @return Expression to be substituted.
 */
const_Expr_ptr ProofStep::operator[](const_Node_ptr node) const
{
	auto find = subst.find(node);
	if (find != subst.end())
		return find->second;
	else
		return const_Expr_ptr();
}

/**
 * Does the proof step prove a certain statement?
 *
 * @param statement Statement to prove
 * @return True, if the statement can be proven this way.
 */
bool ProofStep::proves(const Statement &statement) const
{
	return rule->validate(subst, ref_statement_list, statement.getDefinition());
}
