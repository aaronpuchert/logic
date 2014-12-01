# Syntax #
We follow the Lisp syntax. A program is a list of expressions, which may again
contain other expressions.

## Basic logic ##
### Statements (Propositional calculus) ###
There are two statement constants: `true` and `false`. Statement variables are
declared by

	<statement-declaration> := (statement <statement-name>)

where `<name>` is a unique name. Such statements are called atomic statements.
The can be composed in the following ways:

	<statement> := <statement-name>
		| (not <statement>)
		| (and <statement> <statement>)
		| (or <statement> <statement>)
		| (impl <statement> <statement>)
		| (equiv <statement> <statement>)

### Types, Predicates and Quantifiers (Predicate logic) ###
Types reduce the scope of predicates and quantifiers. A type is simply declared
by

	<type-declaration> := (type <typename>)

There are two predefined types: `statement` and `predicate`. This allows as to
make statements about statements and predicates. As soon as we have a type, we
can define individuals of this type by

	<declaration> := (<typename> <name>)

Note that the declaration of statement variables is a special case of this one.
Predicates can be declared, only giving the types of arguments, or be defined,
giving a statement that depends on the arguments:

	<predicate-declaration> := (predicate <pred_name> <type-list>)
	<predicate-definition> := (predicate-definition [<pred_name>] <dec-list> <statement>)
	<type_list> := (list <typename>*)
	<dec-list> := (list <declaration>*)

Predicate definitions may omit a name, allowing for anonymous predicates. The
`<statement>` may depend on the variables given in `<dec-list>`. “Calling” a
predicate is now done by

	<statement> |= (<name> <expression>*)

and results in a statement. As of now, the only expressions we know are
individuals. There are two other important statements in predicate logic:

	<statement> |= (forall <declaration> <statement>)
		| (exists <declaration> <statement>)

### Rules ###
The rules of logic are not hardcoded into the system and must be stated
explicitly. They are used to verify proofs.

Every rule has a name, by which it is referenced in proofs, and a list of
variables which can be substituted.

	<variable-list> := (list (<declaration>*))

With these prerequisites, we can define tautologies:

	<tautology> := (tautology <name> <variable-list> <statement>)

Such a rule states that `<statement>` is always true, regardless of the values
of any variables used.

Equivalence rules state that two statements are logically equivalent:

	<equivalence-rule> := (equivrule <name> <variable-list> <statement> <statement>)

This means one can be replaced by the other. The last and most common form of
rules are deduction rules. They state that if every statement in the
`<statement-list>` is true, then `<statement>` is also true:

	<deduction-rule> := (deductionrule <name> <variable-list> <statement-list> <statement>)
	<statement-list> := (list <statement>*)

The rules of standard logic will be predefined in `basic/rules.thy`.

## Structure of theories ##
Theories consist of declarations, axioms and (proved) statements. Consequently,
a theory is just a list of objects like these:

	<theory> := (<declaration>
		| <predicate-declaration>
		| <predicate-definition>
		| <theory-axiom>
		| <theory-statement>) *

### Axioms and statements ###
Axioms are statements without proof. Their syntax is

	<theory-axiom> := (axiom <statement>)

Statements (with proof) have the following syntax:

	<theory-statement> := (statement <statement> <proof>)

### Proofs ###
There are two kinds of proofs: elementary proofs and composite proofs. An
elementary proof consists of the application of a rule:

	<proof> := (<rule> <variable-list> <statement-list>)

A composite proof consists of statements:

	<proof> |= (proof <theory-statement>*)

It may also contain definitions, assumptions, etc. — maybe later.
