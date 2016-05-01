/******************************************************************************
 * @file   main.c
 * @author Federico Iannucci
 * @date   29 dic 2015
 * @brief  Test main for the FAP library
 ******************************************************************************/

#include "fap.h"

using namespace std;

int main(int argc, const char *argv[]) {

  //  int i1 = 355646, i2 = 4348, i3 = 567;
  //  IntegerType ci1((uint64_t) i1, 60, true), ci2((uint64_t) i2, 60, true),
  //  ci3(
  //      (uint64_t) i3, 60, true);
  //  cout << "Add : " << i1 << "\n" << i2 << "\n" << i1 + i2 + i3 << "\n";
  //  cout << ci1 << "\n" << ci2 << "\n" << ci1 + ci2 + ci3 << "\n";
  //  cout << (int) ci1 << "\n" << (int) ci2 << "\n" << ci3 << "\n"
  //       << (int) (ci1 + ci2 + ci3) << "\n";
  //
  //  cout << "Sub : " << i1 << "\n" << i2 << "\n" << i3 << "\n" << i1 - i2 <<
  //  "\n";
  //  cout << ci1 << "\n" << ci2 << "\n" << ci1 - ci2 << "\n";
  //  cout << (int) ci1 << "\n" << (int) ci2 << "\n" << (int) (ci1 - ci2) <<
  //  "\n";
  //
  //  cout << "Mul : " << i1 << "\n" << i2 << "\n" << i3 << "\n" << i1 * i2 * i3
  //       << "\n";
  //  cout << ci1 << "\n" << ci2 << "\n" << ci3 << "\n" << ci1 * ci2 * ci3 <<
  //  "\n";
  //  cout << (int) ci1 << "\n" << (int) ci2 << "\n" << (int) ci3 << "\n"
  //       << (int) (ci1 * ci2 * ci3) << "\n";
  //
  //  cout << "Div : " << i1 << "\n" << i2 << "\n" << i1 / i2 << "\n";
  //  cout << ci1 << "\n" << ci2 << "\n" << ci1 / ci2 << "\n";
  //  cout << (int) ci1 << "\n" << (int) ci2 << "\n" << (int) (ci1 / ci2) <<
  //  "\n";
  //  exit(1);

  //  VariablePrecisionMap map;
  //  map.insert(pair<string, FAP_fp_prec_t>("o_pixel", { 11, 5 }));
  //  map.insert(pair<string, FAP_fp_prec_t>("w", { 11, 18 }));
  //  for (int i = 18; i >= 8; --i) {
  //    map.at("o_pixel").mant_size = i;
  //    cout << calculateAccuracySobel(map) << endl;
  //  }

  ::fap::FloatingPointType a, b;
  a = 10.57;
  b = 67.12;
  ::std::cout << a << (double)a << "\n";
  ::std::cout << a*b << (double)(a*b) << "\n";
  a.changePrec({10,52});
  ::std::cout << a << "\n" << (double)a << "\n";
  ::std::cout << a*b << (double)(a*b) << "\n";
  return 0;

  ::std::string alg = "test";
#ifdef _FAP_LIBRARY_
  if (alg == "test") {
    printf("Running testing ... \n");
    while (1) {
      ::fap::FloatingPointType::test((float)1, (float)1);
      ::fap::FloatingPointType::test(GENERATE_RAND_FLOAT, GENERATE_RAND_FLOAT);
      ::fap::FloatingPointType::test(GENERATE_RAND_FLOAT, -GENERATE_RAND_FLOAT);
      ::fap::FloatingPointType::test(-GENERATE_RAND_FLOAT, GENERATE_RAND_FLOAT);
      ::fap::FloatingPointType::test(-GENERATE_RAND_FLOAT,
                                     -GENERATE_RAND_FLOAT);
      ::fap::FloatingPointType::test(GENERATE_RAND_DOUBLE,
                                     GENERATE_RAND_DOUBLE);
      ::fap::FloatingPointType::test(GENERATE_RAND_DOUBLE,
                                     -GENERATE_RAND_DOUBLE);
      ::fap::FloatingPointType::test(-GENERATE_RAND_DOUBLE,
                                     GENERATE_RAND_DOUBLE);
      ::fap::FloatingPointType::test(-GENERATE_RAND_DOUBLE,
                                     -GENERATE_RAND_DOUBLE);
    }
    return 0;
  }
#endif
#ifdef _FAP_LIBRARY_VIVADO_
  if (alg == "test_vivado") {
    printf("Running testing for Vivado Version ... \n");
    while (1) {
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test((float)1, (float)1);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(GENERATE_RAND_FLOAT,
                                                           GENERATE_RAND_FLOAT);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          GENERATE_RAND_FLOAT, -GENERATE_RAND_FLOAT);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(-GENERATE_RAND_FLOAT,
                                                           GENERATE_RAND_FLOAT);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          -GENERATE_RAND_FLOAT, -GENERATE_RAND_FLOAT);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          GENERATE_RAND_DOUBLE, GENERATE_RAND_DOUBLE);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          GENERATE_RAND_DOUBLE, -GENERATE_RAND_DOUBLE);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          -GENERATE_RAND_DOUBLE, GENERATE_RAND_DOUBLE);
      ::fap::vivado::FloatingPointTypeVivado<11, 52>::test(
          -GENERATE_RAND_DOUBLE, -GENERATE_RAND_DOUBLE);
    }
    return 0;
  }
#endif
  return 0;
}
