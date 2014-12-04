#include "../core/logic.hpp"
#include "../core/lisp.hpp"
#define BOOST_TEST_MODULE LispTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace Core;
using std::make_shared;

void checkRule(Rule_ptr rule, const std::string& result)
{
	std::ostringstream stream;
	Writer writer(stream);
	rule->accept(&writer);
	BOOST_CHECK_EQUAL(stream.str(), result);
}

BOOST_AUTO_TEST_CASE(rule_writer_test)
{
	// Create statement variables
	Type_ptr statement = make_shared<Type>("statement");
	std::shared_ptr<Variable> stmt_a = make_shared<Variable>(statement, "a");
	std::shared_ptr<Variable> stmt_b = make_shared<Variable>(statement, "b");
	Expr_ptr expr_a = make_shared<AtomicExpr>(stmt_a);
	Expr_ptr expr_b = make_shared<AtomicExpr>(stmt_b);

	// Rule of the excluded middle.
	Expr_ptr not_a = make_shared<NegationExpr>(expr_a);
	Expr_ptr taut_stmt = make_shared<ConnectiveExpr>(ConnectiveExpr::OR, expr_a, not_a);
	Rule_ptr tautology = make_shared<Tautology>("excluded_middle",
		Rule::VarList{*stmt_a}, taut_stmt);

	checkRule(tautology, "(tautology excluded_middle (list (statement a)) (or a (not a)))\n");

	// Rule of double negation.
	Expr_ptr not_not_a = make_shared<NegationExpr>(not_a);
	Rule_ptr equivrule = make_shared<EquivalenceRule>("double_negation",
		Rule::VarList{*stmt_a}, not_not_a, expr_a);

	checkRule(equivrule, "(equivrule double_negation (list (statement a)) (not (not a)) a)\n");

	// The modus ponens rule.
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL, expr_a, expr_b);
	std::vector<Expr_ptr> premisses{impl, expr_a};
	Rule_ptr deductionrule = make_shared<DeductionRule>("ponens",
		Rule::VarList{*stmt_a, *stmt_b}, premisses, expr_b);

	checkRule(deductionrule, "(deductionrule ponens (list (statement a) (statement b)) (list (impl a b) a) b)\n");
}
