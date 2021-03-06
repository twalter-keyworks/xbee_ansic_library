/*
 * Copyright (c) 2010-2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
/*
	Code for testing 64-bit integer support.

	Generate tables of known values.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#define SHIFT_BITS 13

#define JSLL_HI(n)	(uint32_t)(n >> 32)
#define JSLL_LO(n)	(uint32_t)(n)

#define jsll_print_init( n) \
	printf( "JSLL_INIT( 0x%08" PRIx32 ", 0x%08" PRIx32 " )", JSLL_HI(n), JSLL_LO(n))

void jsll_rand( uint64_t *n)
{
	// rand only generates 31 bits, so use 3 to get a random 64-bit value
	*n = ((uint64_t)(uint32_t) rand() << 33)
		^ ((uint64_t)(uint32_t) rand() << 16)
		+ (uint32_t) rand();
}

#define BINARY_OP(op) \
	{ c = a op b; jsll_print_init( c); printf( ",\t// a %s b\n\t", #op); }

#define UNARY_OP(op) \
	{ c = op a; jsll_print_init( c); printf( ",\t// %sa\n\t", #op); }

#define TF(exp)	(exp) ? "TRUE" : "FALSE"

void jsll_gen( void)
{
	int i;
	uint64_t a, b, c;
	int32_t i32;
	uint32_t ui32;

	puts( "// file generated by jsll_gen.c on a system with 64-bit integers");
	puts( "");
	printf( "#define SHIFT_BITS %u\n", SHIFT_BITS);
	puts( "typedef struct testrec_t {");
	puts( "   JSInt64 a, b, add, sub, mul, and, or, xor, not, neg;");
	puts( "   JSInt64 shl, shr, ushr;");
	puts( "   int32_t l2i;");
	puts( "   uint32_t l2ui;");
	puts( "   JSInt64 i2l;");
	puts( "   JSUint64 ui2l;");
	puts( "   const char *hexstr, *decstr, *udecstr;");
	puts( "   bool_t gtu, ltu, gt, lt;");
	puts( "} testrec_t;");
	puts( "");
	puts( "const testrec_t tests[] = {");

	for (i = 0; i < 20; ++i)
	{
		if (i)
		{
			jsll_rand( &a);
		}
		else
		{
			a = 0;
		}
		jsll_rand( &b);

		printf( "{ // record %u\n\t", i);
		jsll_print_init( a);
		printf( ",\t// a\n\t");
		jsll_print_init( b);
		printf( ",\t// b\n\t");

		BINARY_OP(+);		// add
		BINARY_OP(-);		// sub
		BINARY_OP(*);		// mul

		BINARY_OP(&);		// and
		BINARY_OP(|);		// or
		BINARY_OP(^);		// xor

		UNARY_OP(~);		// not
		UNARY_OP(-);		// neg

		c = a << SHIFT_BITS;
		jsll_print_init( c);
		printf( ",\t// a << %u\n\t", SHIFT_BITS);

		c = (int64_t) a >> SHIFT_BITS;
		jsll_print_init( c);
		printf( ",\t// a >> %u (signed)\n\t", SHIFT_BITS);

		c = a >> SHIFT_BITS;
		jsll_print_init( c);
		printf( ",\t// a >> %u (unsigned)\n\t", SHIFT_BITS);

		i32 = (int32_t) a;
		ui32 = (uint32_t) a;
		printf( "%" PRId32 ", UINT32_C(%" PRIu32
			"),\t// (int32_t) a, (uint32_t) a\n\t", i32, ui32);

		c = (int64_t) i32;
		jsll_print_init( c);
		printf( ",\t// (int64_t)(int32_t)a\n\t");

		c = (uint64_t) ui32;
		jsll_print_init( c);
		printf( ",\t// (uint64_t)(uint32_t)a\n\t");

		printf( "\"%016" PRIx64 "\",\t// a (hexadecimal)\n\t", a);
		printf( "\"%" PRId64 "\",\t// a (signed decimal)\n\t", a);
		printf( "\"%" PRIu64 "\",\t// a (unsigned decimal)\n\t", a);

		printf ("%s, %s,\t// a > b, a < b (unsigned)\n",
			TF(a > b), TF(a < b));

		printf ("\t%s, %s,\t// a > b, a < b (signed)\n",
			TF((int64_t)a > (int64_t)b), TF((int64_t)a < (int64_t)b));

		puts( "},");
	}
	puts( "};");
}

// generate the vectors for the shift test
void jsll_gen_shift( void)
{
	uint64_t base, shl, ushr;
	int64_t shr;
	int i;
	unsigned char shift;

	puts( "typedef struct shiftrec_t {");
	puts( "   JSUint64 shl, ushr;");
	puts( "   JSInt64 shr;");
	puts( "} shiftrec_t;");
	puts( "typedef struct shifttest_t {");
	puts( "   JSUint64 base;");
	puts( "   shiftrec_t shift[63];");
	puts( "} shifttest_t;");

	puts( "const shifttest_t shift_tests[] = {");
	for (i = 0; i < 4; ++i)
	{
		jsll_rand( &base);

		// set high (sign) bit of base based on the value of i
		// ensures two tests with bit set and two without
		if (i & 1)
		{
			base |= (UINT64_C(1) << 63);
		}
		else
		{
			base &= ~(UINT64_C(1) << 63);
		}
		printf( "\t{ ");
		jsll_print_init( base);
		printf( ",\t// record %u\n", i);
		puts( "\t {");
		for (shift = 1; shift < 64; ++shift)
		{
			shl = base << shift;
			ushr = base >> shift;
			shr = (int64_t)base >> shift;
			printf( "\t\t{ ");
			jsll_print_init( shl);
			printf( ",\t// shift left %u\n\t\t  ", shift);
			jsll_print_init( ushr);
			printf( ",\t// shift right (logical/unsigned)\n\t\t  ");
			jsll_print_init( shr);
			printf( "\t// shift right (arithmetic/signed)\n\t\t},\n");
		}
		puts( "\t },\n\t},");
	}
	puts( "};");
}

int main()
{
	jsll_gen();
	jsll_gen_shift();

	return 0;
}
