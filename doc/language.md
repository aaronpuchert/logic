# Syntax #
We follow the Lisp syntax. There are two central structures: nodes and
expressions. Expressions are defined recursively as usual, more details can be
found in the following sections. Theories are lists of nodes, which are simply
names (identifiers). Nodes with just a name can be understood as declarations.
It is also possible to give an expression for a node, i.e. a definition.

## Types ##
Expressions and nodes are statically typed. There are a few built-in types:

	<type> := type | statement | rule

It is also possible to define your own types anywhere in a theory. Nodes of a
certain type are then declared via:

	<node> := (<type> <type-name>)
	<type> |= <type-name>

While these types are all atomic, there are also compound types to describe
lambda expressions such as predicates.

	<type> |= (lambda <return-type> (list <argument-type>*))

Say we have a type `number` and want a 1-valued predicate on numbers, then its
type would be

	(lambda statement (list number))

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

Hence these are operators that accept statements as their input and return a
statement.

### Predicates and Quantifiers (Predicate logic) ###
Predicates and quantifiers are statically typed. Individuals of a type are
defined by

	<declaration> |= (<type> <name>)

Predicates can be declared, only giving the types of arguments, or be defined,
giving a statement that depends on the arguments:

	<declaration> |= (<predicate-type> <pred-name> [<predicate-lambda>])
	<predicate-lambda> = <dec-list> <statement>
	<dec-list> := (list <declaration>*)

If a predicate lambda is given, the predicate type can be deduced and we can
just write `predicate` (or `auto`?)

A predicate-lambda is an expression that can be used within other expressions.
The `<statement>` may depend on the variables given in `<dec-list>`. “Calling” a
predicate is now done by

	<statement> |= (<pred-name> <expression>*)

and results in a statement. Of course, we need quantifier expressions for
predicate logic:

	<statement> |= ({forall|exists} <predicate-expression>)

The `<predicate-expression>` could be a predicate lambda or the name of a
predicate.

### Rules ###
The rules of logic are not hardcoded into the system and must be stated
explicitly. They are used to verify proofs.

Every rule has a name, by which it is referenced in proofs, and a list of
variables which can be substituted.

	<dec-list> := (list (<declaration>*))

With these prerequisites, we can define tautologies:

	<tautology> := (tautology <name> <dec-list> <statement>)

Such a rule states that `<statement>` is always true, regardless of the values
of any variables used.

Equivalence rules state that two statements are logically equivalent:

	<equivalence-rule> := (equivrule <name> <dec-list> <statement> <statement>)

This means one can be replaced by the other. The last and most common form of
rules are deduction rules. They state that if every statement in the
`<statement-list>` is true, then `<statement>` is also true:

	<deduction-rule> := (deductionrule <name> <dec-list> <statement-list> <statement>)
	<statement-list> := (list <statement>*)

The rules of standard logic will be predefined in `basic/rules.thy`.

## Structure of theories ##
Theories consist of declarations, axioms and (proved) statements. Consequently,
a theory is just a list of objects like these:

	<theory> := (<declaration>
		| <theory-axiom>
		| <theory-statement>)*

### Axioms and statements ###
Axioms are statements without proof. Their syntax is

	<theory-axiom> := (axiom [<name>] <statement>)

Statements (with proof) have the following syntax:

	<theory-statement> := (statement [<name>] <statement> <proof>)

A name, if given, serves as identifier for references.

### References ###
When deducing new statements from given ones, we need a possibility to reference
statements given before. For named statements, the reference is given via the
name:

	<reference> := <name>

There are two special names: `this` refers to the statement itself, while
`parent` refers to the parent of the statement itself (see below). To refer to
the parent of the parent, just write `parent^2`. Then obviously `parent^1` is
the same as `parent`. Of course, these two are not useful alone, but together
with the following concept they help to refer to unnamed statements:

	<reference> |= <name>~<number>

This refers to the `<number>`th statement before `name`. Thus, `<name>~0` is the
same as `<name>`, `<name>~1` is the statement before it and so on. This can be
used in conjunction with `this` and `parent`: `this~1` is the last statement
before the current one, `parent~2` is the second statement before our parent.
This goes on: `parent^2~1` is the statement just before our grandparent.

There is another kind of reference we could implement: hashes derived from the
statement (and maybe its axioms). This would allow us to have fixed references
in clear text, albeit not terribly readable.

### Proofs ###
There are two kinds of proofs: elementary proofs and composite proofs. An
elementary proof consists of the application of a rule:

	<proof> := (<rule> <expression-list> <reference-list>)
	<expression-list> := (list <expression>*)
	<reference-list> := (list <reference>*)

The number of expressions and references, respectively, depends on the number
of variables in the `<variable-list>` part of the rule and the number of further
arguments (which are statements) given minus one, respectively. So there is no
reference needed for applying a tautology, one for applying a equivalence rule,
and `n` for applying a deduction rule with `n` premisses.

A composite proof consists of statements:

	<proof> |= (proof <theory-statement>*)

It may also contain definitions, assumptions, etc. — maybe later.
