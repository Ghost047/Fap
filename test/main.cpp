//===- Main.cpp -------------------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2015, 2016  Federico Iannucci (fed.iannucci@gmail.com)
// 
//  This file is part of Fap.
//
//  Fap is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Fap is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with Fap. If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//
/// \file Main.cpp
/// \author Federico Iannucci
/// \brief Flexible Arbitrary Precision Library.
///        Test main.
//===----------------------------------------------------------------------===//

#include "Fap.h"

using namespace std;

int main(int argc, const char *argv[]) {
  
  cout << "IntegerType:\n";
  // IntegerType
  int i1 = 355646, i2 = 4348, i3 = 567;
  // The previous 3 integers are put in integers with 60 bits instead of 64
  // and with the compensation enabled (which tries to reduce the overall error)
  ::fap::IntegerType ci1((uint64_t) i1, 60, true), ci2((uint64_t) i2, 60, true),
		  	  ci3((uint64_t) i3, 60, true);
  cout << "Addition : \n" 
       << "Number 1: " << i1 << "\n" 
	   << "Number 1: " << i2 << "\n"
	   << "Number 3: " << i3 << "\n"
	   << "Sum: " << i1 + i2 + i3 << "\n";
  cout << "Binary representation of IntegerType of 1: " << ci1 << "\n" 
       << "Binary representation of IntegerType of 2: " << ci2 << "\n"
	   << "Binary representation of IntegerType of 3: " << ci3 << "\n"
       << "Binary representation of IntegerType of the sum: " << ci1 + ci2 + ci3 << "\n";
  cout << "Int value of IntegerType of 1: " << (int) ci1 << "\n" 
       << "Int value of IntegerType of 2: " << (int) ci2 << "\n" 
       << "Int value of IntegerType of 3: " << (int) ci3 << "\n"
       << "Int value of IntegerType of the sum: " << (int) (ci1 + ci2 + ci3) << "\n";

  cout << "\n******************************************************************\n";
  cout << "FloatingPoint Type:\n";
  // FloatingPointType
  ::fap::FloatingPointType a, b;
  a = 10.57; // Default cast is double, so exp of 11 bits and mantissa of 52 bits
  b = 67.12; // Default cast is double, so exp of 11 bits and mantissa of 52 bits
  ::std::cout << "Binary representation of a: " << a 
              << "Double value of a: " << (double)a << "\n";
  ::std::cout << "Binary representation of a*b: " << a*b // b is automatically cast to FloatingPointType
              << "Double value of a: " << (double)(a*b) << "\n";
  a.changePrec({10,50}); // Change the bit-width of the exponent to 10 bits and mantissa to 50
  // It is possibile to specify the bit-widths also in the constructor
  ::std::cout << "Changed the bit-width of the exponent to 10 bits and mantissa to 50 bits:\n";
  ::std::cout << "Binary representation of a: " << a 
              << "Double value of a: " << (double)a << "\n";
  ::std::cout << "Binary representation of a*b: " << a*b // b is automatically cast to FloatingPointType with 		 compatible bit-width of exponent and mantissa
              << "Double value of a*b: " << (double)(a*b) << "\n";

  // These commented code can be used to see that the procedures used by the FloatingPointType class
  // are compliant with IEEE 754, indeed the result of every operation on two random float/double values
  // is the same with native float/double type and FloatingPointType objects
//  printf("Running testing ... \n");
//  while (1) {
//    ::fap::FloatingPointTye::test((float)1, (float)1);
//    ::fap::FloatingPointType::test(GENERATE_RAND_FLOAT, GENERATE_RAND_FLOAT);
//    ::fap::FloatingPointType::test(GENERATE_RAND_FLOAT, -GENERATE_RAND_FLOAT);
//    ::fap::FloatingPointType::test(-GENERATE_RAND_FLOAT, GENERATE_RAND_FLOAT);
//    ::fap::FloatingPointType::test(-GENERATE_RAND_FLOAT,
//                                   -GENERATE_RAND_FLOAT);
//    ::fap::FloatingPointType::test(GENERATE_RAND_DOUBLE,
//                                   GENERATE_RAND_DOUBLE);
//    ::fap::FloatingPointType::test(GENERATE_RAND_DOUBLE,
//                                   -GENERATE_RAND_DOUBLE);
//    ::fap::FloatingPointType::test(-GENERATE_RAND_DOUBLE,
//                                    GENERATE_RAND_DOUBLE);
//    :fap::FloatingPointType::test(-GENERATE_RAND_DOUBLE,
//                                  -GENERATE_RAND_DOUBLE);
//  }
  return 0;
}
