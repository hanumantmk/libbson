/*
 * Copyright 2015 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <bson.h>

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "bson-tests.h"
#include "TestSuite.h"


/*
 *----------------------------------------------------------------------------
 * dec128_to_string tests
 *
 * The following tests confirm functionality of the dec128_to_string function.
 *
 * All decimal test data is generated using the Intel Decimal Floating-Point
 * Math Library's bid128_from_string routine.
 *
 *----------------------------------------------------------------------------
 */


#define DEC128_FROM_ULLS(h, l) { .high = (h), .low = (l) }


void
test_dec128_to_string__infinity (void) {
   bson_dec128_t positive_infinity = DEC128_FROM_ULLS (0x7800000000000000, 0),
                 negative_infinity = DEC128_FROM_ULLS (0xf800000000000000, 0);
   char bid_string[BSON_DEC128_STRING];

   bson_dec128_to_string (positive_infinity, bid_string);
   assert (!strcmp (bid_string, "Infinity"));

   bson_dec128_to_string (negative_infinity, bid_string);
   assert (!strcmp (bid_string, "-Infinity"));
}


void
test_dec128_to_string__nan (void) {
   bson_dec128_t dec_pnan        = DEC128_FROM_ULLS(0x7c00000000000000, 0),
                 dec_nnan        = DEC128_FROM_ULLS(0xfc00000000000000, 0),
                 dec_psnan       = DEC128_FROM_ULLS(0x7e00000000000000, 0),
                 dec_nsnan       = DEC128_FROM_ULLS(0xfe00000000000000, 0),
                 dec_payload_nan = DEC128_FROM_ULLS(0x7e00000000000000, 12);

   /* All the above should just be NaN. */
   char bid_string[BSON_DEC128_STRING];

   bson_dec128_to_string (dec_pnan, bid_string);
   assert (!strcmp (bid_string, "NaN"));

   bson_dec128_to_string (dec_nnan, bid_string);
   assert (!strcmp (bid_string, "NaN"));

   bson_dec128_to_string (dec_psnan, bid_string);
   assert (!strcmp (bid_string, "NaN"));

   bson_dec128_to_string (dec_nsnan, bid_string);
   assert (!strcmp (bid_string, "NaN"));

   bson_dec128_to_string (dec_payload_nan, bid_string);
   assert (!strcmp (bid_string, "NaN"));
}


void
test_dec128_to_string__regular (void) {
   char bid_string[BSON_DEC128_STRING];
   bson_dec128_t one =  DEC128_FROM_ULLS (0x3040000000000000, 0x0000000000000001),
      zero = DEC128_FROM_ULLS (0x3040000000000000, 0x0000000000000000),
      two  = DEC128_FROM_ULLS (0x3040000000000000, 0x0000000000000002),
      negative_one = DEC128_FROM_ULLS (0xb040000000000000, 0x0000000000000001),
      negative_zero = DEC128_FROM_ULLS (0xb040000000000000, 0x0000000000000000),
      tenth = DEC128_FROM_ULLS (0x303e000000000000, 0x0000000000000001), // 0.1
      smallest_regular = DEC128_FROM_ULLS (0x3034000000000000, 0x00000000000004d2), // 0.001234
      largest_regular  = DEC128_FROM_ULLS (0x3040000000000000, 0x0000001cbe991a14), // 12345789012
      trailing_zeros = DEC128_FROM_ULLS(0x302a000000000000, 0x00000000075aef40), // 0.00123400000
      all_digits = DEC128_FROM_ULLS(0x2ffc3cde6fff9732, 0xde825cd07e96aff2); // 0.1234567890123456789012345678901234

   bson_dec128_to_string (one, bid_string);
   assert (!strcmp ("1", bid_string));

   bson_dec128_to_string (zero, bid_string);
   assert (!strcmp ("0", bid_string));

   bson_dec128_to_string (two, bid_string);
   assert (!strcmp ("2", bid_string));

   bson_dec128_to_string (negative_one, bid_string);
   assert (!strcmp ("-1", bid_string));

   bson_dec128_to_string (negative_zero, bid_string);
   assert (!strcmp ("-0", bid_string));

   bson_dec128_to_string (tenth, bid_string);
   assert (!strcmp ("0.1", bid_string));

   bson_dec128_to_string (smallest_regular, bid_string);
   assert (!strcmp ("0.001234", bid_string));

   bson_dec128_to_string (largest_regular, bid_string);
   assert (!strcmp ("123456789012", bid_string));

   bson_dec128_to_string (trailing_zeros, bid_string);
   assert (!strcmp ("0.00123400000", bid_string));

   bson_dec128_to_string (all_digits, bid_string);
   assert (!strcmp ("0.1234567890123456789012345678901234", bid_string));
}


void
test_dec128_to_string__scientific (void) {
   char bid_string[BSON_DEC128_STRING];

   bson_dec128_t huge = // 1.000000000000000000000000000000000E+6144
      DEC128_FROM_ULLS (0x5ffe314dc6448d93, 0x38c15b0a00000000),
   tiny =               // 1E-6176
      DEC128_FROM_ULLS (0x0000000000000000, 0x0000000000000001),
   neg_tiny =           // -1E-6176
      DEC128_FROM_ULLS (0x8000000000000000, 0x0000000000000001),
   large    =           // 9.999987654321E+112
      DEC128_FROM_ULLS (0x3108000000000000, 0x000009184db63eb1),
   largest  =           // 9.999999999999999999999999999999999E+6144
      DEC128_FROM_ULLS (0x5fffed09bead87c0, 0x378d8e63ffffffff),
   tiniest  =           // 9.999999999999999999999999999999999E-6143
      DEC128_FROM_ULLS (0x0001ed09bead87c0, 0x378d8e63ffffffff),
   full_house =         // 5.192296858534827628530496329220095E+33
      DEC128_FROM_ULLS (0x3040ffffffffffff, 0xffffffffffffffff);
   bson_dec128_to_string (huge, bid_string);
   assert (!strcmp ("1.000000000000000000000000000000000E+6144", bid_string));

   bson_dec128_to_string (tiny, bid_string);
   assert (!strcmp ("1E-6176", bid_string));

   bson_dec128_to_string (neg_tiny, bid_string);
   assert (!strcmp ("-1E-6176", bid_string));

   bson_dec128_to_string (neg_tiny, bid_string);
   assert (!strcmp ("-1E-6176", bid_string));

   bson_dec128_to_string (large, bid_string);
   assert (!strcmp ("9.999987654321E+112", bid_string));

   bson_dec128_to_string (largest, bid_string);
   assert (!strcmp ("9.999999999999999999999999999999999E+6144", bid_string));

   bson_dec128_to_string (tiniest, bid_string);
   assert (!strcmp ("9.999999999999999999999999999999999E-6143", bid_string));

   bson_dec128_to_string (full_house, bid_string);
   assert (!strcmp ("5.192296858534827628530496329220095E+33", bid_string));
}


void
test_dec128_to_string__zeros (void) {
   char bid_string[BSON_DEC128_STRING];

   bson_dec128_t zero = // 0
      DEC128_FROM_ULLS (0x3040000000000000, 0x0000000000000000);
   bson_dec128_t pos_exp_zero = // 0E+300
      DEC128_FROM_ULLS (0x3298000000000000, 0x0000000000000000);
   bson_dec128_t neg_exp_zero = // 0E-600
      DEC128_FROM_ULLS (0x2b90000000000000, 0x0000000000000000);

   bson_dec128_to_string (zero, bid_string);
   assert (!strcmp ("0", bid_string));

   bson_dec128_to_string (pos_exp_zero, bid_string);
   assert (!strcmp ("0E+300", bid_string));

   bson_dec128_to_string (neg_exp_zero, bid_string);
   assert (!strcmp ("0E-600", bid_string));
}


#define IS_NAN(dec) (dec).high == 0x7c00000000000000ull


void
test_dec128_from_string__invalid_inputs (void) {
   bson_dec128_t dec;
   bson_dec128_from_string (".", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string (".e", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("invalid", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("in", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("i", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("E02", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("..1", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("1abcede", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("1.24abc", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("1.24abcE+02", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("1.24E+02abc2d", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("E+02", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("e+02", &dec);
   IS_NAN (dec);
}


void
test_dec128_from_string__nan (void) {
   bson_dec128_t dec;
   bson_dec128_from_string ("NaN", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("+NaN", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("-NaN", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("-nan", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("1e", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("+nan", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("nan", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("Nan", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("+Nan", &dec);
   assert (IS_NAN (dec));
   bson_dec128_from_string ("-Nan", &dec);
   assert (IS_NAN (dec));
}


#define IS_PINFINITY(dec) (dec).high == 0x7800000000000000
#define IS_NINFINITY(dec) (dec).high  == 0xf800000000000000


void
test_dec128_from_string__infinity (void) {
   bson_dec128_t dec;
   bson_dec128_from_string ("Infinity", &dec);
   assert (IS_PINFINITY (dec));
   bson_dec128_from_string ("+Infinity", &dec);
   assert (IS_PINFINITY (dec));
   bson_dec128_from_string ("+Inf", &dec);
   assert (IS_PINFINITY (dec));
   bson_dec128_from_string ("-Inf", &dec);
   assert (IS_NINFINITY (dec));
   bson_dec128_from_string ("-Infinity", &dec);
   assert (IS_NINFINITY (dec));
}


bool dec128_equal(bson_dec128_t dec, uint64_t high, uint64_t low) {
   bson_dec128_t test = DEC128_FROM_ULLS(high, low);
   return memcmp(&dec, &test, sizeof(dec)) == 0;
}

void
test_dec128_from_string__simple (void) {
   bson_dec128_t one;
   bson_dec128_t negative_one;
   bson_dec128_t zero;
   bson_dec128_t negative_zero;
   bson_dec128_t number;
   bson_dec128_t number_two;
   bson_dec128_t negative_number;
   bson_dec128_t fractional_number;
   bson_dec128_t leading_zeros    ;
   bson_dec128_t leading_insignificant_zeros;

   bson_dec128_from_string ("1", &one);
   bson_dec128_from_string ("-1", &negative_one);
   bson_dec128_from_string ("0", &zero);
   bson_dec128_from_string ("-0", &negative_zero);
   bson_dec128_from_string ("12345678901234567", &number);
   bson_dec128_from_string ("989898983458", &number_two);
   bson_dec128_from_string ("-12345678901234567", &negative_number);

   bson_dec128_from_string ("0.12345", &fractional_number);
   bson_dec128_from_string ("0.0012345", &leading_zeros    );

   bson_dec128_from_string ("00012345678901234567", &leading_insignificant_zeros);

   assert (dec128_equal (one, 0x3040000000000000, 0x0000000000000001));
   assert (dec128_equal (negative_one, 0xb040000000000000, 0x0000000000000001));
   assert (dec128_equal (zero, 0x3040000000000000, 0x0000000000000000));
   assert (dec128_equal (negative_zero, 0xb040000000000000, 0x0000000000000000));
   assert (dec128_equal (number, 0x3040000000000000, 0x002bdc545d6b4b87));
   assert (dec128_equal (number_two, 0x3040000000000000, 0x000000e67a93c822));
   assert (dec128_equal (negative_number, 0xb040000000000000, 0x002bdc545d6b4b87));
   assert (dec128_equal (fractional_number, 0x3036000000000000, 0x0000000000003039));
   assert (dec128_equal (leading_zeros, 0x3032000000000000, 0x0000000000003039));
   assert (dec128_equal (leading_insignificant_zeros, 0x3040000000000000, 0x002bdc545d6b4b87));
}


void
test_dec128_from_string__scientific (void) {
   bson_dec128_t ten;
   bson_dec128_t ten_again;
   bson_dec128_t one;
   bson_dec128_t huge_exp;
   bson_dec128_t tiny_exp;
   bson_dec128_t fractional;

   bson_dec128_from_string ("10e0", &ten);
   bson_dec128_from_string ("1e1", &ten_again);
   bson_dec128_from_string ("10e-1", &one);

   assert (dec128_equal (ten, 0x3040000000000000, 0x000000000000000a));
   assert (dec128_equal (ten_again, 0x3042000000000000, 0x0000000000000001));
   assert (dec128_equal (one, 0x303e000000000000, 0x000000000000000a));

   bson_dec128_from_string ("12345678901234567e6111", &huge_exp);
   bson_dec128_from_string ("1e-6176", &tiny_exp);

   assert (dec128_equal (huge_exp, 0x5ffe000000000000, 0x002bdc545d6b4b87));
   assert (dec128_equal (tiny_exp, 0x0000000000000000, 0x0000000000000001));

   bson_dec128_from_string ("-100E-10", &fractional);

   assert (dec128_equal (fractional, 0xb02c000000000000, 0x0000000000000064));
}


void
test_dec128_from_string__large (void) {
   bson_dec128_t large;
   bson_dec128_t all_digits;
   bson_dec128_t largest;
   bson_dec128_t tiniest;
   bson_dec128_t full_house;

   bson_dec128_from_string ("12345689012345789012345", &large);
   bson_dec128_from_string ("1234567890123456789012345678901234", &all_digits);
   bson_dec128_from_string ("9.999999999999999999999999999999999E+6144", &largest);
   bson_dec128_from_string ("9.999999999999999999999999999999999E-6143", &tiniest);
   bson_dec128_from_string ("5.192296858534827628530496329220095E+33", &full_house);

   assert (dec128_equal (large, 0x304000000000029d, 0x42da3a76f9e0d979));
   assert (dec128_equal (all_digits, 0x30403cde6fff9732, 0xde825cd07e96aff2));
   assert (dec128_equal (largest, 0x5fffed09bead87c0, 0x378d8e63ffffffff));
   assert (dec128_equal (tiniest, 0x0001ed09bead87c0, 0x378d8e63ffffffff));
   assert (dec128_equal (full_house, 0x3040ffffffffffff, 0xffffffffffffffff));
}


void
test_dec128_from_string__exponent_normalization (void) {
   bson_dec128_t trailing_zeros;
   bson_dec128_t one_normalize;
   bson_dec128_t no_normalize;
   bson_dec128_t a_disaster;

   bson_dec128_from_string ("1000000000000000000000000000000000000000", &trailing_zeros);
   bson_dec128_from_string ("10000000000000000000000000000000000", &one_normalize);
   bson_dec128_from_string ("1000000000000000000000000000000000", &no_normalize);
   bson_dec128_from_string (
      "100000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000",
   &a_disaster);

   bson_dec128_t zero;
   bson_dec128_from_string ("1E-6177", &zero);

   assert (dec128_equal (trailing_zeros, 0x304c314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (one_normalize, 0x3042314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (no_normalize, 0x3040314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (a_disaster, 0x37cc314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (zero, 0x0000000000000000, 0x0000000000000000));
}


void
test_dec128_from_string__zeros (void) {
   bson_dec128_t zero;
   bson_dec128_t exponent_zero;
   bson_dec128_t large_exponent;
   bson_dec128_t negative_zero;

   bson_dec128_from_string ("0", &zero);
   bson_dec128_from_string ("0e-611", &exponent_zero);
   bson_dec128_from_string ("0e+6000", &large_exponent);
   bson_dec128_from_string ("-0e-1", &negative_zero);

   assert (dec128_equal (zero, 0x3040000000000000, 0x0000000000000000));
   assert (dec128_equal (exponent_zero, 0x2b7a000000000000, 0x0000000000000000));
   assert (dec128_equal (large_exponent, 0x5f20000000000000, 0x0000000000000000));
   assert (dec128_equal (negative_zero, 0xb03e000000000000, 0x0000000000000000));
}


void
test_dec128_from_string__round (void) {
   bson_dec128_t truncate;
   bson_dec128_t up;
   bson_dec128_t check_tie_up;
   bson_dec128_t check_tie_trunc;
   bson_dec128_t extra_digit_up;
   bson_dec128_t extra_digit_down;
   bson_dec128_t extra_digit_tie;
   bson_dec128_t extra_digit_tie_break;
   bson_dec128_t too_big;
   bson_dec128_t largest_binary;
   bson_dec128_t round_propagate;
   bson_dec128_t round_propagate_large;
   bson_dec128_t not_inf;
   bson_dec128_t round_propagate_inf;

   bson_dec128_from_string ("10E-6177", &truncate);
   bson_dec128_from_string ("15E-6177", &up);
   bson_dec128_from_string ("251E-6178", &check_tie_up);
   bson_dec128_from_string ("250E-6178", &check_tie_trunc);

   bson_dec128_from_string ("10000000000000000000000000000000006", &extra_digit_up);
   bson_dec128_from_string ("10000000000000000000000000000000003", &extra_digit_down);
   bson_dec128_from_string ("10000000000000000000000000000000005", &extra_digit_tie);
   bson_dec128_from_string ("100000000000000000000000000000000051", &extra_digit_tie_break);

   bson_dec128_from_string ("10000000000000000000000000000000006E6111", &too_big);

   bson_dec128_from_string ("12980742146337069071326240823050239", &largest_binary);

   bson_dec128_from_string ("99999999999999999999999999999999999", &round_propagate);
   bson_dec128_from_string (
      "9999999999999999999999999999999999999999999999999999999999999999999"
      "999999999999999999999999999999999"
   , &round_propagate_large);

   bson_dec128_from_string ("9999999999999999999999999999999999E6111", &not_inf);
   bson_dec128_from_string ("99999999999999999999999999999999999E6144", &round_propagate_inf);

   assert (dec128_equal (truncate, 0x0000000000000000, 0x0000000000000001));
   assert (dec128_equal (up, 0x0000000000000000, 0x0000000000000002));
   assert (dec128_equal (check_tie_up, 0x0000000000000000, 0x0000000000000003));
   assert (dec128_equal (check_tie_trunc, 0x0000000000000000, 0x0000000000000002));

   assert (dec128_equal (extra_digit_up, 0x3042314dc6448d93, 0x38c15b0a00000001));
   assert (dec128_equal (extra_digit_down, 0x3042314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (extra_digit_tie, 0x3042314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (extra_digit_tie_break, 0x3044314dc6448d93, 0x38c15b0a00000001));

   assert (dec128_equal (too_big, 0x7800000000000000, 0x0000000000000000));
   assert (dec128_equal (largest_binary, 0x3042400000000000, 0x0000000000000000));
   assert (dec128_equal (round_propagate, 0x3044314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (round_propagate_large, 0x30c6314dc6448d93, 0x38c15b0a00000000));
   assert (dec128_equal (not_inf, 0x5fffed09bead87c0, 0x378d8e63ffffffff));
   assert (dec128_equal (round_propagate_inf, 0x7800000000000000, 0x0000000000000000));
}

void
test_dec128_install (TestSuite *suite)
{
   TestSuite_Add (suite,
                  "/bson/dec128/to_string/infinity",
                  test_dec128_to_string__infinity);
   TestSuite_Add (suite,
                  "/bson/dec128/to_string/nan",
                  test_dec128_to_string__nan);
   TestSuite_Add (suite,
                  "/bson/dec128/to_string/regular",
                  test_dec128_to_string__regular);
   TestSuite_Add (suite,
                  "/bson/dec128/to_string/scientific",
                  test_dec128_to_string__scientific);
   TestSuite_Add (suite,
                  "/bson/dec128/to_string/zero",
                  test_dec128_to_string__zeros);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/invalid",
                  test_dec128_from_string__invalid_inputs);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/nan",
                  test_dec128_from_string__nan);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/infinity",
                  test_dec128_from_string__infinity);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/basic",
                  test_dec128_from_string__simple);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/scientific",
                  test_dec128_from_string__scientific);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/large",
                  test_dec128_from_string__large);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/exponent_normalization",
                  test_dec128_from_string__exponent_normalization);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/zero",
                  test_dec128_from_string__zeros);
   TestSuite_Add (suite,
                  "/bson/dec128/from_string/round",
                  test_dec128_from_string__round);
}
