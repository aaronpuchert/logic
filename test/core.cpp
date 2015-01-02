#include "../core/logic.hpp"
#include "../core/expression.hpp"
#include "../core/lisp.hpp"
#define BOOST_TEST_MODULE CoreTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <sstream>
#include <iostream>

using namespace Core;
using std::make_shared;

BOOST_AUTO_TEST_CASE(type_comparator_test)
{
	Node_ptr type_def[2];
	Type_ptr variable[3], lambda[2];

	type_def[0] = make_shared<Node>(BuiltInType::type, "type1");
	type_def[1] = make_shared<Node>(BuiltInType::type, "type2");
	variable[0] = make_shared<VariableType>(type_def[0]);
	variable[1] = make_shared<VariableType>(type_def[0]);
	variable[2] = make_shared<VariableType>(type_def[1]);

	lambda[0] = make_shared<LambdaType>(std::vector<const_Type_ptr>{BuiltInType::statement, variable[0]});
	lambda[1] =  make_shared<LambdaType>(std::vector<const_Type_ptr>{variable[2]}, variable[0]);

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

template <typename T>
void checkResult(const T *object, const std::string& result)
{
	std::ostringstream stream;
	Writer writer(stream);
	object->accept(&writer);
	std::cout << "Writer: \e[1m" << stream.str() << "\e[0m";
	BOOST_CHECK_EQUAL(stream.str(), result);
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
		Theory{stmt_a}, taut_stmt);

	checkResult(tautology.get(), "(tautology excluded_middle (list (statement a)) (or a (not a)))\n");
	position = rules.add(tautology, position);

	// Rule of double negation.
	Expr_ptr not_not_a = make_shared<NegationExpr>(not_a);
	Rule_ptr equivrule = make_shared<EquivalenceRule>("double_negation",
		Theory{stmt_a}, not_not_a, expr_a);

	checkResult(equivrule.get(), "(equivrule double_negation (list (statement a)) (not (not a)) a)\n");
	position = rules.add(equivrule, position);

	// The modus ponens rule.
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL, expr_a, expr_b);
	std::vector<Expr_ptr> premisses{impl, expr_a};
	Rule_ptr deductionrule = make_shared<DeductionRule>("ponens",
		Theory{stmt_a, stmt_b}, premisses, expr_b);

	checkResult(deductionrule.get(), "(deductionrule ponens (list (statement a) (statement b)) (list (impl a b) a) b)\n");
	position = rules.add(deductionrule, position);

	// The specialization rule
	Node_ptr type_decl = make_shared<Node>(BuiltInType::type, "T");
	Type_ptr gen_type = make_shared<VariableType>(type_decl);

	Type_ptr pred_type = make_shared<LambdaType>(std::vector<const_Type_ptr>{gen_type});
	Node_ptr pred_node = make_shared<Node>(pred_type, "P");

	Node_ptr var_y = make_shared<Node>(gen_type, "y");

	Expr_ptr expr_y = make_shared<AtomicExpr>(var_y);
	Expr_ptr atomic_pred = make_shared<AtomicExpr>(pred_node);
	Expr_ptr forall_expr = make_shared<QuantifierExpr>(QuantifierExpr::FORALL,
		atomic_pred);
	Expr_ptr pred_expr = make_shared<PredicateExpr>(pred_node,
		std::vector<Expr_ptr>{expr_y});
	Rule_ptr specializationrule = make_shared<DeductionRule>(
		"specialization", Theory{type_decl, pred_node, var_y},
		std::vector<Expr_ptr>{forall_expr}, pred_expr);

	checkResult(specializationrule.get(), "(deductionrule specialization (list (type T) ((lambda statement (list T)) P) (T y)) (list (forall P)) (P y))\n");
	position = rules.add(specializationrule, position);
}

BOOST_AUTO_TEST_CASE(theory_writer_test)
{
	Theory theory;
	Theory::iterator position = theory.begin();

	// (type person)
	Node_ptr person_node = make_shared<Node>(BuiltInType::type, "person");
	Type_ptr person = make_shared<VariableType>(person_node);
	checkResult(person_node.get(), "(type person)\n");
	position = theory.add(person_node, position);

	// (predicate schüler? (list person))
	Type_ptr pred_type = make_shared<LambdaType>(std::vector<const_Type_ptr>{person});
	Node_ptr student = make_shared<Node>(pred_type, "schüler?");
	checkResult(student.get(), "((lambda statement (list person)) schüler?)\n");
	position = theory.add(student, position);

	// (predicate dumm? (list person))
	Node_ptr stupid = make_shared<Node>(pred_type, "dumm?");
	checkResult(stupid.get(), "((lambda statement (list person)) dumm?)\n");
	position = theory.add(stupid, position);

	// (person fritz) ; this is in fact a constant, but what is the difference?
	Node_ptr fritz = make_shared<Node>(person, "fritz");
	checkResult(fritz.get(), "(person fritz)\n");
	position = theory.add(fritz, position);
	Expr_ptr fritz_expr = make_shared<AtomicExpr>(fritz);

	// (axiom (schüler? fritz))
	Expr_ptr axiom1_expr = make_shared<PredicateExpr>(student,
		std::vector<Expr_ptr>{fritz_expr});
	Statement_ptr axiom1 = make_shared<Statement>("", axiom1_expr);
	checkResult(axiom1.get(), "(axiom (schüler? fritz))\n");
	position = theory.add(axiom1, position);

	// (axiom (forall (list (person x)) (impl (schüler? x) (dumm? x))))
	Node_ptr var_x = make_shared<Node>(person, "x");
	Expr_ptr expr_x = make_shared<AtomicExpr>(var_x);
	Expr_ptr student_x = make_shared<PredicateExpr>(student,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr stupid_x = make_shared<PredicateExpr>(stupid,
		std::vector<Expr_ptr>{expr_x});
	Expr_ptr impl = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL,
		student_x, stupid_x);
	std::shared_ptr<PredicateLambda> impl_pred =
		make_shared<PredicateLambda>(Theory{var_x}, impl);
	Expr_ptr forall_expr = make_shared<QuantifierExpr>
		(QuantifierExpr::FORALL, impl_pred);
	Statement_ptr axiom2 = make_shared<Statement>("", forall_expr);
	checkResult(axiom2.get(), "(axiom (forall (list (person x)) (impl (schüler? x) (dumm? x))))\n");
	position = theory.add(axiom2, position);

	//  (statement
	//  	(dumm? fritz)
	Expr_ptr statement_expr = make_shared<PredicateExpr>(stupid,
		std::vector<Expr_ptr>{fritz_expr});
	Statement_ptr statement = make_shared<Statement>("", statement_expr);
	position = theory.add(statement, position);

	//  	(proof
	std::shared_ptr<LongProof> proof = make_shared<LongProof>(&theory, position);
	Theory::iterator sub_pos = proof->subTheory.begin();
	//  		(statement (impl (schüler? fritz) (dumm? fritz))
	Expr_ptr inter1_expr = make_shared<ConnectiveExpr>(ConnectiveExpr::IMPL,
		axiom1_expr, statement_expr);
	Statement_ptr inter1 = make_shared<Statement>("", inter1_expr);
	//  			(specialization
	//  				(list person (list (person x)) (impl (schüler? x) (dumm? x)) fritz)
	//  				(list parent~1)
	//  			)
	std::shared_ptr<ProofStep> step1 = make_shared<ProofStep>(&rules, "specialization",
		std::vector<Expr_ptr>{make_shared<AtomicExpr>(person_node), impl_pred, fritz_expr},
		std::vector<Reference>{Reference(&theory, --position)});
	inter1->addProof(step1);
	sub_pos = proof->subTheory.add(inter1, sub_pos);
	//  		)
	//  		(statement (dumm? fritz)
	Statement_ptr inter2 = make_shared<Statement>("", statement_expr);
	//  			(ponens
	//  				(list (schüler? fritz) (dumm? fritz))
	//  				(list parent~2 this~1)
	//  			)
	std::shared_ptr<ProofStep> step2 = make_shared<ProofStep>(&rules,
		"ponens", std::vector<Expr_ptr>{axiom1_expr, statement_expr},
		std::vector<Reference>{Reference(&theory, --position), Reference(&proof->subTheory, sub_pos)});
	inter2->addProof(step2);
	sub_pos = proof->subTheory.add(inter2, sub_pos);
	//  		)
	//  	)
	//  )
	statement->addProof(proof);
	checkResult(statement.get(), "(lemma (dumm? fritz) (proof (lemma (impl (schüler? fritz) (dumm? fritz)) (specialization (list person (list (person x)) (impl (schüler? x) (dumm? x)) fritz) (list parent~1))) (lemma (dumm? fritz) (ponens (list (schüler? fritz) (dumm? fritz)) (list parent~2 this~1)))))\n");
}
