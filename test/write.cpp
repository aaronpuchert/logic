#include "../core/logic.hpp"
#include "../core/theory.hpp"
#include "../core/lisp.hpp"
#define BOOST_TEST_MODULE LispTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

using namespace Core;
using std::make_shared;

template <typename T>
void checkResult(const T *object, const std::string& result)
{
	std::ostringstream stream;
	Writer writer(stream);
	object->accept(&writer);
	std::cout << "Writer: \e[1m" << stream.str() << "\e[0m";
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

	checkResult(tautology.get(), "(tautology excluded_middle (list (statement a)) (or a (not a)))\n");

	// Rule of double negation.
	Expr_ptr not_not_a = make_shared<NegationExpr>(not_a);
	Rule_ptr equivrule = make_shared<EquivalenceRule>("double_negation",
		Rule::VarList{*stmt_a}, not_not_a, expr_a);

	checkResult(equivrule.get(), "(equivrule double_negation (list (statement a)) (not (not a)) a)\n");

	// The modus ponens rule.
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL, expr_a, expr_b);
	std::vector<Expr_ptr> premisses{impl, expr_a};
	Rule_ptr deductionrule = make_shared<DeductionRule>("ponens",
		Rule::VarList{*stmt_a, *stmt_b}, premisses, expr_b);

	checkResult(deductionrule.get(), "(deductionrule ponens (list (statement a) (statement b)) (list (impl a b) a) b)\n");
}

BOOST_AUTO_TEST_CASE(theory_writer_test)
{
	Theory theory(nullptr);
	Theory::iterator position = theory.begin();

	// (type person)
	Type_ptr person = make_shared<Type>("person");
	checkResult(person.get(), "(type person)\n");
	position = theory.add(person, position);

	// (predicate schüler? (list person))
	std::shared_ptr<PredicateDecl> student = make_shared<PredicateDecl>("schüler?", 1);
	student->setParameterType(0, person);
	checkResult(student.get(), "(predicate schüler? (list person))\n");
	position = theory.add(student, position);

	// (predicate dumm? (list person))
	std::shared_ptr<PredicateDecl> stupid = make_shared<PredicateDecl>("dumm?", 1);
	stupid->setParameterType(0, person);
	checkResult(stupid.get(), "(predicate dumm? (list person))\n");
	position = theory.add(stupid, position);

	// (person fritz) ; this is in fact a constant, but what is the difference?
	Var_ptr fritz = make_shared<Variable>(person, "fritz");
	checkResult(fritz.get(), "(person fritz)\n");
	position = theory.add(fritz, position);
	Expr_ptr fritz_expr = make_shared<AtomicExpr>(fritz);

	// (axiom (schüler? fritz))
	Expr_ptr axiom1_expr = make_shared<PredicateExpr>(student,
		std::vector<Expr_ptr>{fritz_expr});
	std::shared_ptr<Statement> axiom1 =
		make_shared<Statement>("fritz_is_student", axiom1_expr);
	checkResult(axiom1.get(), "(axiom (schüler? fritz))\n");
	position = theory.add(axiom1, position);

	// (axiom (forall (list (person x)) (impl (schüler? x) (dumm? x))))
	Var_ptr var_x = make_shared<Variable>(person, "x");
	Expr_ptr expr_x = make_shared<AtomicExpr>(var_x);
	Expr_ptr student_x = make_shared<PredicateExpr>(student,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr stupid_x = make_shared<PredicateExpr>(stupid,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL,
		student_x, stupid_x);
	PredicateLambda impl_pred(std::vector<Variable>{*var_x}, impl);
	Expr_ptr forall_expr = make_shared<QuantifierExpr>
		(QuantifierExpr::FORALL, std::move(impl_pred));
	std::shared_ptr<Statement> axiom2 =
		make_shared<Statement>("students_are_stupid", forall_expr);
	checkResult(axiom2.get(), "(axiom (forall (list (person x)) (impl (schüler? x) (dumm? x))))\n");
	position = theory.add(axiom2, position);

	//  (statement
	//  	(dumm? fritz)
	Expr_ptr statement_expr = make_shared<PredicateExpr>(stupid,
		std::vector<Expr_ptr>{fritz_expr});
	std::shared_ptr<Statement> statement =
		make_shared<Statement>("fritz_is_stupid", statement_expr);
	//  	(proof
	//  		(statement (impl (schüler? fritz) (dumm? fritz))
	//  			(specialization
	//  				(list (predicate (list (person x)) (impl (schüler? x) (dumm? x))) person fritz)
	//  				(list (ref parent~1))
	//  			)
	//  		)
	//  		(statement (dumm? fritz)
	//  			(ponens
	//  				(list (schüler? fritz) (dumm? fritz))
	//  				(list (ref parent~2) (ref this~1))
	//  			)
	//  		)
	//  	)
	//  )
	// TODO: add the proof
	position = theory.add(statement, position);
}
