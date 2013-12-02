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

* `sqrt(x)`
* `abs(x)`
* `exp(x)`
* `log(x)` -> log<sub>10</sub>(x)
* `log2(x)` -> log<sub>2</sub>(x)
* `ln(x)` -> log<sub>e</sub>(x)
* `logbase(x, b)` -> log<sub>b</sub>(x)
* `sin(x)`
* `cos(x)`
* `tan(x)`
* `sec(x)`
* `csc(x)`
* `cot(x)`
* `asin(x)`
* `acos(x)`
* `atan(x)`
* `atan2(y, x)`
* `asec(x)`
* `acsc(x)`
* `acot(x)`
* `sinh(x)`
* `cosh(x)`
* `tanh(x)`
* `sech(x)`
* `csch(x)`
* `coth(x)`
* `asinh(x)`
* `acosh(x)`
* `atanh(x)`
* `asech(x)`
* `acsch(x)`
* `acoth(x)`

SuperCalc likes to be as precise as it knows how, so floating point values are avoided as much as possible. Even for division and negative powers, SuperCalc will attempt to use fractions as a value type instead of floating point values.

Example of using fractions:

	>>> (2 / 7) ^ 2
	4/49 (0.0816326530612245)
	>>> -(3 + 4!/7)^3
	-91125/343 (-265.67055393586)

Variables are supported:

	>>> a = 5
	5
	>>> a * 3
	15
	>>> b = 2 * a + 4
	14
	>>> ans + 4
	18

Variable names may only contain alphanumeric characters and '_', but the first character may not be a number.

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
	>>> f(7)
	25

Even with multiple arguments:

	>>> f(x, y) = x + y
	>>> f(3, 5)
	8
	>>> g(x, y, z) = f(x, y) * z
	>>> g(1, 2, 3)
	9

Function arguments do not affect variables:

	>>> x = 7
	7
	>>> f(x) = 4x
	>>> f(6)
	24
	>>> x
	7

Functions can use global variables:

	>>> myFunc(arg) = 3 * arg + glb
	>>> glb = 7
	7
	>>> myFunc(4)
	19
	>>> glb += 3
	10
	>>> myFunc(4)
	22
	>>> other(x) = x + ans
	>>> 8
	8
	>>> other(4)
	12
	>>> other(4)
	16

Functions *are* variables:

	>>> f(x, y) = x^2 - y^2
	>>> g = f
	>>> g(4, 3)
	7
	>>> h(x) = x^2
	>>> getFunc() = h
	>>> func = getFunc()
	>>> func(3)
	9

Vectors are supported as well:

	>>> a = <1, 2, 3>
	<1, 2, 3>
	>>> b = <6, 5, 3>
	<6, 5, 3>
	>>> a - b
	<-5, -3, 0>
	>>> 6 * a - b
	<0, 7, 15>

Vector specific builtins:

* `dot(vector1, vector2)`
* `cross(vector1, vector2)`
* `map(function, vector)`

Vector functions:
	
	>>> a = <1, 2, 3>
	<1, 2, 3>
	>>> b = <6, 5, 3>
	<6, 5, 3>
	>>> cross(a, b)
	<-9, 15, -7>
	>>> dot(a, b)
	25

Function calls and vector subscripting is recursive:

	>>> getVec() = <1, 2, 3, <4, 5>>
	>>> getVec()[3][1]
	5
	>>> getFunc() = getVec
	>>> getFunc()()[3][0]
	4

Examples using `map`:

	>>> a = <3, 4, 5>
	<3, 4, 5>
	>>> f(x) = 2x + 1
	>>> map(f, a)
	<7, 9, 11>
	>>> angles = <0, pi/2, pi, 3pi/2, 2pi>
	<0, 1.5707963267949, 3.14159265358979, 4.71238898038469, 6.28318530717959>
	>>> map(sin, angles)
	<0, 1, 0, -1, 0>


Vectors can have any dimension greater than one:

	>>> a = <7, 2, 5.5, 7.6>
	<7, 2, 5.5, 7.6>
	>>> dot(a, <1, 2, 3, 4>)
	57.9
	>>> c = <1, 2>
	<1, 2>
	
Multiplying or dividing two vectors uses their components:

	>>> <1, 2, 3> * <4, 7, 2>
	<4, 14, 6>
	>>> <1, 4, 5> / <4, 6, 2>
	<1/4, 2/3, 5/2>

Vectors even support scalar operations:

	>>> a = <4, 7, -3>
	<4, 7, -3>
	>>> a + 2
	<4.92998110995055, 8.62746694241347, -3.69748583246292>
	>>> ans / 3
	<1.64332703665018, 2.87582231413782, -1.23249527748764>
	>>> ans ^ 2
	<2.70052374938548, 8.27035398249302, 1.51904460902933>
	>>> 2 / ans
	<0.740597078790777, 0.241827617564335, 1.31661702896138>

Variables can be deleted using `~`:

	>>> a = 4
	4
	>>> ~a
	>>> a
	Name Error: No variable named 'a' found.
	>>> f(x) = 2x + 5
	>>> ~f
	>>> f(3)
	Name Error: No variable named 'f' found.

And the interpreter can be reset using `~~~`:

	>>> a = 4
	4
	>>> b = 7
	7
	>>> f(x) = 4x - 3!
	>>> ~~~
	>>> a
	Name Error: No variable named 'a' found.
	>>> f(3)
	Name Error: No variable named 'f' found.

Error messages attempt to be clear:

	>>> 3 / (1 - 1)
	Math Error: Division by zero.
	>>> sqrt()
	Type Error: Builtin 'sqrt' expects 1 argument, not 0.
	>>> sqrt(4, 3)
	Type Error: Builtin 'sqrt' expects 1 argument, not 2.
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

For curious users, there is a verbose printing feature. The level of verbosity is determined by the number of `?`s prepended to the input string. For verbosity >= 1, SuperCalc will print a parenthesized version of the parsed input before evaluation. This is useful to check the order of operations being evaluated. Also, for verbosity >= 2, SuperCalc will also print a verbose dump of the internal parse tree.

Examples of verbose printing:

	>>> ? 3 + 4 - 2
	((3 + 4) - 2)
	5
	>>> ?? 8 - 9(6^2 + 3/7)^3
	- (
	  [a] 8
	  [b] * (
	    [a] 9
	    [b] ^ (
	      [a] + (
	        [a] ^ (
	          [a] 6
	          [b] 2
	        )
	        [b] / (
	          [a] 3
	          [b] 7
	        )
	      )
	      [b] 3
	    )
	  )
	)
	(8 - (9 * (((6 ^ 2) + (3 / 7)) ^ 3)))
	-149229631/343 (-435071.810495627)

Another usage of SuperCalc's verbose output is with functions. For verbosity >= 1, SuperCalc will print a parenthesized version of the function declaration, showing the function's name, argument names, and body. For verbosity >= 2, SuperCalc will also print the function's name, argument names, and the parse tree of it's body.

Examples of printing functions verbosely:

	>>> f(x) = 3x
	>>> ?f
	f(x) = (3 * x)
	>>> g(x, y) = x^2 - 2x*y + 1
	>>> ??g
	g(x, y) {
	  + (
	    [a] - (
	      [a] ^ (
	        [a] x
	        [b] 2
	      )
	      [b] * (
	        [a] * (
	          [a] 2
	          [b] x
	        )
	        [b] y
	      )
	    )
	    [b] 1
	  )
	}
	g(x, y) = (((x ^ 2) - ((2 * x) * y)) + 1)

##Building

To build on Linux, you need Automake and Autoconf:

	$ ./autogen.sh
	$ ./configure
	$ make
