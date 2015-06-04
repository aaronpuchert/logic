#include "../core/logic.hpp"
#include "../core/expression.hpp"
#include "../core/lisp.hpp"
#include "../core/debug.hpp"
#define BOOST_TEST_MODULE CoreTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <boost/test/output_test_stream.hpp>
#include <functional>

using namespace Core;
using std::make_shared;

//////////////////////////
// Test type comparator //
//////////////////////////

BOOST_AUTO_TEST_CASE(type_comparator_test)
{
	Node_ptr type_def[2];
	Expr_ptr variable[3], lambda[2];

	type_def[0] = make_shared<Node>(BuiltInType::type, "type1");
	type_def[1] = make_shared<Node>(BuiltInType::type, "type2");
	variable[0] = make_shared<AtomicExpr>(type_def[0]);
	variable[1] = make_shared<AtomicExpr>(type_def[0]);
	variable[2] = make_shared<AtomicExpr>(type_def[1]);

	lambda[0] = make_shared<LambdaType>(std::vector<const_Expr_ptr>{BuiltInType::statement, variable[0]});
	lambda[1] =  make_shared<LambdaType>(std::vector<const_Expr_ptr>{variable[2]}, variable[0]);

	TypeComparator compare;
	BOOST_CHECK(compare(BuiltInType::statement.get(), BuiltInType::statement.get()));
	BOOST_CHECK(!compare(BuiltInType::statement.get(), variable[1].get()));
	BOOST_CHECK(!compare(BuiltInType::statement.get(), lambda[0].get()));
	BOOST_CHECK(compare(variable[0].get(), variable[1].get()));
	BOOST_CHECK(!compare(variable[0].get(), variable[2].get()));
	BOOST_CHECK(compare(variable[2].get(), variable[2].get()));
	BOOST_CHECK(compare(lambda[1].get(), lambda[1].get()));
	BOOST_CHECK(!compare(lambda[0].get(), lambda[1].get()));
}

//////////////////////
// Test type checks //
//////////////////////

std::function<bool (const TypeException &ex)>
	type_exception_pred(const std::string &msg)
{
	return [msg] (const TypeException &ex) -> bool
	{
		if (msg != ex.what()) {
			std::cout << "Expected TypeException:\n\t" << msg << std::endl;
			std::cout << "but got\t" << ex.what() << std::endl;
		}
		return (msg == ex.what());
	};
}

BOOST_AUTO_TEST_CASE(type_check_test)
{
	Expr_ptr var_type, lambda_type[2];
	Node_ptr type_def, var_def[3], lambda[3], statement[3];
	Expr_ptr atomic[3], lambda_def, pred_expr[2], neg_expr, quant_expr[3];

	// Build types
	type_def = make_shared<Node>(BuiltInType::type, "var_type");
	var_type = make_shared<AtomicExpr>(type_def);
	lambda_type[0] = make_shared<LambdaType>(std::vector<const_Expr_ptr>{var_type});
	lambda_type[1] = make_shared<LambdaType>(std::vector<const_Expr_ptr>{var_type}, var_type);

	// Make some variables
	var_def[0] = make_shared<Node>(var_type, "x");
	var_def[1] = make_shared<Node>(var_type, "y");
	var_def[2] = make_shared<Node>(BuiltInType::statement, "a");
	for (int i=0; i<3; ++i)
		atomic[i] = make_shared<AtomicExpr>(var_def[i]);

	// Set definitions
	BOOST_CHECK_NO_THROW(var_def[1]->setDefinition(atomic[0]));
	BOOST_CHECK_EXCEPTION(
		var_def[2]->setDefinition(atomic[0]),
		TypeException,
		type_exception_pred("expected statement, but got var_type")
	);

	// Make some lambdas
	lambda[0] = make_shared<Node>(lambda_type[0], "pred");
	lambda[1] = make_shared<Node>(lambda_type[1], "lambda");
	lambda[2] = make_shared<Node>(lambda_type[0], "pred");

	// Call the predicate
	BOOST_CHECK_NO_THROW(pred_expr[0] = make_shared<LambdaCallExpr>(lambda[0], std::vector<Expr_ptr>{atomic[0]}));
	BOOST_CHECK_EXCEPTION(
		pred_expr[1] = make_shared<LambdaCallExpr>(lambda[0], std::vector<Expr_ptr>{atomic[2]}),
		TypeException,
		type_exception_pred("expected var_type, but got statement in argument 1")
	);

	// Define a predicate via another
	neg_expr = make_shared<NegationExpr>(pred_expr[0]);
	BOOST_CHECK_NO_THROW(lambda_def = make_shared<LambdaExpr>(std::vector<Node_ptr>{var_def[0]}, neg_expr));
	BOOST_CHECK_NO_THROW(lambda[2]->setDefinition(lambda_def));
	BOOST_CHECK_EXCEPTION(
		lambda[1]->setDefinition(lambda_def),
		TypeException,
		type_exception_pred("expected (var_type)->var_type, but got (var_type)->statement")
	);

	// Make quantifier statements
	BOOST_CHECK_NO_THROW(quant_expr[0] = make_shared<QuantifierExpr>(QuantifierExpr::FORALL, lambda_def));
	BOOST_CHECK_NO_THROW(quant_expr[1] = make_shared<QuantifierExpr>(QuantifierExpr::FORALL, make_shared<AtomicExpr>(lambda[0])));
	BOOST_CHECK_EXCEPTION(
		quant_expr[2] = make_shared<QuantifierExpr>(QuantifierExpr::FORALL, make_shared<AtomicExpr>(lambda[1])),
		TypeException,
		type_exception_pred("expected statement, but got var_type in return value")
	);

	// Make some statements
	BOOST_CHECK_NO_THROW(statement[0] = make_shared<Statement>("", pred_expr[0]));
	BOOST_CHECK_NO_THROW(statement[1] = make_shared<Statement>("", quant_expr[0]));
	BOOST_CHECK_EXCEPTION(
		statement[2] = make_shared<Statement>("", lambda_def),
		TypeException,
		type_exception_pred("expected statement, but got (var_type)->statement")
	);
}

//////////////////////////////////////////
// Test the syntax tree and Lisp writer //
//////////////////////////////////////////

template <typename T>
void checkResult(const T *object, const std::string& result)
{
	std::ostringstream stream;
	Writer writer(stream, std::numeric_limits<int>::max());		// don't wrap
	object->accept(&writer);
	BOOST_CHECK_EQUAL(stream.str(), result);
}

// Check theories --- against files
template <>
void checkResult<Theory>(const Theory *theory, const std::string& filename)
{
	boost::test_tools::output_test_stream stream(filename, true);
	Writer writer(stream, 80);		// wrap at 80 chars
	theory->accept(&writer);

	BOOST_CHECK(stream.match_pattern());
}

// Rule system
Theory rules;

BOOST_AUTO_TEST_CASE(rule_writer_test)
{
	// Put rules in theory
	Theory::iterator position = rules.begin();
	Theory::iterator it;	// Iterator for within rules

	// Create statement variables
	Node_ptr stmt_a = make_shared<Node>(BuiltInType::statement, "a");
	Node_ptr stmt_b = make_shared<Node>(BuiltInType::statement, "b");
	Expr_ptr expr_a = make_shared<AtomicExpr>(stmt_a);
	Expr_ptr expr_b = make_shared<AtomicExpr>(stmt_b);

	// Rule of the excluded middle.
	Expr_ptr not_a = make_shared<NegationExpr>(expr_a);
	Expr_ptr taut_stmt = make_shared<ConnectiveExpr>(ConnectiveExpr::OR, expr_a, not_a);
	Rule_ptr tautology = make_shared<Tautology>("excluded_middle",
		std::vector<Node_ptr>{stmt_a}, taut_stmt);

	checkResult(tautology.get(), "(tautology excluded_middle (list (statement a)) (or a (not a)))\n");
	position = rules.add(tautology, position);

	// Rule of double negation.
	Expr_ptr not_not_a = make_shared<NegationExpr>(not_a);
	Rule_ptr equivrule = make_shared<EquivalenceRule>("double_negation",
		std::vector<Node_ptr>{stmt_a}, not_not_a, expr_a);

	checkResult(equivrule.get(), "(equivrule double_negation (list (statement a)) (not (not a)) a)\n");
	position = rules.add(equivrule, position);

	// The modus ponens rule.
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL, expr_a, expr_b);
	std::vector<Expr_ptr> premisses{impl, expr_a};
	Rule_ptr deductionrule = make_shared<DeductionRule>("ponens",
		std::vector<Node_ptr>{stmt_a, stmt_b}, premisses, expr_b);

	checkResult(deductionrule.get(), "(deductionrule ponens (list (statement a) (statement b)) (list (impl a b) a) b)\n");
	position = rules.add(deductionrule, position);

	// The specialization rule
	Node_ptr type_decl = make_shared<Node>(BuiltInType::type, "T");
	Expr_ptr gen_type = make_shared<AtomicExpr>(type_decl);

	Expr_ptr pred_type = make_shared<LambdaType>(std::vector<const_Expr_ptr>{gen_type});
	Node_ptr pred_node = make_shared<Node>(pred_type, "P");

	Node_ptr var_y = make_shared<Node>(gen_type, "y");

	Expr_ptr expr_y = make_shared<AtomicExpr>(var_y);
	Expr_ptr atomic_pred = make_shared<AtomicExpr>(pred_node);
	Expr_ptr forall_expr = make_shared<QuantifierExpr>(QuantifierExpr::FORALL,
		atomic_pred);
	Expr_ptr pred_expr = make_shared<LambdaCallExpr>(pred_node,
		std::vector<Expr_ptr>{expr_y});
	Rule_ptr specializationrule = make_shared<DeductionRule>(
		"specialization", std::vector<Node_ptr>{type_decl, pred_node, var_y},
		std::vector<Expr_ptr>{forall_expr}, pred_expr);

	checkResult(specializationrule.get(), "(deductionrule specialization (list (type T) ((lambda-type statement (list T)) P) (T y)) (list (forall P)) (P y))\n");
	position = rules.add(specializationrule, position);

	// Check all rules with line wrapping
	checkResult(&rules, "examples/rules.lth");
}

BOOST_AUTO_TEST_CASE(theory_writer_test)
{
	Theory theory;
	Theory::iterator position = theory.begin();

	// (type person)
	Node_ptr person_node = make_shared<Node>(BuiltInType::type, "person");
	Expr_ptr person = make_shared<AtomicExpr>(person_node);
	checkResult(person_node.get(), "(type person)\n");
	position = theory.add(person_node, position);

	// (predicate schüler? (list person))
	Expr_ptr pred_type = make_shared<LambdaType>(std::vector<const_Expr_ptr>{person});
	Node_ptr student = make_shared<Node>(pred_type, "schüler?");
	checkResult(student.get(), "((lambda-type statement (list person)) schüler?)\n");
	position = theory.add(student, position);

	// (predicate dumm? (list person))
	Node_ptr stupid = make_shared<Node>(pred_type, "dumm?");
	checkResult(stupid.get(), "((lambda-type statement (list person)) dumm?)\n");
	position = theory.add(stupid, position);

	// (person fritz) ; this is in fact a constant, but what is the difference?
	Node_ptr fritz = make_shared<Node>(person, "fritz");
	checkResult(fritz.get(), "(person fritz)\n");
	position = theory.add(fritz, position);
	Expr_ptr fritz_expr = make_shared<AtomicExpr>(fritz);

	// (axiom (schüler? fritz))
	Expr_ptr axiom1_expr = make_shared<LambdaCallExpr>(student,
		std::vector<Expr_ptr>{fritz_expr});
	Statement_ptr axiom1 = make_shared<Statement>("", axiom1_expr);
	checkResult(axiom1.get(), "(axiom (schüler? fritz))\n");
	position = theory.add(axiom1, position);

	// (axiom (forall (list (person x)) (impl (schüler? x) (dumm? x))))
	Node_ptr var_x = make_shared<Node>(person, "x");
	Expr_ptr expr_x = make_shared<AtomicExpr>(var_x);
	Expr_ptr student_x = make_shared<LambdaCallExpr>(student,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr stupid_x = make_shared<LambdaCallExpr>(stupid,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL,
		student_x, stupid_x);
	std::shared_ptr<LambdaExpr> impl_pred =
		make_shared<LambdaExpr>(std::vector<Node_ptr>{var_x}, impl);
	Expr_ptr forall_expr = make_shared<QuantifierExpr>
		(QuantifierExpr::FORALL, impl_pred);
	Statement_ptr axiom2 = make_shared<Statement>("", forall_expr);
	checkResult(axiom2.get(), "(axiom (forall (lambda (list (person x)) (impl (schüler? x) (dumm? x)))))\n");
	position = theory.add(axiom2, position);

	// We want to prove: fritz is stupid
	Expr_ptr statement_expr = make_shared<LambdaCallExpr>(stupid,
		std::vector<Expr_ptr>{fritz_expr});

	//  (lemma (impl (schüler? fritz) (dumm? fritz))
	Expr_ptr lemma1_expr = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL,
		axiom1_expr, statement_expr);
	Statement_ptr lemma1 = make_shared<Statement>("", lemma1_expr);
	position = theory.add(lemma1, position);
	//  	(specialization
	//  		(list person (list (person x)) (impl (schüler? x) (dumm? x)) fritz)
	//  		(list this~1)
	//  	)
	Reference this1(&theory, position);
	const_Object_ptr specialization = *rules.get("specialization");
	std::shared_ptr<ProofStep> step1 = make_shared<ProofStep>(
		std::static_pointer_cast<const Rule>(specialization),
		std::vector<Expr_ptr>{person, impl_pred, fritz_expr},
		std::vector<Reference>{this1 - 1});
	lemma1->addProof(step1);
	//  )

	//  (lemma (dumm? fritz)
	Statement_ptr lemma2 = make_shared<Statement>("", statement_expr);
	position = theory.add(lemma2, position);
	//  	(ponens
	//  		(list (schüler? fritz) (dumm? fritz))
	//  		(list this~1 this~3)
	//  	)
	Reference this2(&theory, position);
	const_Object_ptr ponens = *rules.get("ponens");
	std::shared_ptr<ProofStep> step2 = make_shared<ProofStep>(
		std::static_pointer_cast<const Rule>(ponens),
		std::vector<Expr_ptr>{axiom1_expr, statement_expr},
		std::vector<Reference>{this2 - 1, this2 - 3});
	lemma2->addProof(step2);
	//  )

	// Check the whole theory with line wrapping
	checkResult(&theory, "examples/simple.lth");

	// Verify the theory
	BOOST_CHECK(theory.verify());
}

//////////////////////
// Check the parser //
//////////////////////

BOOST_AUTO_TEST_CASE(parser_rule_test)
{
	std::ifstream file("basic/rules.lth");
	Parser parser(file, std::cout, "basic/rules.lth");
	Theory rules = parser.parseTheory();
	BOOST_CHECK_EQUAL(parser.getErrors(), 0);
}

BOOST_AUTO_TEST_CASE(parser_theory_test)
{
	std::ifstream file("examples/simple.lth");
	Parser parser(file, std::cout, "examples/simple.lth");
	parser.rules = &rules;
	Theory simple = parser.parseTheory();
	BOOST_CHECK_EQUAL(parser.getErrors(), 0);

	// Verify the theory
	BOOST_CHECK(simple.verify());
}
