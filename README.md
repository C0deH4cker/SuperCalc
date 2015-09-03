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

	sc> (2 / 7) ^ 2
	4/49 (0.0816326530612245)
	sc> -(3 + 4!/7)^3
	-91125/343 (-265.67055393586)

Variables are supported:

	sc> a = 5
	5
	sc> a * 3
	15
	sc> b = 2 * a + 4
	14
	sc> ans + 4
	18

Variable names may only contain alphanumeric characters and '_', but the first character may not be a number.

Shorthand notations for variable modification work as well using any of the binary operators:

	sc> x = 4
	4
	sc> x += 3
	7
	sc> x *= 2
	14
	sc> x /= 3
	14/3 (4.66666666666667)
	sc> x *= 6
	28
	sc> x %= 2
	0
	sc> x += 8
	8
	sc> x ^= 2
	64

Functions are also supported:

	sc> f(x) = 3x + 4
	sc> f(7)
	25

Even with multiple arguments:

	sc> f(x, y) = x + y
	sc> f(3, 5)
	8
	sc> g(x, y, z) = f(x, y) * z
	sc> g(1, 2, 3)
	9

Function arguments do not affect variables:

	sc> x = 7
	7
	sc> f(x) = 4x
	sc> f(6)
	24
	sc> x
	7

Functions can use global variables:

	sc> myFunc(arg) = 3 * arg + glb
	sc> glb = 7
	7
	sc> myFunc(4)
	19
	sc> glb += 3
	10
	sc> myFunc(4)
	22
	sc> other(x) = x + ans
	sc> 8
	8
	sc> other(4)
	12
	sc> other(4)
	16

Functions *are* variables:

	sc> f(x, y) = x^2 - y^2
	sc> g = f
	sc> g(4, 3)
	7
	sc> h(x) = x^2
	sc> getFunc() = h
	sc> func = getFunc()
	sc> func(3)
	9

Vectors are supported as well:

	sc> a = <1, 2, 3>
	<1, 2, 3>
	sc> b = <6, 5, 3>
	<6, 5, 3>
	sc> a - b
	<-5, -3, 0>
	sc> 6 * a - b
	<0, 7, 15>

Vector specific builtins:

* `dot(vector1, vector2)`
* `cross(vector1, vector2)`
* `map(function, vector)`
* `elem(vector, index)` -> `vector[index]`
* `mag(vector)` -> `|vector|`
* `norm(vector)` -> Normalize the vector (`vector / |vector|`)

Vector functions:
	
	sc> a = <1, 2, 3>
	<1, 2, 3>
	sc> b = <6, 5, 3>
	<6, 5, 3>
	sc> cross(a, b)
	<-9, 15, -7>
	sc> dot(a, b)
	25

Function calls and vector subscripting is recursive:

	sc> getVec() = <1, 2, 3, <4, 5>>
	sc> getVec()[3][1]
	5
	sc> getFunc() = getVec
	sc> getFunc()()[3][0]
	4

Examples using `map`:

	sc> a = <3, 4, 5>
	<3, 4, 5>
	sc> f(x) = 2x + 1
	sc> map(f, a)
	<7, 9, 11>
	sc> angles = <0, pi/2, pi, 3pi/2, 2pi>
	<0, 1.5707963267949, 3.14159265358979, 4.71238898038469, 6.28318530717959>
	sc> map(sin, angles)
	<0, 1, 0, -1, 0>
	sc> add1(x) = 1 + x
	sc> map(add1, map(sqrt, <1, 4, 9, 16, 20, 16/9>))
	<2, 3, 4, 5, 5.47213595499958, 7/3>


Vectors can have any dimension greater than one:

	sc> a = <7, 2, 5.5, 7.6>
	<7, 2, 5.5, 7.6>
	sc> dot(a, <1, 2, 3, 4>)
	57.9
	sc> c = <1, 2>
	<1, 2>
	
Multiplying or dividing two vectors uses their components:

	sc> <1, 2, 3> * <4, 7, 2>
	<4, 14, 6>
	sc> <1, 4, 5> / <4, 6, 2>
	<1/4, 2/3, 5/2>

Vectors even support scalar operations:

	sc> a = <4, 7, -3>
	<4, 7, -3>
	sc> a + 2
	<4.92998110995055, 8.62746694241347, -3.69748583246292>
	sc> ans / 3
	<1.64332703665018, 2.87582231413782, -1.23249527748764>
	sc> ans ^ 2
	<2.70052374938548, 8.27035398249302, 1.51904460902933>
	sc> 2 / ans
	<0.740597078790777, 0.241827617564335, 1.31661702896138>

Variables can be deleted using `~`:

	sc> a = 4
	4
	sc> ~a
	sc> a
	Name Error: No variable named 'a' found.
	sc> f(x) = 2x + 5
	sc> ~f
	sc> f(3)
	Name Error: No variable named 'f' found.

And the interpreter can be reset using `~~~`:

	sc> a = 4
	4
	sc> b = 7
	7
	sc> f(x) = 4x - 3!
	sc> ~~~
	sc> a
	Name Error: No variable named 'a' found.
	sc> f(3)
	Name Error: No variable named 'f' found.

Error messages attempt to be clear:

	sc> 3 / (1 - 1)
	Math Error: Division by zero.
	sc> sqrt()
	Type Error: Builtin 'sqrt' expects 1 argument, not 0.
	sc> sqrt(4, 3)
	Type Error: Builtin 'sqrt' expects 1 argument, not 2.
	sc> sqrt(-1)
	Math Error: Builtin function 'sqrt' returned an invalid value.
	sc> a
	Name Error: No variable named 'a' found.
	sc> 17 $ 8
	Syntax Error: Unexpected character: '$'.
	sc> 8 +
	Syntax Error: Premature end of input.
	sc> pi(1, 2)
	Type Error: Builtin 'pi' is not a function.

For curious users, there is a verbose printing feature. The level of verbosity is determined by the number of `?`s prepended to the input string. For verbosity >= 1, SuperCalc will print a parenthesized version of the parsed input before evaluation. This is useful to check the order of operations being evaluated. Also, for verbosity >= 2, SuperCalc will also print a verbose dump of the internal parse tree. Verbosity >= 3 will force pretty printing (with unicode characters).

Examples of verbose printing:

	sc> ?w 3 + 4 - 2
	(3 + 4) - 2
	5
	sc> ?rwt 8 - 9(6^2 + 3/7)^3
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
	8 - (9 * (((6 ^ 2) + (3 / 7)) ^ 3))
	8 - 9 * (6 ^ 2 + 3 / 7) ^ 3
	-149229631/343 (-435071.810495627)

Another usage of SuperCalc's verbose output is with functions. For verbosity >= 1, SuperCalc will print a parenthesized version of the function declaration, showing the function's name, argument names, and body. For verbosity >= 2, SuperCalc will also print the function's name, argument names, and the parse tree of its body.

Examples of printing functions verbosely:

	sc> f(x) = 3x
	sc> ? f
	f(x) = 3 * x
	sc> ?p sqrt(4)
	√(4)
	2
	sc> g(x, y) = x^2 - 2x*y + 1
	sc> ?rt g
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
	sc> ?x g
	<vardata name="g">
	  <func>
	    <argnames>
	      <arg name="x"/>
	      <arg name="y"/>
	    </argnames>
	    <expr>
	      <add>
	        <sub>
	          <pow>
	            <var name="x"/>
	            <int>2</int>
	          </pow>
	          <mul>
	            <mul>
	              <int>2</int>
	              <var name="x"/>
	            </mul>
	            <var name="y"/>
	          </mul>
	        </sub>
	        <int>1</int>
	      </add>
	    </expr>
	  </func>
	</vardata>

##Building

To build on Linux or Mac OS X, you can use Automake and Autoconf:

	$ ./autogen.sh
	$ ./configure
	$ make
