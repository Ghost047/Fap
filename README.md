# FAP - Flexible Arbitrary Precision Numeric Library

FAP is a C++ numeric library that provides support for arbitrary precision operations for integer and floating point types.
FAP fills the empty space left by the lack of accessible arbitrary precision libraries that support actually arbitrary precision operations.

FLAP works on the binary representation of the numeric values changing the bit-width. In particular, FLAP alters the precision of an operation by neglecting some of least-significant bits, supporting different ways in order to do that. 

## Integer Types
Specifically for the integer types it supports two ways: 
1. **Zeroing**: all the neglected bits are considered as 0. This simplifies arithmetic operations because those bits are never involved in calculations. The introduced error can be estimated within the range $[0, 2^n-1]$, where $n$ is the number of neglected bits.
2. **Half-value**: the neglected bits are consider as a bit string with a leading 1 and all the remaining as 0. This representation makes a symmetrical initial error interval equals to $[-2^{n-1},2^{n-1}-1]$. Inside the library is called **compensation**.

## Floating Point Types
As for floating point, FLAP supports the IEEE 754 floating point standard and it is able to handle reduction for both the exponent and the mantissa fields.
The exponent is considered as an integer value, hence it is managed with the zeroing or half-value technique.
Conversely, the mantissa can be bit-width reduced and approximated by employing IEEE 754 rounding modes, such as rounding to the nearest, rounding toward 0, and so on.

## Internals
FAP internally *simulates* the IEEE procedures to compute the result of numeric operations, so it uses a double precision. Addition, multiplication and division of arbitrary width sized floating point types are complaint to the IEEE 754, for example, FAP manages the IEEE 754 float representation, the rounding techniques and the **GRS}** (guard round and sticky bit) bits of the mantissa.

## Implementation
FLAP provides two main classes: `IntegerType` and `FloatingPointType`.
The former encapsulates the integer types, from byte to long long integers, the latter encapsulates the IEEE 754 floating point types, float and double.
Both classes overloads the default operators for addition, subtraction, multiplication. Further the `FloatingPointType` overloads the division.
Provided operators can be even applied between two numeric types with different precision: FAP manages this case by performing the operation at the lowest or highest precision.

Furthermore, FAP integrates casting function in order to convert custom types to/from standard types. Indeed, when an operation involves a custom type with a standard type, the standard type is automatically cast.

## Usage
See the [test main](test/main.cpp) which contains examples of usage.

In order to manipulate double values the library uses the `uin128_t` type, so, in order to use the library, that type has to be supported by your compiler.

## Build Test
In order to compile the test main you needs CMake (> 3.3) to create the makefiles. Then, use the make target *fap_test*, e.g. make fap_test.

## Papers
The FAP Numeric Library is cited in several papers, under the alias of FLAP:
1. [An extendible design exploration tool for supporting approximate computing techniques](http://ieeexplore.ieee.org/document/7483888/)
2. [A Pruning Technique for B&B Based Design Exploration of Approximate Computing Variants](http://ieeexplore.ieee.org/abstract/document/7560284/) 

