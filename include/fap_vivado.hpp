/******************************************************************************
 * @file   fap_vivado.hpp
 * @author Federico Iannucci
 * @date   10 gen 2016
 * @brief  Fap Library adapted for Xilinx - Vivado HSL
 ******************************************************************************/
#ifndef SRC_INCLUDE_FAP_VIVADO_HPP_
#define SRC_INCLUDE_FAP_VIVADO_HPP_

//#define _FAP_VIVADO_DEBUG_

#define _FAP_LIBRARY_VIVADO_

#include "ap_int.h"
#include <iostream>

#define FLOAT_SIGN_SIZE               1
#define FLOAT_EXP_SIZE                8
#define FLOAT_MANT_SIZE               23
#define FLOAT_SIZE                    FLOAT_EXP_SIZE + FLOAT_MANT_SIZE + FLOAT_SIGN_SIZE

#define DOUBLE_SIGN_SIZE              1
#define DOUBLE_EXP_SIZE               11
#define DOUBLE_MANT_SIZE              52
#define DOUBLE_SIZE                   DOUBLE_EXP_SIZE + DOUBLE_MANT_SIZE + DOUBLE_SIGN_SIZE

#define GENERATE_RAND_FLOAT           (float)(((double)rand()/RAND_MAX) * 100)
#define GENERATE_RAND_DOUBLE          (((double)rand()/RAND_MAX)*100)

typedef ap_uint<8> ap_uint8_t;
typedef ap_uint<64> ap_uint64_t;
typedef ap_uint<128> ap_uint128_t;

#define VIVADO_EXPONENT_BIAS(prec)           VIVADO_MASK_LOWER_HIGH(ExpType, (prec - 1))

#define VIVADO_MASK_BIT_HIGH(type, pos)      (type)(((ap_uint128_t)0x01) << pos) ///< Create a mask with the pos-nth bit high
#define VIVADO_MASK_BIT_LOW(type, pos)       (type)((~VIVADO_MASK_BIT_HIGH(type, pos))) ///< Create a mask with the pos-nth bit low where the others are high
#define VIVADO_MASK_LOWER_LOW(type, num)     (type)(((((ap_uint128_t)0xFFFFFFFFFFFFFFFF) << 64) | 0xFFFFFFFFFFFFFFFF) << num) ///< Create a mask with all lower bit low for num positions
#define VIVADO_MASK_LOWER_HIGH(type, num)    (type)((~VIVADO_MASK_LOWER_LOW(type, num))) ///< Create a mask with all lower bit high for num positions

using namespace std;

namespace fap {
namespace vivado {

void shift_right_(ap_uint128_t* bit_vector, int to_shift, uint8_t* grs);
void shift_left_(ap_uint128_t* bit_vector, int to_shift, uint8_t* grs);

/// @brief Precision type
struct vivadoPrecType {
  /// @brief Ctor
  vivadoPrecType(uint8_t e_size = 0, uint8_t m_size = 0)
      : exp_size(e_size),
        mant_size(m_size) {
  }

  uint8_t exp_size;  ///< Size of the exponent
  uint8_t mant_size;  ///< Size of the mantissa
};

/// @brief Rounding methods
typedef enum {
  ROUND_TOWARD_0 = 0,
  ROUND_TOWARD_PINF,
  ROUND_TOWARD_NINF,
  ROUND_NEAREST
} vivadoRoundingMethod;

template<int expWidth, int mantWidth>
class FloatingPointTypeVivado {
  typedef ap_uint<1> SignType;
  typedef ap_uint<16> ExpType;
  typedef ap_uint<((mantWidth + 1) * 2) + 1> MantType;
 public:
  /// Ctors
  FloatingPointTypeVivado()
      : sign(0),
        exp(0),
        mant(0),
        grs(0),
        prec(vivadoPrecType(expWidth, mantWidth)) {
  }

  /// @brief Conversion from float
  FloatingPointTypeVivado(float f) {
    *this = f;
    this->changePrec(vivadoPrecType(expWidth, mantWidth));
  }

  /// @brief Conversion from double
  FloatingPointTypeVivado(double d) {
    *this = d;
    this->changePrec(vivadoPrecType(expWidth, mantWidth));
  }

  /// @brief Conversion from int
  FloatingPointTypeVivado(int i) {
    *this = (double) i;
    this->changePrec(vivadoPrecType(expWidth, mantWidth));
  }

  // Getters and Setters

  SignType getSign() const {
    return (this->sign & VIVADO_MASK_BIT_HIGH(SignType, 0));
  }

  void setSign(SignType sign) {
    this->sign = (sign & VIVADO_MASK_BIT_HIGH(SignType, 0));
  }

  ExpType getExp() const {
    return (this->exp & VIVADO_MASK_LOWER_HIGH(ExpType, this->prec.exp_size));
  }

  void setExp(ExpType exp) {
    this->exp = (exp & VIVADO_MASK_LOWER_HIGH(ExpType, this->prec.exp_size));
  }

  MantType getMant() const {
//    return (this->mant & VIVADO_MASK_LOWER_HIGH(fap_fp_mant_t, this->prec.mant_size));
    return this->mant;
  }

  void setMant(MantType mant) {
    this->mant =
        (mant & VIVADO_MASK_LOWER_HIGH(MantType, this->prec.mant_size));
  }

  MantType getMantHb() const {
    // If mant is 0 this is the zero floating point
    if (this->isZero()) {
      return (MantType) 0;
    }
    return (VIVADO_MASK_BIT_HIGH(MantType, this->prec.mant_size) | this->mant);
  }

  void setMantHb() {
    this->mant = this->getMantHb();
  }

  uint8_t getGrs() const {
    return grs;
  }

  void setGrs(uint8_t grs) {
    this->grs = grs;
  }

  vivadoPrecType getPrec() const {
    return prec;
  }

  void setPrec(vivadoPrecType prec) {
    this->prec = prec;
  }

#ifndef __HLS_SYN__
  const ::std::string& getName() const {
    return name;
  }

  void setName(const ::std::string& name) {
    this->name = name;
  }
#endif

  // Overloaded operators
  ///////////////////////////////////////////////////////////////////////////////
  // Conversion operators
  /// brief Conversion to float
  explicit operator float() const {
    // Check precision to check that 'enters' in the float type
    if (this->prec.exp_size > FLOAT_EXP_SIZE
        || this->prec.mant_size > FLOAT_MANT_SIZE) {
//      throw "FloatingPointTypeVivado precision is more than the float";
      return 0.0;
    }

    uint32_t ext_sign = ((uint32_t) this->getSign())
        << (FLOAT_SIZE - FLOAT_SIGN_SIZE);

    // De-bias with exponent size
    int64_t debiased_exp =
        this->getExp() - VIVADO_EXPONENT_BIAS(this->prec.exp_size);
    // Re-bias with the float size
    debiased_exp += VIVADO_EXPONENT_BIAS(FLOAT_EXP_SIZE);
    uint32_t ext_exp = ((uint32_t) debiased_exp)
        << (FLOAT_SIZE - FLOAT_SIGN_SIZE - FLOAT_EXP_SIZE);

    uint32_t ext_mant = ((uint32_t)this->getMant()
        << (FLOAT_MANT_SIZE - this->prec.mant_size));

    uint32_t fp = ext_sign | ext_exp | ext_mant;
    return *(float*) (&fp);
  }
  /// brief Conversion to double
  explicit operator double() const {
    // Check precision to check that 'enters' in the double type
    if (this->prec.exp_size > DOUBLE_EXP_SIZE
        || this->prec.mant_size > DOUBLE_MANT_SIZE) {
      return 0.0;
    }

    uint64_t ext_sign = ((uint64_t) this->getSign())
        << (DOUBLE_SIZE - DOUBLE_SIGN_SIZE);

    // De-bias with exponent size
    ExpType debiased_exp =
        this->getExp() - VIVADO_EXPONENT_BIAS(this->prec.exp_size);
    // Re-bias with the float size
    debiased_exp += VIVADO_EXPONENT_BIAS(DOUBLE_EXP_SIZE);
    uint64_t ext_exp = ((uint64_t) debiased_exp)
        << (DOUBLE_SIZE - DOUBLE_SIGN_SIZE - DOUBLE_EXP_SIZE);

    uint64_t ext_mant = ((uint64_t)this->getMant()
        << (DOUBLE_MANT_SIZE - this->prec.mant_size));

    uint64_t d = ext_sign | ext_exp | ext_mant;
    return *(double*) (&d);
  }
  /// brief Conversion to int
  explicit operator int() const {
    return (int) (double) *this;
  }
  /// brief Conversion to unsigned char
  explicit operator unsigned char() const {
    return (unsigned char) (int) *this;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Assign operators
  FloatingPointTypeVivado& operator=(const FloatingPointTypeVivado& rhs) {
    this->sign = rhs.getSign();
    this->exp = rhs.getExp();
    this->mant = rhs.getMant();
    this->grs = rhs.getGrs();
    this->prec = rhs.getPrec();
    return *this;
  }
  template<int expWidth2, int mantWidth2>
  FloatingPointTypeVivado& operator=(
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    this->sign = rhs.getSign();
    this->exp = rhs.getExp();
    this->mant = rhs.getMant();
    this->grs = rhs.getGrs();
    this->prec = rhs.getPrec();
    return *this;
  }

  FloatingPointTypeVivado& operator=(float rhs) {
    this->prec.mant_size = FLOAT_MANT_SIZE;
    this->prec.exp_size = FLOAT_EXP_SIZE;
    uint32_t f_as_4_byte = *(uint32_t *) (&rhs);
    this->setSign(f_as_4_byte >> (FLOAT_SIZE - FLOAT_SIGN_SIZE));
    this->setExp(
        (f_as_4_byte >> (FLOAT_SIZE - FLOAT_SIGN_SIZE - FLOAT_EXP_SIZE)));
    // Manage low width
    int width_diff = FLOAT_MANT_SIZE - MantType::width;
    if (width_diff > 0) {
      ap_uint128_t temp = (f_as_4_byte
          & VIVADO_MASK_LOWER_HIGH(uint32_t, FLOAT_MANT_SIZE));
      uint8_t temp_grs = 0;
      this->shift_right_(&temp, width_diff, &temp_grs);
      this->setMant(temp);
      this->setGrs(temp_grs);
      this->prec.mant_size -= width_diff;
    } else {
      this->setMant(f_as_4_byte);
      this->setGrs(0);
    }
    return *this;
  }
  FloatingPointTypeVivado& operator=(double rhs) {
    this->prec.mant_size = DOUBLE_MANT_SIZE;
    this->prec.exp_size = DOUBLE_EXP_SIZE;
    uint64_t d_as_8_byte = *(uint64_t *) (&rhs);
    this->setSign(d_as_8_byte >> (DOUBLE_SIZE - DOUBLE_SIGN_SIZE));
    this->setExp(
        (d_as_8_byte >> (DOUBLE_SIZE - DOUBLE_SIGN_SIZE - DOUBLE_EXP_SIZE)));
    // Manage low width
    int width_diff = DOUBLE_MANT_SIZE - MantType::width;
    if (width_diff > 0) {
      ap_uint128_t temp = (d_as_8_byte
          & VIVADO_MASK_LOWER_HIGH(uint64_t, DOUBLE_MANT_SIZE));
      uint8_t temp_grs = 0;
      this->shift_right_(&temp, width_diff, &temp_grs);
      this->setMant(temp);
      this->setGrs(temp_grs);
      this->prec.mant_size -= width_diff;
    } else {
      this->setMant(d_as_8_byte);
      this->setGrs(0);
    }
    return *this;
  }
  FloatingPointTypeVivado& operator=(int i) {
    *this = (double) i;
    return *this;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Arithmetic operators
  // Each operations is done at the minimum precision between the operands
  template<int expWidth2, int mantWidth2>
  FloatingPointTypeVivado& operator+=(
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& fp) {
    // Compute the minimum precision
//    constexpr int minExpWidth = expWidth < expWidth2? expWidth : expWidth2;
    constexpr int minMantWidth =
        mantWidth < mantWidth2 ?
            (2 * (mantWidth + 1)) + 1 : (2 * (mantWidth2 + 1)) + 1;

    //  ::std::cout << "Custom Add";
    FloatingPointTypeVivado& lhs = *this;
    FloatingPointTypeVivado<expWidth2, mantWidth2> rhs = fp;

#ifdef _FAP_VIVADO_DEBUG_
    lhs.setName("Result");
    rhs.setName("Operand 2");
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif

    this->adaptPrec(rhs);

    // One of the operands is NaN
    if (lhs.isNaN() || rhs.isNaN()) {
      // Set the result NaN
      lhs.setNaN();
      return lhs;
    }

    // One of the operands is infinity
    if (lhs.isInf() || rhs.isInf()) {
      if ((lhs.isPinf() && rhs.isNinf()) || (rhs.isPinf() && lhs.isNinf())) {
        // Set the result NaN
        lhs.setNaN();
      }
      if ((lhs.isPinf() && rhs.isPinf())) {
        // Set the result PINF
        lhs.setPinf();
      }
      if ((lhs.isNinf() && rhs.isNinf())) {
        // Set the result NINF
        lhs.setNinf();
      }
      // Not both are infinity, the infinity dominates
      if (lhs.isInf()) {
      } else if (rhs.isInf()) {
        lhs = rhs;
      }
      return lhs;
    }

    // Check special cases
    if (lhs.isZero()) {
      lhs = rhs;
      return lhs;
    } else if (rhs.isZero()) {
      return lhs;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Always take the minor exponent and take it to the major
    // Local vars
    // Compute in double result precision
    ap_uint<16> op_prec = this->prec.mant_size + 1;
    // When shift use the extended mantissa
    lhs.setMantHb();
    rhs.setMantHb();
    lhs.shift(-op_prec);
    rhs.shift(-op_prec);

    int exp_diff = lhs.getExp() - rhs.getExp();
#ifdef _FAP_VIVADO_DEBUG_
    printf("FAP_FP_ADD - exp_diff=%d\n", exp_diff);
    lhs.setName("Result Post Shift");
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif
    if (exp_diff > 0) {
      rhs.shift(exp_diff);
    } else if (exp_diff < 0) {
      lhs.shift(-exp_diff);
      lhs.setExp(rhs.getExp());
      lhs.setGrs(rhs.getGrs());
    }

    //  less_exp_op->exp += (exp_diff);

#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif
    // Now the mantix are alligned on radix point
    // Manage the mantissa and sign
    // If the sign are egual remains that
    if (lhs.getSign() == rhs.getSign()) {
      this->mant = (ap_uint<minMantWidth> ) lhs.getMant()
          + (ap_uint<minMantWidth> ) rhs.getMant();
    } else {
      if (lhs.getMant() >= rhs.getMant()) {
        //      this->sign = fap_fp_get_sign(&op1);
        this->mant = (ap_uint<minMantWidth> ) lhs.getMant()
            - (ap_uint<minMantWidth> ) rhs.getMant();
      } else {
        this->sign = rhs.getSign();
        this->mant = -(ap_uint<minMantWidth> ) lhs.getMant()
            + (ap_uint<minMantWidth> ) rhs.getMant();
      }
    }

    // Case the SUM is 0
    if (this->mant == 0) {
      lhs.setExp(0);
    }
#ifdef _FAP_VIVADO_DEBUG_
    lhs.setName("Result Post Add");
    ::std::cout << lhs << ::std::endl;
    lhs.setName("Normalize");
#endif
    // Normalize
    lhs.normalize(MantType::width, op_prec * 2);
    // Shift
    lhs.shift(op_prec);
    // Mask
    lhs.setMant(lhs.getMant());
    // Round
    lhs.round();
    ///////////////////////////////////////////////////////////////////////////////
#ifdef _FAP_VIVADO_DEBUG_
    lhs.setName("Result Final");
    ::std::cout << lhs << ::std::endl;
#endif
    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  FloatingPointTypeVivado& operator-=(
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& fp) {
    *this += (-fp);
    return *this;
  }

  template<int expWidth2, int mantWidth2>
  FloatingPointTypeVivado& operator*=(
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& fp) {
    // Compute the minimum precision
//    constexpr int minExpWidth = expWidth < expWidth2? expWidth : expWidth2;
    constexpr int minMantWidth =
        mantWidth < mantWidth2 ?
            (2 * (mantWidth + 1)) + 1 : (2 * (mantWidth2 + 1)) + 1;

    FloatingPointTypeVivado& lhs = *this;
    FloatingPointTypeVivado rhs = fp;

#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << "MUL" << ::std::endl;
    lhs.setName("Result");
    rhs.setName("Operand 2");
#endif

    this->adaptPrec(rhs);

    // One of the operands is NaN
    // x * NaN or NaN * NaN
    if (lhs.isNaN() || rhs.isNaN()) {
      // Set the result NaN
      lhs.isNaN();
      return lhs;
    }

    // Set the sign
    lhs.setSign(lhs.getSign() ^ rhs.getSign());
    // One of the operands is infinity
    if (lhs.isInf() || rhs.isInf()) {
      // Infinity * Infinity
      if (lhs.isInf() && rhs.isInf()) {
        // Set the result NaN
        lhs.setNaN();
        return lhs;
      }
      // Not both are infinity, the infinity dominates
      // Infinity * 0
      if (lhs.isZero() || rhs.isZero()) {
        // Set the result NaN
        lhs.setNaN();
        return lhs;
      }
      lhs.setInf();
      return lhs;
    }

    // Check special cases
    // x * 0
    if (lhs.isZero() || rhs.isZero()) {
      lhs.setZero();
      return lhs;
    }

    lhs.setMantHb();
    rhs.setMantHb();
#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif

    // MASK_BIT_HIGH of mant size create a mask with the mant_size + 1 bit high
    // Create result product between mantissa and mask with mant_size * 2 mask
    int mant_double_size = (lhs.getPrec().mant_size + 1) * 2;
    this->mant = VIVADO_MASK_LOWER_HIGH(MantType, mant_double_size)
        & ((ap_uint<minMantWidth> ) lhs.getMant()
            * (ap_uint<minMantWidth> ) rhs.getMant());
    // Compute exponent subtracting the bias (exp + bias + exp + bias -> new_exp + 2bias -bias --> new_exp + bias
    lhs.setExp(
        lhs.getExp()
            + rhs.getExp() - VIVADO_EXPONENT_BIAS(lhs.getPrec().exp_size));
#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
#endif
    // Multiplication of mantissas create a double sized mantissa
    // Multiplying 2 number of the type 1.x * 1.x --> yy.xx so there are 2 bits after the radix,
    // there are problems when yy = 01 because there is a shift more
    // Manage this problem
    int actual_mant_prec = mant_double_size - 1;  // Covers 10/11
    lhs.normalize(MantType::width, actual_mant_prec);

    // Re-shift to mant_size
    int to_shift = actual_mant_prec - (lhs.getPrec().mant_size + 1);
    lhs.shift(to_shift);
#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
#endif
    // Mask
    lhs.setMant(lhs.getMant());
    // Round
    lhs.round();

    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  FloatingPointTypeVivado& operator/=(
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& fp) {
    // Compute the minimum precision
//    constexpr int minExpWidth = expWidth < expWidth2? expWidth : expWidth2;
    constexpr int minMantWidth =
        mantWidth < mantWidth2 ?
            (2 * (mantWidth + 1)) + 1 : (2 * (mantWidth2 + 1)) + 1;

    FloatingPointTypeVivado& lhs = *this;
    FloatingPointTypeVivado rhs = fp;

#ifdef _FAP_VIVADO_DEBUG_
    lhs.setName("Result");
    rhs.setName("Operand 2");
#endif

    this->adaptPrec(rhs);

    // One of the operands is NaN
    // x * NaN or NaN * NaN
    if (lhs.isNaN() || rhs.isNaN()) {
      // Set the result NaN
      lhs.isNaN();
      return lhs;
    }

    // Set the sign
    lhs.setSign(lhs.getSign() ^ rhs.getSign());
    // Dividend is infinity
    // - infinity/x
    if (lhs.isInf()) {
      // infinity/infinity
      if (rhs.isInf()) {
        lhs.isNaN();
        return lhs;
      }
      lhs.setInf();
    }
    // Divisor is infinity
    if (rhs.isInf()) {
      // x/infinity
      lhs.setZero();
      return lhs;
    }
    // Divisor is 0
    if (rhs.isZero()) {
      lhs.setInf();
      return lhs;
    }

    // Check special cases
    // Dividend is 0
    if (lhs.isZero()) {
      return lhs;
    }

    // Normal cases
    ap_uint<16> precision = lhs.getPrec().mant_size + 1;
    lhs.setMantHb();
    rhs.setMantHb();
#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif
    // Shift dividend
    int round_shift = (MantType::width - precision * 2);
    int dividend_shift = precision + round_shift;  // + round_shift for the guard,round and sticky bits
    lhs.shift(-dividend_shift);

#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif

    // MASK_BIT_HIGH of mant size create a mask with the mant_size + 1 bit high
    // Create result product between mantissa and mask with mant_size * 2 mask
    this->mant = (ap_uint<minMantWidth> ) lhs.getMant()
        / (ap_uint<minMantWidth> ) rhs.getMant();
    lhs.shift(round_shift);

    // Calculate exponent
    lhs.setExp(
        lhs.getExp() - rhs.getExp()
            + VIVADO_EXPONENT_BIAS(lhs.getPrec().exp_size) - 1);
#ifdef _FAP_VIVADO_DEBUG_
    ::std::cout << lhs << ::std::endl;
    ::std::cout << rhs << ::std::endl;
#endif
    // Multiplication of mantissas create a double sized mantissa
    lhs.normalize(MantType::width, precision);
    // Truncate to correct mant precision
    lhs.setMant(this->mant);
    // Round
    lhs.round();

    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  friend FloatingPointTypeVivado operator+(
      FloatingPointTypeVivado lhs,
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    lhs += rhs;
    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  friend FloatingPointTypeVivado operator-(
      FloatingPointTypeVivado lhs,
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    lhs -= rhs;
    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  friend FloatingPointTypeVivado operator*(
      FloatingPointTypeVivado lhs,
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    lhs *= rhs;
    return lhs;
  }

  template<int expWidth2, int mantWidth2>
  friend FloatingPointTypeVivado operator/(
      FloatingPointTypeVivado lhs,
      const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    lhs /= rhs;
    return lhs;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Unary Operator
  friend FloatingPointTypeVivado operator-(FloatingPointTypeVivado lhs) {
    lhs.setSign(~lhs.getSign());
    return lhs;
  }

  ///////////////////////////////////////////////////////////////////////////////
  // Relational operators
  template<int expWidth2, int mantWidth2>
  bool operator<=(const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    return static_cast<double>(*this) <= static_cast<double>(rhs);
  }
  template<int expWidth2, int mantWidth2>
  bool operator<(const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    return static_cast<double>(*this) < static_cast<double>(rhs);
  }
  template<int expWidth2, int mantWidth2>
  bool operator>=(const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    return static_cast<double>(*this) >= static_cast<double>(rhs);
  }
  template<int expWidth2, int mantWidth2>
  bool operator>(const FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    return static_cast<double>(*this) > static_cast<double>(rhs);
  }

  ///////////////////////////////////////////////////////////////////////////////
  bool isZero() const {
    return (this->getMant() == 0 && this->getExp() == 0);
  }
  bool isSubN() const {
    return (this->getMant() != 0 && this->getExp() == 0);
  }
  bool isInf() const {
    return (this->getMant() == 0
        && this->getExp()
            == (VIVADO_MASK_LOWER_HIGH(ExpType, this->prec.exp_size)));
  }
  bool isPinf() const {
    return (this->isInf() && this->getSign() == 0);
  }
  bool isNinf() const {
    return (this->isInf() && this->getSign() != 0);
  }
  bool isNaN() const {
    return (this->getMant() != 0
        && this->getExp()
            == (VIVADO_MASK_LOWER_HIGH(ExpType, this->prec.exp_size)));
  }

  void setZero() {
    this->setExp(0);
    this->setMant(0);
  }
  void setInf() {
    this->setMant(0);
    this->setExp((VIVADO_MASK_LOWER_HIGH(ExpType, this->prec.exp_size)));
  }
  void setPinf() {
    this->setInf();
    this->setSign(0);
  }
  void setNinf() {
    this->setInf();
    this->setSign(1);
  }
  void setNaN() {
    this->setInf();
    this->setMant(1);
  }

  ///////////////////////////////////////////////////////////////////////////////
  template<int expWidth2, int mantWidth2>
  void adaptPrec(FloatingPointTypeVivado<expWidth2, mantWidth2>& rhs) {
    vivadoPrecType min_prec, rhs_prec = rhs.getPrec();
    min_prec.exp_size =
        this->prec.exp_size < rhs_prec.exp_size ?
            this->prec.exp_size : rhs_prec.exp_size;

    min_prec.mant_size =
        this->prec.mant_size < rhs_prec.mant_size ?
            this->prec.mant_size : rhs_prec.mant_size;

    // Change precision
    this->changePrec(min_prec);
    rhs.changePrec(min_prec);
  }
  void changePrec(vivadoPrecType new_prec) {
    // Check if it's necessary to apply the rounding: shift + rounding method
    int to_shift = 0;
//#ifdef _FAP_VIVADO_DEBUG_
//    printf("\nFAP_FP_CHANGE_PREC - old_prec.exp=%d | old_prec.mant=%d\n",
//           this->prec.exp_size, this->prec.mant_size);
//    printf("FAP_FP_CHANGE_PREC - new_prec.exp=%d | new_prec.mant=%d\n",
//           new_prec.exp_size, new_prec.mant_size);
//    fap_print_binary(this, "FAP_FP_CHANGE_PREC - old");
//#endif
    if (this->prec.exp_size != new_prec.exp_size) {
      // Re-bias and round, if necessary, the exponent
      ap_uint128_t expanded_exp = this->exp;
      // De-bias the exponent
      expanded_exp -= VIVADO_EXPONENT_BIAS(this->prec.exp_size);

      // Rounding, reducing the "grain level" of representation
      to_shift = this->prec.exp_size - new_prec.exp_size;
      if (to_shift > 0) {
        uint8_t grs = 0x00;
        // Shift the exponent to the right to match the correct size
        this->shift_right_(&expanded_exp, to_shift, &grs);
        expanded_exp <<= to_shift;
        // Round to nearest
        //      fap_round_nearest(&expanded_exp, grs);
      }
      //    // Check limits of new exponent range : [-(2^n-1)+2; (2^n-1)-1]
      //    int lower_bound = -((VIVADO_MASK_LOWER_HIGH(fap_fp_exp_t, new_prec.exp_size) >> 2) << 1);
      //    int upper_bound = ((VIVADO_MASK_LOWER_HIGH(fap_fp_exp_t, new_prec.exp_size) >> 2) << 1) + 1;
      //    if (expanded_exp < lower_bound){
      //      expanded_exp = lower_bound;
      //    } else if (expanded_exp > upper_bound){
      //      expanded_exp = upper_bound;
      //    }

      // Re-bias
      expanded_exp += VIVADO_EXPONENT_BIAS(new_prec.exp_size);
      this->exp = (ExpType) expanded_exp;
    }
    // Update exponent size
    this->prec.exp_size = new_prec.exp_size;
//#ifdef _FAP_VIVADO_DEBUG_
//    fap_print_binary(this, "FAP_FP_CHANGE_PREC - after_exp");
//#endif
    if (this->prec.mant_size != new_prec.mant_size) {
      // Shift the mantissa to fit the new precision
      to_shift = this->prec.mant_size - new_prec.mant_size;
      this->shift(to_shift);

      // Update mantissa size
      this->prec.mant_size = new_prec.mant_size;
      // Apply round if necessary
      if (to_shift > 0) {
        // Round
        this->round();
      }
    }
  }

#ifndef __HLS_SYN__
  static void test(double op1, double op2) {
    ::std::cout << ".";
    double res;
    ::fap::vivado::FloatingPointTypeVivado<expWidth, mantWidth> fop1 = op1,
        fop2 = op2, fp_res, fapf_res_to_0, fapf_res_to_pinf, fapf_res_to_ninf;

    if ((double) fop1 != op1) {
      ::std::cout << "Operand1 doens't match" << ::std::endl;
      ::std::cout << op1 << " - " << (double) fop1 << ::std::endl;
      ::std::cout << FloatingPointTypeVivado<11, 52>(op1) << " - " << fop1
          << ::std::endl;
      exit(1);
    }
    if ((double) fop2 != op2) {
      ::std::cout << "Operand2 doens't match" << ::std::endl;
      ::std::cout << op2 << " - " << (double) fop2 << ::std::endl;
      ::std::cout << FloatingPointTypeVivado<11, 52>(op2) << " - " << fop2
          << ::std::endl;
      exit(1);
    }

    bool isDifferent = false;

    res = op1 + op2;
    fp_res = fop1 + fop2;
    if (res != (double) fp_res) {
      ::std::cout << "Fail on Addition" << ::std::endl;
      isDifferent = true;
    }

    res = op1 - op2;
    fp_res = fop1 - fop2;
    if (res != (double) fp_res) {
      ::std::cout << "Fail on Sub" << ::std::endl;
      isDifferent = true;
    }

    res = op1 * op2;
    fp_res = fop1 * fop2;
    if (res != (double) fp_res) {
      ::std::cout << "Fail on Mul" << ::std::endl;
      isDifferent = true;
    }

    res = op1 / op2;
    fp_res = fop1 / fop2;
    if (res != (double) fp_res) {
      ::std::cout << "Fail on Div" << ::std::endl;
//      isDifferent = true;
    }

    if (isDifferent) {
      ::std::cout
          << "*********************************************************\n";
      ::std::cout << res << "!=" << (double) fp_res << ::std::endl;

      fop1.setName("Operand 1");
      fop2.setName("Operand 2");
      ::std::cout << fop1 << ::std::endl;
      ::std::cout << fop2 << ::std::endl;
      fp_res.setName("Custom Result");
      ::std::cout << fp_res << ::std::endl;
      ::std::cout << "Real Result"
          << FloatingPointTypeVivado<expWidth, mantWidth>(res) << ::std::endl;

      exit(1);
    }
  }
#endif
  void shift(int to_shift) {
    ap_uint128_t ex_mant = this->mant;
    if (to_shift > 0) {
      shift_right_(&ex_mant, to_shift, &(this->grs));
    } else if (to_shift < 0) {
      shift_left_(&ex_mant, -to_shift, &(this->grs));
    }
    this->mant = (MantType) ex_mant;
  }
  void normalize(int max_prec, int actual_prec) {
    // First bit high relative to prec
    int first_bit_high = 0;
    while (((VIVADO_MASK_BIT_HIGH(MantType, ((max_prec - 1) - first_bit_high))
        & this->mant) == 0) && (first_bit_high < max_prec))
      first_bit_high++;
    if (first_bit_high != max_prec) {
      // Have to shift on the left or on the right?
      // Check the first high bit position respect to actual_prec
      int first_bit_high_pos = (max_prec - first_bit_high);
      int to_shift = (actual_prec - first_bit_high_pos);
#ifdef _FAP_VIVADO_DEBUG_
      ::std::cout << "FAPF_NORMALIZE - to_shift=" << -to_shift << "\n";
      ::std::cout << *this;
#endif
      this->shift(-to_shift);
      this->exp -= to_shift;
#ifdef _FAP_VIVADO_DEBUG_
      ::std::cout << *this;
#endif
      this->mant &= VIVADO_MASK_LOWER_HIGH(MantType, actual_prec);
    }
  }
  void round(vivadoRoundingMethod method = ROUND_NEAREST) {
    // Modify the grs to match the proper rounding method
    switch (method) {
      case ROUND_TOWARD_0: {
        this->grs = 0;
      }
        break;
      case ROUND_TOWARD_PINF: {
        if (this->getSign() == 0x00 && this->grs != 0x00) {
          this->grs = 0x07;
        } else {
          this->grs = 0x00;
        }
      }
        break;
      case ROUND_TOWARD_NINF: {
        if (this->getSign() == 0x01 && this->grs != 0x00) {
          this->grs = 0x07;
        } else {
          this->grs = 0x00;
        }
      }
        break;
      default:
        break;
    }

    // Apply nearest rounding method with grs updated
    if (this->grs == 0x4
        && ((this->mant & VIVADO_MASK_BIT_HIGH(MantType, 0)) == (MantType) 1)) {
      this->mant += 0x1;
    } else if (grs >= 0x05) {
      this->mant += 0x1;
    }
    // Check if the mant is now all zeros, but the hidden bit, mant_size-nth bit
    if ((this->mant & VIVADO_MASK_BIT_HIGH(MantType, this->prec.mant_size))
        != (MantType) 0) {
      this->exp += 0x1;
    }

    // The round method has been applied reset the grs
    this->grs = 0x00;
    this->setMant(this->mant);
  }

  ///////////////////////////////////////////////////////////////////////////////
  // I/O operators
#ifndef __HLS_SYN__
  friend ::std::ostream& operator<<(
      ::std::ostream& out,
      const FloatingPointTypeVivado<expWidth, mantWidth>& fp) {
    if (fp.getName() != "") {
      out << fp.getName() << " - ";
    }
    //  out << " s | exp              | mant                             | grs\n";
    //  for (ap_uint<16> i = 0; i < strlen(descr) + 3; ++i){
    //    out << " ";
    //  }
    out << "[" << (int) fp.getPrec().exp_size << ":"
        << (int) fp.getPrec().mant_size << "]";
    out << "[";
    out << setw(1) << (int) fp.getSign() << " | ";
    out << setw(16) << setfill('0') << ::std::hex << fp.getExp() << ::std::dec
        << " | ";
    out << setw(32) << hex << fp.getMant() << dec;
    out << " | ";
    out << (((fp.getGrs()) & VIVADO_MASK_BIT_HIGH(uint8_t, 2)) >> 2);
    out << (((fp.getGrs()) & VIVADO_MASK_BIT_HIGH(uint8_t, 1)) >> 1);
    out << (((fp.getGrs()) & VIVADO_MASK_BIT_HIGH(uint8_t, 0)));
    out << "]\n";

    return out;
  }
#else
  friend ::std::ostream& operator<<(
      ::std::ostream& out,
      const FloatingPointTypeVivado<expWidth, mantWidth>& fp) {
    out << fp.getName();
    return out;
  }
#endif

 private:
#ifndef __HLS_SYN__
  ::std::string name;
#endif
  SignType sign;  ///< Sign used 1 bit
  ExpType exp;  ///< Exponent on max 16 bit
  MantType mant;  ///< Mantissa on max 64 bit
  uint8_t grs;  ///< Guard, round and sticky bits of the mantissa
  vivadoPrecType prec;  ///< Information about the precision

  void shift_right_(ap_uint128_t* bit_vector, int to_shift, uint8_t* grs) {
    // Shift of to_shift and take the last 3 bits and update the grs bits
    // Check if there was the sticky bit
    uint8_t sticky_1 = 0x00;
    if ((*grs & VIVADO_MASK_LOWER_HIGH(uint8_t, to_shift)) != 0x00) {
      sticky_1 = 0x01;
    }
#ifdef _FAP_VIVADO_DEBUG_
    printf("\nFAP_FP_RSHIFT - to_shift=%d\n", to_shift);
    printf("FAP_FP_RSHIFT - sticky=%02x\n", sticky_1);
    printf("FAP_FP_RSHIFT - grs=%02x\n", *grs);
#endif
    if (to_shift < 3) {
      *grs >>= to_shift;
      *grs |= (uint8_t) (((*bit_vector)
          & VIVADO_MASK_LOWER_HIGH(ap_uint128_t, to_shift)) << (3 - to_shift));
      *grs |= sticky_1 != 0x00 ? 0x01 : 0x00;
    } else {
      *grs = (uint8_t) (((*bit_vector) >> (to_shift - 3))
          & VIVADO_MASK_LOWER_HIGH(uint8_t, 3));
      // Set the sticky bit, checking if the lower bits after it are != from 0 OR
      // If the previous grs bits were != 0x00
      if (((*bit_vector & VIVADO_MASK_LOWER_HIGH(ap_uint128_t, (to_shift - 3)))
          != 0 || sticky_1) && to_shift != 3) {
        *grs |= 0x1;  // Set s bit

      }
    }
    *bit_vector >>= to_shift;
#ifdef _FAP_VIVADO_DEBUG_
    printf("FAP_FP_RSHIFT - new_grs=%02x\n", *grs);
#endif
  }

  /// @brief Same as fap_fp_rshift_mant_, but to the left
  void shift_left_(ap_uint128_t* bit_vector, int to_shift, uint8_t* grs) {
#ifdef _FAP_VIVADO_DEBUG_
    printf("\nFAP_FP_LSHIFT - to_shift=%d\n", to_shift);
    printf("FAP_FP_LSHIFT - grs=%02x\n", *grs);
#endif
    // Shift of to_shift and take the last 3 bits and update the grs bits
    *bit_vector <<= to_shift;
    // Put the bit of grs
    // Zeros entered in the least significant bits of mantissa
    if (to_shift < 3) {
      *bit_vector |= (uint8_t) ((*grs & VIVADO_MASK_LOWER_HIGH(uint8_t, 3))
          >> (3 - to_shift));
    } else {
      *bit_vector |= ((ap_uint128_t) *grs & VIVADO_MASK_LOWER_HIGH(uint8_t, 3))
          << (to_shift - 3);
    }
    *grs = (*grs << to_shift) & VIVADO_MASK_LOWER_HIGH(uint8_t, 3);
#ifdef _FAP_VIVADO_DEBUG_
    printf("FAP_FP_LSHIFT - grs=%02x\n", *grs);
#endif
  }
};

}  // end namespace vivado
}  // end namespace fap

#endif /* SRC_INCLUDE_FAP_VIVADO_HPP_ */
