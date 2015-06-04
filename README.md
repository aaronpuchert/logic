logic
=====

This shall be a logic system using machine learning. It is a work in progress.
It doesn't even have a name yet.

[![Build Status](https://travis-ci.org/aaronpuchert/logic.svg)](https://travis-ci.org/aaronpuchert/logic)

Motivation
----------
The current state of mathematical research is difficult to overview. No one
cares to clean up the mess we leave behind. Efforts to structure the theories
developed over myriads of papers are underrated.

On top of that, the huge volume of mathematical knowledge makes it hard to find
certain results that might be useful for your own research. It's hard to find
anything about a result if you don't know the (mostly arbitrary) name given to
it. You'd need to ask one of these living encyclopediæ, the rare species of
mathematicians who have a certain insight into most areas of mathematics.

This is not inevitable. We can keep up with the growing complexity if we try to
structure our knowledge better instead of waiting for figuring itself out. This
can not be the effort of a single person, or a small group like Bourbaki.
Helpfully, we now have the internet for bringing together many people around the
world in pursuit of a common goal.

Structure
---------
There will be three components:

* the logic core, which provides routines to deal with logical expressions
  and statements,
* the learning algorithm, trying to learn from given theories how to fill
  small gaps in an argument,
* the natural language connection, which allows users to read and write
  mathematics in their native language.

The logic core deals with theory files in the Lisp-like syntax described in
[doc/language.md](doc/language.md).

Installation
------------
The following tools are required to build the logic system (some are optional):
* GCC/Clang or compatible with full C++11 support
* GNU Make
* Boost test library (for tests)
* Doxygen (to generate the documentation)

Once you have these installed, you can run `make <target>`, with target
* `all` for everything,
* `test` for the tests,
* `tools` for the tools,
* `doc` for the Doxygen documentation and
* `clean` to remove all generated files.

There is no main program yet, hence no installation target. By default, release
versions are built. If you want debug symbols, add the option `VARIANT=debug`.

Usage
-----
There isn't much to use yet, except the parser.

### Parser ###
This is a tool to parse a theory in Lisp syntax and verify it. The parser is
invoked by

	$VARIANT/parser <theory file> [<rules file>]

where `$VARIANT` is either `debug` or `release`. If no rules file is given,
`basic/rules.lth` is used.
