#SuperCalc

#####A mathematical expression parser and evaluator plus more.


Currently, SuperCalc supports the following binary operators:

* Addition (`a + b`)
* Subtraction (`a - b`)
* Multiplication (`a * b`)
* Division (`a / b`)
* Modulus (`a % b`)
* Power (`a ^ b`)

SuperCalc also supports the following unary operators:

* Negation (`-x`)
* Factorial (`x!`)

Additionally, the following constants are defined:

* `pi`
* `e`
* `phi`

As well as the following mathematical functions:

* `sqrt`
* `abs`
* `exp`
* `log` (base 10)
* `log2` (base 2)
* `ln` (base e)
* `logbase(x, b)` -> log_b(x)
* `sin`
* `cos`
* `tan`
* `sec`
* `csc`
* `cot`
* `asin`
* `acos`
* `atan`
* `atan2`
* `asec`
* `acsc`
* `acot`
* `sinh`
* `cosh`
* `tanh`
* `sech`
* `csch`
* `coth`
* `asinh`
* `acosh`
* `atanh`
* `asech`
* `acsch`
* `acoth`

SuperCalc likes to be as precise as it knows how, so floating point values are avoided as much as possible. Even for division and negative powers, SuperCalc will attempt to use fractions as a value type instead of floating point values.

Example of using fractions:

	>>> (2 / 7) ^ 2
	4/49 (0.081632653061224)
	>>> -(3 + 4!/7)^3
	-91125/343 (-265.670553935860084)

Variables are supported:

	>>> a = 5
	5
	>>> a * 3
	15
	>>> b = 2 * a + 4
	14
	>>> ans + 4
	18

Shorthand notations for variable modification work as well using any of the binary operators:

	>>> x = 4
	4
	>>> x += 3
	7
	>>> x *= 2
	14
	>>> x /= 3
	14/3 (4.66666666666667)
	>>> x *= 6
	28
	>>> x %= 2
	0
	>>> x += 8
	8
	>>> x ^= 2
	64

Functions are also supported:

	>>> f(x) = 3x + 4
	f
	>>> f(7)
	25

Even with multiple arguments:

	>>> f(x, y) = x + y
	f
	>>> f(3, 5)
	8
	>>> g(x, y, z) = f(x, y) * z
	g
	>>> g(1, 2, 3)
	9

Function arguments do not affect variables:

	>>> x = 7
	7
	>>> f(x) = 4x
	f
	>>> f(6)
	24
	>>> x
	7

Functions can use global variables:

	>>> myFunc(arg) = 3 * arg + glb
	myFunc
	>>> glb = 7
	7
	>>> myFunc(4)
	19
	>>> glb += 3
	10
	>>> myFunc(4)
	22
	>>> other(x) = x + ans
	other
	>>> 8
	8
	>>> other(4)
	12
	>>> other(4)
	16

Error messages attempt to be clear:

	>>> 3 / (1 - 1)
	Math Error: Division by zero.
	>>> sqrt()
	Type Error: Builtin 'sqrt' takes 1 arguments, not 0.
	>>> sqrt(4, 3)
	Type Error: Builtin 'sqrt' takes 1 arguments, not 2.
	>>> sqrt(-1)
	Math Error: Builtin function 'sqrt' returned an invalid value.
	>>> a
	Name Error: No variable named 'a' found.
	>>> 17 $ 8
	Syntax Error: Unexpected character: '$'.
	>>> 8 +
	Syntax Error: Premature end of input.
	>>> pi(1, 2)
	Type Error: Builtin 'pi' is not a function.


##Building

To build on Linux, you need Automake and Autoconf:

	$ ./autogen.sh
	$ ./configure
	$ make
