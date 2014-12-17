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
#include <sstream>
using namespace Core;

/**
 * Compare function object for nodes, similar to std::less.
 */
bool Theory::NodeCompare::operator ()(const Node *x, const Node *y) const
{
	return (x->getName() < y->getName());
}

/**
 * Construct a theory.
 * @param parent The parent theory, if this is a subtheory, otherwise nullptr.
 */
Theory::Theory(Theory* parent) : parent(parent) {}

/**
 * @brief Add node to theory.
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
		name_space[object.get()] = position;
		return position;
	}
	else
		throw NamespaceException(NamespaceException::DUPLICATE,
			object->getName());
}

/**
 * @brief Get the object having a specific name.
 * @param reference Identifier to search for.
 * @return Pointer to the node or nullptr, if no such node exists.
 */
Theory::iterator Theory::get(const std::string& reference) const
{
	// construct a dummy node
	SearchNode node(reference);

	auto entry = name_space.find(&node);
	if (entry != name_space.end())
		return entry->second;
	else if (parent != nullptr)
		return parent->get(reference);
	else
		return entry->second;
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
 * verify the theory.
 */
bool Theory::verify()
{
	//
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

	return str.str().c_str();
}

/**
 * Add a proof to a statement.
 */
bool Statement::addProof(Proof_ptr proof)
{
	//
}

/**
 * Initialize a proof step.
 */
ProofStep::ProofStep(Theory *System, const std::string &rule_name,
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
