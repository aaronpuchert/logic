/*
 *   Handling the abstract syntax tree: substitutions etc.
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

#include "../core/logic.hpp"
#include "../core/lisp.hpp"
#include "../core/debug.hpp"
#include <iostream>
#include <fstream>

/**
 * Parser file and return theory.
 * @param filename Name of the file containing the theory.
 * @param num_errors Pointer where the number of errors shall be written to.
 * @param rules Theory containing the rules, when parsing a theory with proofsteps., or nullptr.
 * @return Parsed theory.
 */
Core::Theory parse(const char *filename, int *num_errors, const Core::Theory *rules = nullptr)
{
	std::ifstream file(filename);
	if (!file) {
		*num_errors = -1;
		return Core::Theory();
	}

	Core::Parser parser(file, std::cout, filename);
	if (rules)
		parser.rules = rules;
	Core::Theory res = parser.parseTheory();

	*num_errors = parser.getErrors();
	return res;
}

int main(int argc, char **argv)
{
	if (argc == 1)
		std::cout << "Usage: " << argv[0] << " <theory file> [<rules file>]\n";

	const char *rules_file;
	if (argc >= 3)
		rules_file = argv[2];
	else
		rules_file = "basic/rules.lth";

	// Parse rules
	int err_num;
	Core::Theory rules = parse(rules_file, &err_num, &rules);
	if (err_num) {
		std::cout << "Couldn't parse rules file " << rules_file << std::endl;
		return err_num;
	}

	// Parse file itself
	Core::Theory theory = parse(argv[1], &err_num, &rules);
	if (err_num) {
		std::cout << "Couldn't parse theory file " << argv[1] << std::endl;
		return err_num;
	}

	// Verify the theory
	if (theory.verify())
		std::cout << "Verified theory!\n";
	else
		std::cout << "Couldn't verify theory.\n";
}
