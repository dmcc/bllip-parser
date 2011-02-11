
#include "cephes.h"

#include <math.h>

/*							mconf.h
 *
 *	Common include file for math routines
 *
 *
 *
 * SYNOPSIS:
 *
 * #include "mconf.h"
 *
 *
 *
 * DESCRIPTION:
 *
 * This file contains definitions for error codes that are
 * passed to the common error handling routine mtherr()
 * (which see).
 *
 * The file also includes a conditional assembly definition
 * for the type of computer arithmetic (IEEE, DEC, Motorola
 * IEEE, or UNKnown).
 * 
 * For Digital Equipment PDP-11 and VAX computers, certain
 * IBM systems, and others that use numbers with a 56-bit
 * significand, the symbol DEC should be defined.  In this
 * mode, most floating point constants are given as arrays
 * of octal integers to eliminate decimal to binary conversion
 * errors that might be introduced by the compiler.
 *
 * For little-endian computers, such as IBM PC, that follow the
 * IEEE Standard for Binary Floating Point Arithmetic (ANSI/IEEE
 * Std 754-1985), the symbol IBMPC should be defined.  These
 * numbers have 53-bit significands.  In this mode, constants
 * are provided as arrays of hexadecimal 16 bit integers.
 *
 * Big-endian IEEE format is denoted MIEEE.  On some RISC
 * systems such as Sun SPARC, double precision constants
 * must be stored on 8-byte address boundaries.  Since integer
 * arrays may be aligned differently, the MIEEE configuration
 * may fail on such machines.
 *
 * To accommodate other types of computer arithmetic, all
 * constants are also provided in a normal decimal radix
 * which one can hope are correctly converted to a suitable
 * format by the available C language compiler.  To invoke
 * this mode, define the symbol UNK.
 *
 * An important difference among these modes is a predefined
 * set of machine arithmetic constants for each.  The numbers
 * MACHEP (the machine roundoff error), MAXNUM (largest number
 * represented), and several other parameters are preset by
 * the configuration symbol.  Check the file const.c to
 * ensure that these values are correct for your computer.
 *
 * Configurations NANS, INFINITIES, MINUSZERO, and DENORMAL
 * may fail on many systems.  Verify that they are supposed
 * to work on your computer.
 */

/*
Cephes Math Library Release 2.3:  June, 1995
Copyright 1984, 1987, 1989, 1995 by Stephen L. Moshier
*/


/* Constant definitions for math error conditions
 */

#define DOMAIN		1	/* argument domain error */
#define SING		2	/* argument singularity */
#define OVERFLOW	3	/* overflow range error */
#define UNDERFLOW	4	/* underflow range error */
#define TLOSS		5	/* total loss of precision */
#define PLOSS		6	/* partial loss of precision */

#define EDOM		33
#define ERANGE		34

/* Complex numeral.  */
typedef struct
	{
	double r;
	double i;
	} cmplx;

/* Long double complex numeral.  */
/*
typedef struct
	{
	long double r;
	long double i;
	} cmplxl;
*/

/* Type of computer arithmetic */

/* PDP-11, Pro350, VAX:
 */
/* #define DEC 1 */

/* Intel IEEE, low order words come first:
 */
/* #define IBMPC 1 */

/* Motorola IEEE, high order words come first
 * (Sun 680x0 workstation):
 */
/* #define MIEEE 1 */

/* UNKnown arithmetic, invokes coefficients given in
 * normal decimal format.  Beware of range boundary
 * problems (MACHEP, MAXLOG, etc. in const.c) and
 * roundoff problems in pow.c:
 * (Sun SPARCstation)
 */
#define UNK 1 

/* If you define UNK, then be sure to set BIGENDIAN properly. */
#define BIGENDIAN 0

/* Define this `volatile' if your compiler thinks
 * that floating point arithmetic obeys the associative
 * and distributive laws.  It will defeat some optimizations
 * (but probably not enough of them).
 *
 * #define VOLATILE volatile
 */
#define VOLATILE

/* For 12-byte long doubles on an i386, pad a 16-bit short 0
 * to the end of real constants initialized by integer arrays.
 *
 * #define XPD 0,
 *
 * Otherwise, the type is 10 bytes long and XPD should be
 * defined blank (e.g., Microsoft C).
 *
 * #define XPD
 */
#define XPD 0,

/* Define to support tiny denormal numbers, else undefine. */
/* #define DENORMAL 1 */

/* Define to ask for infinity support, else undefine. */
/* #define INFINITIES 1 */

/* Define to ask for support of numbers that are Not-a-Number,
   else undefine.  This may automatically define INFINITIES in some files. */
/* #define NANS 1 */

/* Define to distinguish between -0.0 and +0.0.  */
/* #define MINUSZERO 1 */

/* Define 1 for ANSI C atan2() function
   See atan.c and clog.c. */
#define ANSIC 1

/* Get ANSI function prototypes, if you want them. */
#ifdef __STDC__
#define ANSIPROT
/* #include "protos.h" */
#else
int mtherr();
#endif

/* Variable for error reporting.  See mtherr.c.  */
int merror;


/*							const.c
 *
 *	Globally declared constants
 *
 *
 *
 * SYNOPSIS:
 *
 * extern double nameofconstant;
 *
 *
 *
 *
 * DESCRIPTION:
 *
 * This file contains a number of mathematical constants and
 * also some needed size parameters of the computer arithmetic.
 * The values are supplied as arrays of hexadecimal integers
 * for IEEE arithmetic; arrays of octal constants for DEC
 * arithmetic; and in a normal decimal scientific notation for
 * other machines.  The particular notation used is determined
 * by a symbol (DEC, IBMPC, or UNK) defined in the include file
 * mconf.h.
 *
 * The default size parameters are as follows.
 *
 * For DEC and UNK modes:
 * MACHEP =  1.38777878078144567553E-17       2**-56
 * MAXLOG =  8.8029691931113054295988E1       log(2**127)
 * MINLOG = -8.872283911167299960540E1        log(2**-128)
 * MAXNUM =  1.701411834604692317316873e38    2**127
 *
 * For IEEE arithmetic (IBMPC):
 * MACHEP =  1.11022302462515654042E-16       2**-53
 * MAXLOG =  7.09782712893383996843E2         log(2**1024)
 * MINLOG = -7.08396418532264106224E2         log(2**-1022)
 * MAXNUM =  1.7976931348623158E308           2**1024
 *
 * The global symbols for mathematical constants are
 * PI     =  3.14159265358979323846           pi
 * PIO2   =  1.57079632679489661923           pi/2
 * PIO4   =  7.85398163397448309616E-1        pi/4
 * SQRT2  =  1.41421356237309504880           sqrt(2)
 * SQRTH  =  7.07106781186547524401E-1        sqrt(2)/2
 * LOG2E  =  1.4426950408889634073599         1/log(2)
 * SQ2OPI =  7.9788456080286535587989E-1      sqrt( 2/pi )
 * LOGE2  =  6.93147180559945309417E-1        log(2)
 * LOGSQ2 =  3.46573590279972654709E-1        log(2)/2
 * THPIO4 =  2.35619449019234492885           3*pi/4
 * TWOOPI =  6.36619772367581343075535E-1     2/pi
 *
 * These lists are subject to change.
 */

/*							const.c */

/*
Cephes Math Library Release 2.3:  March, 1995
Copyright 1984, 1995 by Stephen L. Moshier
*/

/* #include "mconf.h" */

#ifdef UNK
#if 1
double MACHEP =  1.11022302462515654042E-16;   /* 2**-53 */
#else
double MACHEP =  1.38777878078144567553E-17;   /* 2**-56 */
#endif
double UFLOWTHRESH =  2.22507385850720138309E-308; /* 2**-1022 */
#ifdef DENORMAL
double MAXLOG =  7.09782712893383996732E2;     /* log(MAXNUM) */
/* double MINLOG = -7.44440071921381262314E2; */     /* log(2**-1074) */
double MINLOG = -7.451332191019412076235E2;     /* log(2**-1075) */
#else
double MAXLOG =  7.08396418532264106224E2;     /* log 2**1022 */
double MINLOG = -7.08396418532264106224E2;     /* log 2**-1022 */
#endif
double MAXNUM =  1.79769313486231570815E308;    /* 2**1024*(1-MACHEP) */
double PI     =  3.14159265358979323846;       /* pi */
double PIO2   =  1.57079632679489661923;       /* pi/2 */
double PIO4   =  7.85398163397448309616E-1;    /* pi/4 */
double SQRT2  =  1.41421356237309504880;       /* sqrt(2) */
double SQRTH  =  7.07106781186547524401E-1;    /* sqrt(2)/2 */
double LOG2E  =  1.4426950408889634073599;     /* 1/log(2) */
double SQ2OPI =  7.9788456080286535587989E-1;  /* sqrt( 2/pi ) */
double LOGE2  =  6.93147180559945309417E-1;    /* log(2) */
double LOGSQ2 =  3.46573590279972654709E-1;    /* log(2)/2 */
double THPIO4 =  2.35619449019234492885;       /* 3*pi/4 */
double TWOOPI =  6.36619772367581343075535E-1; /* 2/pi */
#ifdef INFINITIES
double INFINITY = 1.0/0.0;  /* 99e999; */
#else
double INFINITY =  1.79769313486231570815E308;    /* 2**1024*(1-MACHEP) */
#endif
#ifdef NANS
double NAN = 1.0/0.0 - 1.0/0.0;
#else
double NAN = 0.0;
#endif
#ifdef MINUSZERO
double NEGZERO = -0.0;
#else
double NEGZERO = 0.0;
#endif
#endif

#ifdef IBMPC
			/* 2**-53 =  1.11022302462515654042E-16 */
unsigned short MACHEP[4] = {0x0000,0x0000,0x0000,0x3ca0};
unsigned short UFLOWTHRESH[4] = {0x0000,0x0000,0x0000,0x0010};
#ifdef DENORMAL
			/* log(MAXNUM) =  7.09782712893383996732224E2 */
unsigned short MAXLOG[4] = {0x39ef,0xfefa,0x2e42,0x4086};
			/* log(2**-1074) = - -7.44440071921381262314E2 */
/*unsigned short MINLOG[4] = {0x71c3,0x446d,0x4385,0xc087};*/
unsigned short MINLOG[4] = {0x3052,0xd52d,0x4910,0xc087};
#else
			/* log(2**1022) =   7.08396418532264106224E2 */
unsigned short MAXLOG[4] = {0xbcd2,0xdd7a,0x232b,0x4086};
			/* log(2**-1022) = - 7.08396418532264106224E2 */
unsigned short MINLOG[4] = {0xbcd2,0xdd7a,0x232b,0xc086};
#endif
			/* 2**1024*(1-MACHEP) =  1.7976931348623158E308 */
unsigned short MAXNUM[4] = {0xffff,0xffff,0xffff,0x7fef};
unsigned short PI[4]     = {0x2d18,0x5444,0x21fb,0x4009};
unsigned short PIO2[4]   = {0x2d18,0x5444,0x21fb,0x3ff9};
unsigned short PIO4[4]   = {0x2d18,0x5444,0x21fb,0x3fe9};
unsigned short SQRT2[4]  = {0x3bcd,0x667f,0xa09e,0x3ff6};
unsigned short SQRTH[4]  = {0x3bcd,0x667f,0xa09e,0x3fe6};
unsigned short LOG2E[4]  = {0x82fe,0x652b,0x1547,0x3ff7};
unsigned short SQ2OPI[4] = {0x3651,0x33d4,0x8845,0x3fe9};
unsigned short LOGE2[4]  = {0x39ef,0xfefa,0x2e42,0x3fe6};
unsigned short LOGSQ2[4] = {0x39ef,0xfefa,0x2e42,0x3fd6};
unsigned short THPIO4[4] = {0x21d2,0x7f33,0xd97c,0x4002};
unsigned short TWOOPI[4] = {0xc883,0x6dc9,0x5f30,0x3fe4};
#ifdef INFINITIES
unsigned short INFINITY[4] = {0x0000,0x0000,0x0000,0x7ff0};
#else
unsigned short INFINITY[4] = {0xffff,0xffff,0xffff,0x7fef};
#endif
#ifdef NANS
unsigned short NAN[4] = {0x0000,0x0000,0x0000,0x7ffc};
#else
unsigned short NAN[4] = {0x0000,0x0000,0x0000,0x0000};
#endif
#ifdef MINUSZERO
unsigned short NEGZERO[4] = {0x0000,0x0000,0x0000,0x8000};
#else
unsigned short NEGZERO[4] = {0x0000,0x0000,0x0000,0x0000};
#endif
#endif

#ifdef MIEEE
			/* 2**-53 =  1.11022302462515654042E-16 */
unsigned short MACHEP[4] = {0x3ca0,0x0000,0x0000,0x0000};
unsigned short UFLOWTHRESH[4] = {0x0010,0x0000,0x0000,0x0000};
#ifdef DENORMAL
			/* log(2**1024) =   7.09782712893383996843E2 */
unsigned short MAXLOG[4] = {0x4086,0x2e42,0xfefa,0x39ef};
			/* log(2**-1074) = - -7.44440071921381262314E2 */
/* unsigned short MINLOG[4] = {0xc087,0x4385,0x446d,0x71c3}; */
unsigned short MINLOG[4] = {0xc087,0x4910,0xd52d,0x3052};
#else
			/* log(2**1022) =  7.08396418532264106224E2 */
unsigned short MAXLOG[4] = {0x4086,0x232b,0xdd7a,0xbcd2};
			/* log(2**-1022) = - 7.08396418532264106224E2 */
unsigned short MINLOG[4] = {0xc086,0x232b,0xdd7a,0xbcd2};
#endif
			/* 2**1024*(1-MACHEP) =  1.7976931348623158E308 */
unsigned short MAXNUM[4] = {0x7fef,0xffff,0xffff,0xffff};
unsigned short PI[4]     = {0x4009,0x21fb,0x5444,0x2d18};
unsigned short PIO2[4]   = {0x3ff9,0x21fb,0x5444,0x2d18};
unsigned short PIO4[4]   = {0x3fe9,0x21fb,0x5444,0x2d18};
unsigned short SQRT2[4]  = {0x3ff6,0xa09e,0x667f,0x3bcd};
unsigned short SQRTH[4]  = {0x3fe6,0xa09e,0x667f,0x3bcd};
unsigned short LOG2E[4]  = {0x3ff7,0x1547,0x652b,0x82fe};
unsigned short SQ2OPI[4] = {0x3fe9,0x8845,0x33d4,0x3651};
unsigned short LOGE2[4]  = {0x3fe6,0x2e42,0xfefa,0x39ef};
unsigned short LOGSQ2[4] = {0x3fd6,0x2e42,0xfefa,0x39ef};
unsigned short THPIO4[4] = {0x4002,0xd97c,0x7f33,0x21d2};
unsigned short TWOOPI[4] = {0x3fe4,0x5f30,0x6dc9,0xc883};
#ifdef INFINITIES
unsigned short INFINITY[4] = {0x7ff0,0x0000,0x0000,0x0000};
#else
unsigned short INFINITY[4] = {0x7fef,0xffff,0xffff,0xffff};
#endif
#ifdef NANS
unsigned short NAN[4] = {0x7ff8,0x0000,0x0000,0x0000};
#else
unsigned short NAN[4] = {0x0000,0x0000,0x0000,0x0000};
#endif
#ifdef MINUSZERO
unsigned short NEGZERO[4] = {0x8000,0x0000,0x0000,0x0000};
#else
unsigned short NEGZERO[4] = {0x0000,0x0000,0x0000,0x0000};
#endif
#endif

#ifdef DEC
			/* 2**-56 =  1.38777878078144567553E-17 */
unsigned short MACHEP[4] = {0022200,0000000,0000000,0000000};
unsigned short UFLOWTHRESH[4] = {0x0080,0x0000,0x0000,0x0000};
			/* log 2**127 = 88.029691931113054295988 */
unsigned short MAXLOG[4] = {041660,007463,0143742,025733,};
			/* log 2**-128 = -88.72283911167299960540 */
unsigned short MINLOG[4] = {0141661,071027,0173721,0147572,};
			/* 2**127 = 1.701411834604692317316873e38 */
unsigned short MAXNUM[4] = {077777,0177777,0177777,0177777,};
unsigned short PI[4]     = {040511,007732,0121041,064302,};
unsigned short PIO2[4]   = {040311,007732,0121041,064302,};
unsigned short PIO4[4]   = {040111,007732,0121041,064302,};
unsigned short SQRT2[4]  = {040265,002363,031771,0157145,};
unsigned short SQRTH[4]  = {040065,002363,031771,0157144,};
unsigned short LOG2E[4]  = {040270,0125073,024534,013761,};
unsigned short SQ2OPI[4] = {040114,041051,0117241,0131204,};
unsigned short LOGE2[4]  = {040061,071027,0173721,0147572,};
unsigned short LOGSQ2[4] = {037661,071027,0173721,0147572,};
unsigned short THPIO4[4] = {040426,0145743,0174631,007222,};
unsigned short TWOOPI[4] = {040042,0174603,067116,042025,};
/* Approximate infinity by MAXNUM.  */
unsigned short INFINITY[4] = {077777,0177777,0177777,0177777,};
unsigned short NAN[4] = {0000000,0000000,0000000,0000000};
#ifdef MINUSZERO
unsigned short NEGZERO[4] = {0000000,0000000,0000000,0100000};
#else
unsigned short NEGZERO[4] = {0000000,0000000,0000000,0000000};
#endif
#endif

#ifndef UNK
extern unsigned short MACHEP[];
extern unsigned short UFLOWTHRESH[];
extern unsigned short MAXLOG[];
extern unsigned short UNDLOG[];
extern unsigned short MINLOG[];
extern unsigned short MAXNUM[];
extern unsigned short PI[];
extern unsigned short PIO2[];
extern unsigned short PIO4[];
extern unsigned short SQRT2[];
extern unsigned short SQRTH[];
extern unsigned short LOG2E[];
extern unsigned short SQ2OPI[];
extern unsigned short LOGE2[];
extern unsigned short LOGSQ2[];
extern unsigned short THPIO4[];
extern unsigned short TWOOPI[];
extern unsigned short INFINITY[];
extern unsigned short NAN[];
extern unsigned short NEGZERO[];
#endif


/*							mtherr.c
 *
 *	Library common error handling routine
 *
 *
 *
 * SYNOPSIS:
 *
 * char *fctnam;
 * int code;
 * int mtherr();
 *
 * mtherr( fctnam, code );
 *
 *
 *
 * DESCRIPTION:
 *
 * This routine may be called to report one of the following
 * error conditions (in the include file mconf.h).
 *  
 *   Mnemonic        Value          Significance
 *
 *    DOMAIN            1       argument domain error
 *    SING              2       function singularity
 *    OVERFLOW          3       overflow range error
 *    UNDERFLOW         4       underflow range error
 *    TLOSS             5       total loss of precision
 *    PLOSS             6       partial loss of precision
 *    EDOM             33       Unix domain error code
 *    ERANGE           34       Unix range error code
 *
 * The default version of the file prints the function name,
 * passed to it by the pointer fctnam, followed by the
 * error condition.  The display is directed to the standard
 * output device.  The routine then returns to the calling
 * program.  Users may wish to modify the program to abort by
 * calling exit() under severe error conditions such as domain
 * errors.
 *
 * Since all error conditions pass control to this function,
 * the display may be easily changed, eliminated, or directed
 * to an error logging device.
 *
 * SEE ALSO:
 *
 * mconf.h
 *
 */

/*
Cephes Math Library Release 2.0:  April, 1987
Copyright 1984, 1987 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

#include <stdio.h>
/* #include "mconf.h" */

/* int merror = 0; */

/* Notice: the order of appearance of the following
 * messages is bound to the error codes defined
 * in mconf.h.
 */
static char *ermsg[7] = {
"unknown",      /* error code 0 */
"domain",       /* error code 1 */
"singularity",  /* et seq.      */
"overflow",
"underflow",
"total loss of precision",
"partial loss of precision"
};


static int mtherr(char *name, int code )
{

  /* Display string passed by calling program,
   * which is supposed to be the name of the
   * function in which the error occurred:
   */
  printf( "\nError in cephes.c: %s ", name );
  
  /* Set global error message word */
  merror = code;
  
  /* Display error message defined
   * by the code argument.
   */
  if( (code <= 0) || (code >= 7) )
    code = 0;
  printf( "%s error\n", ermsg[code] );
  
  /* Return to calling
   * program
   */
  return( 0 );
}


/*							polevl.c
 *							p1evl.c
 *
 *	Evaluate polynomial
 *
 *
 *
 * SYNOPSIS:
 *
 * int N;
 * double x, y, coef[N+1], polevl[];
 *
 * y = polevl( x, coef, N );
 *
 *
 *
 * DESCRIPTION:
 *
 * Evaluates polynomial of degree N:
 *
 *                     2          N
 * y  =  C  + C x + C x  +...+ C x
 *        0    1     2          N
 *
 * Coefficients are stored in reverse order:
 *
 * coef[0] = C  , ..., coef[N] = C  .
 *            N                   0
 *
 *  The function p1evl() assumes that coef[N] = 1.0 and is
 * omitted from the array.  Its calling arguments are
 * otherwise the same as polevl().
 *
 *
 * SPEED:
 *
 * In the interest of speed, there are no checks for out
 * of bounds arithmetic.  This routine is used by most of
 * the functions in the library.  Depending on available
 * equipment features, the user may wish to rewrite the
 * program in microcode or assembly language.
 *
 */


/*
Cephes Math Library Release 2.1:  December, 1988
Copyright 1984, 1987, 1988 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


double polevl(double x, double coef[], int N)
{
  double ans;
  int i;
  double *p;
  
  p = coef;
  ans = *p++;
  i = N;

  do
    ans = ans * x  +  *p++;
  while( --i );
  
  return( ans );
}

/*							p1evl()	*/
/*                                          N
 * Evaluate polynomial when coefficient of x  is 1.0.
 * Otherwise same as polevl.
 */

double p1evl(double x, double coef[], int N )
{
  double ans;
  double *p;
  int i;

  p = coef;
  ans = x + *p++;
  i = N-1;
  
  do
    ans = ans * x  + *p++;
  while( --i );
  
  return( ans );
}

/*							gamma.c
 *
 *	Gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, gamma();
 * extern int sgngam;
 *
 * y = gamma( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns gamma function of the argument.  The result is
 * correctly signed, and the sign (+1 or -1) is also
 * returned in a global (extern) variable named sgngam.
 * This variable is also filled in by the logarithmic gamma
 * function lgam().
 *
 * Arguments |x| <= 34 are reduced by recurrence and the function
 * approximated by a rational function of degree 6/7 in the
 * interval (2,3).  Large arguments are handled by Stirling's
 * formula. Large negative arguments are made positive using
 * a reflection formula.  
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain     # trials      peak         rms
 *    DEC      -34, 34      10000       1.3e-16     2.5e-17
 *    IEEE    -170,-33      20000       2.3e-15     3.3e-16
 *    IEEE     -33,  33     20000       9.4e-16     2.2e-16
 *    IEEE      33, 171.6   20000       2.3e-15     3.2e-16
 *
 * Error for arguments outside the test range will be larger
 * owing to error amplification by the exponential function.
 *
 */
/*							lgam()
 *
 *	Natural logarithm of gamma function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, lgam();
 * extern int sgngam;
 *
 * y = lgam( x );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the base e (2.718...) logarithm of the absolute
 * value of the gamma function of the argument.
 * The sign (+1 or -1) of the gamma function is returned in a
 * global (extern) variable named sgngam.
 *
 * For arguments greater than 13, the logarithm of the gamma
 * function is approximated by the logarithmic version of
 * Stirling's formula using a polynomial approximation of
 * degree 4. Arguments between -33 and +33 are reduced by
 * recurrence to the interval [2,3] of a rational approximation.
 * The cosecant reflection formula is employed for arguments
 * less than -33.
 *
 * Arguments greater than MAXLGM return MAXNUM and an error
 * message.  MAXLGM = 2.035093e36 for DEC
 * arithmetic or 2.556348e305 for IEEE arithmetic.
 *
 *
 *
 * ACCURACY:
 *
 *
 * arithmetic      domain        # trials     peak         rms
 *    DEC     0, 3                  7000     5.2e-17     1.3e-17
 *    DEC     2.718, 2.035e36       5000     3.9e-17     9.9e-18
 *    IEEE    0, 3                 28000     5.4e-16     1.1e-16
 *    IEEE    2.718, 2.556e305     40000     3.5e-16     8.3e-17
 * The error criterion was relative when the function magnitude
 * was greater than one but absolute when it was less than one.
 *
 * The following test used the relative error criterion, though
 * at certain points the relative error could be much higher than
 * indicated.
 *    IEEE    -200, -4             10000     4.8e-16     1.3e-16
 *
 */

/*							gamma.c	*/
/*	gamma function	*/

/*
Cephes Math Library Release 2.2:  July, 1992
Copyright 1984, 1987, 1989, 1992 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/


/* #include "mconf.h" */

#ifdef UNK
static double P[] = {
  1.60119522476751861407E-4,
  1.19135147006586384913E-3,
  1.04213797561761569935E-2,
  4.76367800457137231464E-2,
  2.07448227648435975150E-1,
  4.94214826801497100753E-1,
  9.99999999999999996796E-1
};
static double Q[] = {
-2.31581873324120129819E-5,
 5.39605580493303397842E-4,
-4.45641913851797240494E-3,
 1.18139785222060435552E-2,
 3.58236398605498653373E-2,
-2.34591795718243348568E-1,
 7.14304917030273074085E-2,
 1.00000000000000000320E0
};
#define MAXGAM 171.624376956302725
static double LOGPI = 1.14472988584940017414;
#endif

#ifdef DEC
static unsigned short P[] = {
0035047,0162701,0146301,0005234,
0035634,0023437,0032065,0176530,
0036452,0137157,0047330,0122574,
0037103,0017310,0143041,0017232,
0037524,0066516,0162563,0164605,
0037775,0004671,0146237,0014222,
0040200,0000000,0000000,0000000
};
static unsigned short Q[] = {
0134302,0041724,0020006,0116565,
0035415,0072121,0044251,0025634,
0136222,0003447,0035205,0121114,
0036501,0107552,0154335,0104271,
0037022,0135717,0014776,0171471,
0137560,0034324,0165024,0037021,
0037222,0045046,0047151,0161213,
0040200,0000000,0000000,0000000
};
#define MAXGAM 34.84425627277176174
static unsigned short LPI[4] = {
0040222,0103202,0043475,0006750,
};
#define LOGPI *(double *)LPI
#endif

#ifdef IBMPC
static unsigned short P[] = {
0x2153,0x3998,0xfcb8,0x3f24,
0xbfab,0xe686,0x84e3,0x3f53,
0x14b0,0xe9db,0x57cd,0x3f85,
0x23d3,0x18c4,0x63d9,0x3fa8,
0x7d31,0xdcae,0x8da9,0x3fca,
0xe312,0x3993,0xa137,0x3fdf,
0x0000,0x0000,0x0000,0x3ff0
};
static unsigned short Q[] = {
0xd3af,0x8400,0x487a,0xbef8,
0x2573,0x2915,0xae8a,0x3f41,
0xb44a,0xe750,0x40e4,0xbf72,
0xb117,0x5b1b,0x31ed,0x3f88,
0xde67,0xe33f,0x5779,0x3fa2,
0x87c2,0x9d42,0x071a,0xbfce,
0x3c51,0xc9cd,0x4944,0x3fb2,
0x0000,0x0000,0x0000,0x3ff0
};
#define MAXGAM 171.624376956302725
static unsigned short LPI[4] = {
0xa1bd,0x48e7,0x50d0,0x3ff2,
};
#define LOGPI *(double *)LPI
#endif 

#ifdef MIEEE
static unsigned short P[] = {
0x3f24,0xfcb8,0x3998,0x2153,
0x3f53,0x84e3,0xe686,0xbfab,
0x3f85,0x57cd,0xe9db,0x14b0,
0x3fa8,0x63d9,0x18c4,0x23d3,
0x3fca,0x8da9,0xdcae,0x7d31,
0x3fdf,0xa137,0x3993,0xe312,
0x3ff0,0x0000,0x0000,0x0000
};
static unsigned short Q[] = {
0xbef8,0x487a,0x8400,0xd3af,
0x3f41,0xae8a,0x2915,0x2573,
0xbf72,0x40e4,0xe750,0xb44a,
0x3f88,0x31ed,0x5b1b,0xb117,
0x3fa2,0x5779,0xe33f,0xde67,
0xbfce,0x071a,0x9d42,0x87c2,
0x3fb2,0x4944,0xc9cd,0x3c51,
0x3ff0,0x0000,0x0000,0x0000
};
#define MAXGAM 171.624376956302725
static unsigned short LPI[4] = {
0x3ff2,0x50d0,0x48e7,0xa1bd,
};
#define LOGPI *(double *)LPI
#endif 

/* Stirling's formula for the gamma function */
#if UNK
static double STIR[5] = {
 7.87311395793093628397E-4,
-2.29549961613378126380E-4,
-2.68132617805781232825E-3,
 3.47222221605458667310E-3,
 8.33333333333482257126E-2,
};
#define MAXSTIR 143.01608
static double SQTPI = 2.50662827463100050242E0;
#endif
#if DEC
static unsigned short STIR[20] = {
0035516,0061622,0144553,0112224,
0135160,0131531,0037460,0165740,
0136057,0134460,0037242,0077270,
0036143,0107070,0156306,0027751,
0037252,0125252,0125252,0146064,
};
#define MAXSTIR 26.77
static unsigned short SQT[4] = {
0040440,0066230,0177661,0034055,
};
#define SQTPI *(double *)SQT
#endif
#if IBMPC
static unsigned short STIR[20] = {
0x7293,0x592d,0xcc72,0x3f49,
0x1d7c,0x27e6,0x166b,0xbf2e,
0x4fd7,0x07d4,0xf726,0xbf65,
0xc5fd,0x1b98,0x71c7,0x3f6c,
0x5986,0x5555,0x5555,0x3fb5,
};
#define MAXSTIR 143.01608
static unsigned short SQT[4] = {
0x2706,0x1ff6,0x0d93,0x4004,
};
#define SQTPI *(double *)SQT
#endif
#if MIEEE
static unsigned short STIR[20] = {
0x3f49,0xcc72,0x592d,0x7293,
0xbf2e,0x166b,0x27e6,0x1d7c,
0xbf65,0xf726,0x07d4,0x4fd7,
0x3f6c,0x71c7,0x1b98,0xc5fd,
0x3fb5,0x5555,0x5555,0x5986,
};
#define MAXSTIR 143.01608
static unsigned short SQT[4] = {
0x4004,0x0d93,0x1ff6,0x2706,
};
#define SQTPI *(double *)SQT
#endif

int sgngam = 0;
extern int sgngam;
extern double MAXLOG, MAXNUM, PI;
#ifndef ANSIPROT
double pow(), log(), exp(), sin(), polevl(), p1evl(), floor(), fabs();
int isnan(), isfinite();
#endif
#ifdef INFINITIES
extern double INFINITY;
#endif
#ifdef NANS
extern double NAN;
#endif

/* Gamma function computed by Stirling's formula.
 * The polynomial STIR is valid for 33 <= x <= 172.
 */
static double stirf(double x)
{
  double y, w, v;

  w = 1.0/x;
  w = 1.0 + w * polevl( w, STIR, 4 );
  y = exp(x);
  if( x > MAXSTIR )
    { /* Avoid overflow in pow() */
      v = pow( x, 0.5 * x - 0.25 );
      y = v * (v / y);
    }
  else
    {
      y = pow( x, x - 0.5 ) / y;
    }
  y = SQTPI * y * w;
  return( y );
}



double gamma(double x)
{
  double p, q, z;
  int i;

  sgngam = 1;
#ifdef NANS
  if( isnan(x) )
    return(x);
#endif
#ifdef INFINITIES
#ifdef NANS
  if( x == INFINITY )
    return(x);
  if( x == -INFINITY )
    return(NAN);
#else
  if( !isfinite(x) )
    return(x);
#endif
#endif
  q = fabs(x);

  if( q > 33.0 )
    {
      if( x < 0.0 )
	{
	  p = floor(q);
	  if( p == q )
	    {
#ifdef NANS
	    gamnan:
	      mtherr( "gamma", DOMAIN );
	      return (NAN);
#else
	      goto goverf;
#endif
	    }
	  i = p;
	  if( (i & 1) == 0 )
	    sgngam = -1;
	  z = q - p;
	  if( z > 0.5 )
	    {
	      p += 1.0;
	      z = q - p;
	    }
	  z = q * sin( PI * z );
	  if( z == 0.0 )
	    {
#ifdef INFINITIES
	      return( sgngam * INFINITY);
#else
	    goverf:
	      mtherr( "gamma", OVERFLOW );
	      return( sgngam * MAXNUM);
#endif
	    }
	  z = fabs(z);
	  z = PI/(z * stirf(q) );
	}
      else
	{
	  z = stirf(x);
	}
      return( sgngam * z );
    }
  
  z = 1.0;
  while( x >= 3.0 )
    {
      x -= 1.0;
      z *= x;
    }
  
  while( x < 0.0 )
    {
      if( x > -1.E-9 )
	goto small;
      z /= x;
      x += 1.0;
    }
  
  while( x < 2.0 )
    {
      if( x < 1.e-9 )
	goto small;
      z /= x;
      x += 1.0;
    }
  
  if( x == 2.0 )
    return(z);
  
  x -= 2.0;
  p = polevl( x, P, 6 );
  q = polevl( x, Q, 7 );
  return( z * p / q );
  
 small:
  if( x == 0.0 )
    {
#ifdef INFINITIES
#ifdef NANS
      goto gamnan;
#else
      return( INFINITY );
#endif
#else
      mtherr( "gamma", SING );
      return( MAXNUM );
#endif
    }
  else
    return( z/((1.0 + 0.5772156649015329 * x) * x) );
}



/* A[]: Stirling's formula expansion of log gamma
 * B[], C[]: log gamma function between 2 and 3
 */
#ifdef UNK
static double A[] = {
 8.11614167470508450300E-4,
-5.95061904284301438324E-4,
 7.93650340457716943945E-4,
-2.77777777730099687205E-3,
 8.33333333333331927722E-2
};
static double B[] = {
-1.37825152569120859100E3,
-3.88016315134637840924E4,
-3.31612992738871184744E5,
-1.16237097492762307383E6,
-1.72173700820839662146E6,
-8.53555664245765465627E5
};
static double C[] = {
/* 1.00000000000000000000E0, */
-3.51815701436523470549E2,
-1.70642106651881159223E4,
-2.20528590553854454839E5,
-1.13933444367982507207E6,
-2.53252307177582951285E6,
-2.01889141433532773231E6
};
/* log( sqrt( 2*pi ) ) */
static double LS2PI  =  0.91893853320467274178;
#define MAXLGM 2.556348e305
#endif

#ifdef DEC
static unsigned short A[] = {
0035524,0141201,0034633,0031405,
0135433,0176755,0126007,0045030,
0035520,0006371,0003342,0172730,
0136066,0005540,0132605,0026407,
0037252,0125252,0125252,0125132
};
static unsigned short B[] = {
0142654,0044014,0077633,0035410,
0144027,0110641,0125335,0144760,
0144641,0165637,0142204,0047447,
0145215,0162027,0146246,0155211,
0145322,0026110,0010317,0110130,
0145120,0061472,0120300,0025363
};
static unsigned short C[] = {
/*0040200,0000000,0000000,0000000*/
0142257,0164150,0163630,0112622,
0143605,0050153,0156116,0135272,
0144527,0056045,0145642,0062332,
0145213,0012063,0106250,0001025,
0145432,0111254,0044577,0115142,
0145366,0071133,0050217,0005122
};
/* log( sqrt( 2*pi ) ) */
static unsigned short LS2P[] = {040153,037616,041445,0172645,};
#define LS2PI *(double *)LS2P
#define MAXLGM 2.035093e36
#endif

#ifdef IBMPC
static unsigned short A[] = {
0x6661,0x2733,0x9850,0x3f4a,
0xe943,0xb580,0x7fbd,0xbf43,
0x5ebb,0x20dc,0x019f,0x3f4a,
0xa5a1,0x16b0,0xc16c,0xbf66,
0x554b,0x5555,0x5555,0x3fb5
};
static unsigned short B[] = {
0x6761,0x8ff3,0x8901,0xc095,
0xb93e,0x355b,0xf234,0xc0e2,
0x89e5,0xf890,0x3d73,0xc114,
0xdb51,0xf994,0xbc82,0xc131,
0xf20b,0x0219,0x4589,0xc13a,
0x055e,0x5418,0x0c67,0xc12a
};
static unsigned short C[] = {
/*0x0000,0x0000,0x0000,0x3ff0,*/
0x12b2,0x1cf3,0xfd0d,0xc075,
0xd757,0x7b89,0xaa0d,0xc0d0,
0x4c9b,0xb974,0xeb84,0xc10a,
0x0043,0x7195,0x6286,0xc131,
0xf34c,0x892f,0x5255,0xc143,
0xe14a,0x6a11,0xce4b,0xc13e
};
/* log( sqrt( 2*pi ) ) */
static unsigned short LS2P[] = {
0xbeb5,0xc864,0x67f1,0x3fed
};
#define LS2PI *(double *)LS2P
#define MAXLGM 2.556348e305
#endif

#ifdef MIEEE
static unsigned short A[] = {
0x3f4a,0x9850,0x2733,0x6661,
0xbf43,0x7fbd,0xb580,0xe943,
0x3f4a,0x019f,0x20dc,0x5ebb,
0xbf66,0xc16c,0x16b0,0xa5a1,
0x3fb5,0x5555,0x5555,0x554b
};
static unsigned short B[] = {
0xc095,0x8901,0x8ff3,0x6761,
0xc0e2,0xf234,0x355b,0xb93e,
0xc114,0x3d73,0xf890,0x89e5,
0xc131,0xbc82,0xf994,0xdb51,
0xc13a,0x4589,0x0219,0xf20b,
0xc12a,0x0c67,0x5418,0x055e
};
static unsigned short C[] = {
0xc075,0xfd0d,0x1cf3,0x12b2,
0xc0d0,0xaa0d,0x7b89,0xd757,
0xc10a,0xeb84,0xb974,0x4c9b,
0xc131,0x6286,0x7195,0x0043,
0xc143,0x5255,0x892f,0xf34c,
0xc13e,0xce4b,0x6a11,0xe14a
};
/* log( sqrt( 2*pi ) ) */
static unsigned short LS2P[] = {
0x3fed,0x67f1,0xc864,0xbeb5
};
#define LS2PI *(double *)LS2P
#define MAXLGM 2.556348e305
#endif


/* Logarithm of gamma function */


double lgam(double x)
{
  double p, q, u, w, z;
  int i;

  sgngam = 1;
#ifdef NANS
  if( isnan(x) )
    return(x);
#endif
  
#ifdef INFINITIES
  if( !isfinite(x) )
    return(INFINITY);
#endif

  if( x < -34.0 )
    {
      q = -x;
      w = lgam(q); /* note this modifies sgngam! */
      p = floor(q);
      if( p == q )
	{
	lgsing:
#ifdef INFINITIES
	  mtherr( "lgam", SING );
	  return (INFINITY);
#else
	  goto loverf;
#endif
	}
      i = p;
      if( (i & 1) == 0 )
	sgngam = -1;
      else
	sgngam = 1;
      z = q - p;
      if( z > 0.5 )
	{
	  p += 1.0;
	  z = p - q;
	}
      z = q * sin( PI * z );
      if( z == 0.0 )
	goto lgsing;
      /*	z = log(PI) - log( z ) - w;*/
      z = LOGPI - log( z ) - w;
      return( z );
    }

  if( x < 13.0 )
    {
      z = 1.0;
      p = 0.0;
      u = x;
      while( u >= 3.0 )
	{
	  p -= 1.0;
	  u = x + p;
	  z *= u;
	}
      while( u < 2.0 )
	{
	  if( u == 0.0 )
	    goto lgsing;
	  z /= u;
	  p += 1.0;
	  u = x + p;
	}
      if( z < 0.0 )
	{
	  sgngam = -1;
	  z = -z;
	}
      else
	sgngam = 1;
      if( u == 2.0 )
	return( log(z) );
      p -= 2.0;
      x = x + p;
      p = x * polevl( x, B, 5 ) / p1evl( x, C, 6);
      return( log(z) + p );
    }
  
  if( x > MAXLGM )
    {
#ifdef INFINITIES
      return( sgngam * INFINITY );
#else
    loverf:
      mtherr( "lgam", OVERFLOW );
      return( sgngam * MAXNUM );
#endif
    }
  
  q = ( x - 0.5 ) * log(x) - x + LS2PI;
  if( x > 1.0e8 )
    return( q );
  
  p = 1.0/(x*x);
  if( x >= 1000.0 )
    q += ((   7.9365079365079365079365e-4 * p
	      - 2.7777777777777777777778e-3) *p
	  + 0.0833333333333333333333) / x;
  else
    q += polevl( p, A, 4 ) / x;
  return( q );
}

/*							ndtri.c
 *
 *	Inverse of Normal distribution function
 *
 *
 *
 * SYNOPSIS:
 *
 * double x, y, ndtri();
 *
 * x = ndtri( y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Returns the argument, x, for which the area under the
 * Gaussian probability density function (integrated from
 * minus infinity to x) is equal to y.
 *
 *
 * For small arguments 0 < y < exp(-2), the program computes
 * z = sqrt( -2.0 * log(y) );  then the approximation is
 * x = z - log(z)/z  - (1/z) P(1/z) / Q(1/z).
 * There are two rational functions P/Q, one for 0 < y < exp(-32)
 * and the other for y up to exp(-2).  For larger arguments,
 * w = y - 0.5, and  x/sqrt(2pi) = w + w**3 R(w**2)/S(w**2)).
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 * arithmetic   domain        # trials      peak         rms
 *    DEC      0.125, 1         5500       9.5e-17     2.1e-17
 *    DEC      6e-39, 0.135     3500       5.7e-17     1.3e-17
 *    IEEE     0.125, 1        20000       7.2e-16     1.3e-16
 *    IEEE     3e-308, 0.135   50000       4.6e-16     9.8e-17
 *
 *
 * ERROR MESSAGES:
 *
 *   message         condition    value returned
 * ndtri domain       x <= 0        -MAXNUM
 * ndtri domain       x >= 1         MAXNUM
 *
 */


/*
Cephes Math Library Release 2.1:  January, 1989
Copyright 1984, 1987, 1989 by Stephen L. Moshier
Direct inquiries to 30 Frost Street, Cambridge, MA 02140
*/

/* #include "mconf.h" */
extern double MAXNUM;

#ifdef UNK
/* sqrt(2pi) */
static double s2pi = 2.50662827463100050242E0;
#endif

#ifdef DEC
static unsigned short s2p[] = {0040440,0066230,0177661,0034055};
#define s2pi *(double *)s2p
#endif

#ifdef IBMPC
static unsigned short s2p[] = {0x2706,0x1ff6,0x0d93,0x4004};
#define s2pi *(double *)s2p
#endif

#ifdef MIEEE
static unsigned short s2p[] = {
0x4004,0x0d93,0x1ff6,0x2706
};
#define s2pi *(double *)s2p
#endif

/* approximation for 0 <= |y - 0.5| <= 3/8 */
#ifdef UNK
static double P0[5] = {
-5.99633501014107895267E1,
 9.80010754185999661536E1,
-5.66762857469070293439E1,
 1.39312609387279679503E1,
-1.23916583867381258016E0,
};
static double Q0[8] = {
/* 1.00000000000000000000E0,*/
 1.95448858338141759834E0,
 4.67627912898881538453E0,
 8.63602421390890590575E1,
-2.25462687854119370527E2,
 2.00260212380060660359E2,
-8.20372256168333339912E1,
 1.59056225126211695515E1,
-1.18331621121330003142E0,
};
#endif
#ifdef DEC
static unsigned short P0[20] = {
0141557,0155170,0071360,0120550,
0041704,0000214,0172417,0067307,
0141542,0132204,0040066,0156723,
0041136,0163161,0157276,0007747,
0140236,0116374,0073666,0051764,
};
static unsigned short Q0[32] = {
/*0040200,0000000,0000000,0000000,*/
0040372,0026256,0110403,0123707,
0040625,0122024,0020277,0026661,
0041654,0134161,0124134,0007244,
0142141,0073162,0133021,0131371,
0042110,0041235,0043516,0057767,
0141644,0011417,0036155,0137305,
0041176,0076556,0004043,0125430,
0140227,0073347,0152776,0067251,
};
#endif
#ifdef IBMPC
static unsigned short P0[20] = {
0x142d,0x0e5e,0xfb4f,0xc04d,
0xedd9,0x9ea1,0x8011,0x4058,
0xdbba,0x8806,0x5690,0xc04c,
0xc1fd,0x3bd7,0xdcce,0x402b,
0xca7e,0x8ef6,0xd39f,0xbff3,
};
static unsigned short Q0[36] = {
/*0x0000,0x0000,0x0000,0x3ff0,*/
0x74f9,0xd220,0x4595,0x3fff,
0xe5b6,0x8417,0xb482,0x4012,
0x81d4,0x350b,0x970e,0x4055,
0x365f,0x56c2,0x2ece,0xc06c,
0xcbff,0xa8e9,0x0853,0x4069,
0xb7d9,0xe78d,0x8261,0xc054,
0x7563,0xc104,0xcfad,0x402f,
0xcdd5,0xfabf,0xeedc,0xbff2,
};
#endif
#ifdef MIEEE
static unsigned short P0[20] = {
0xc04d,0xfb4f,0x0e5e,0x142d,
0x4058,0x8011,0x9ea1,0xedd9,
0xc04c,0x5690,0x8806,0xdbba,
0x402b,0xdcce,0x3bd7,0xc1fd,
0xbff3,0xd39f,0x8ef6,0xca7e,
};
static unsigned short Q0[32] = {
/*0x3ff0,0x0000,0x0000,0x0000,*/
0x3fff,0x4595,0xd220,0x74f9,
0x4012,0xb482,0x8417,0xe5b6,
0x4055,0x970e,0x350b,0x81d4,
0xc06c,0x2ece,0x56c2,0x365f,
0x4069,0x0853,0xa8e9,0xcbff,
0xc054,0x8261,0xe78d,0xb7d9,
0x402f,0xcfad,0xc104,0x7563,
0xbff2,0xeedc,0xfabf,0xcdd5,
};
#endif


/* Approximation for interval z = sqrt(-2 log y ) between 2 and 8
 * i.e., y between exp(-2) = .135 and exp(-32) = 1.27e-14.
 */
#ifdef UNK
static double P1[9] = {
 4.05544892305962419923E0,
 3.15251094599893866154E1,
 5.71628192246421288162E1,
 4.40805073893200834700E1,
 1.46849561928858024014E1,
 2.18663306850790267539E0,
-1.40256079171354495875E-1,
-3.50424626827848203418E-2,
-8.57456785154685413611E-4,
};
static double Q1[8] = {
/*  1.00000000000000000000E0,*/
 1.57799883256466749731E1,
 4.53907635128879210584E1,
 4.13172038254672030440E1,
 1.50425385692907503408E1,
 2.50464946208309415979E0,
-1.42182922854787788574E-1,
-3.80806407691578277194E-2,
-9.33259480895457427372E-4,
};
#endif
#ifdef DEC
static unsigned short P1[36] = {
0040601,0143074,0150744,0073326,
0041374,0031554,0113253,0146016,
0041544,0123272,0012463,0176771,
0041460,0051160,0103560,0156511,
0041152,0172624,0117772,0030755,
0040413,0170713,0151545,0176413,
0137417,0117512,0022154,0131671,
0137017,0104257,0071432,0007072,
0135540,0143363,0063137,0036166,
};
static unsigned short Q1[32] = {
/*0040200,0000000,0000000,0000000,*/
0041174,0075325,0004736,0120326,
0041465,0110044,0047561,0045567,
0041445,0042321,0012142,0030340,
0041160,0127074,0166076,0141051,
0040440,0046055,0040745,0150400,
0137421,0114146,0067330,0010621,
0137033,0175162,0025555,0114351,
0135564,0122773,0145750,0030357,
};
#endif
#ifdef IBMPC
static unsigned short P1[36] = {
0x8edb,0x9a3c,0x38c7,0x4010,
0x7982,0x92d5,0x866d,0x403f,
0x7fbf,0x42a6,0x94d7,0x404c,
0x1ba9,0x10ee,0x0a4e,0x4046,
0x463e,0x93ff,0x5eb2,0x402d,
0xbfa1,0x7a6c,0x7e39,0x4001,
0x9677,0x448d,0xf3e9,0xbfc1,
0x41c7,0xee63,0xf115,0xbfa1,
0xe78f,0x6ccb,0x18de,0xbf4c,
};
static unsigned short Q1[32] = {
/*0x0000,0x0000,0x0000,0x3ff0,*/
0xd41b,0xa13b,0x8f5a,0x402f,
0x296f,0x89ee,0xb204,0x4046,
0x461c,0x228c,0xa89a,0x4044,
0xd845,0x9d87,0x15c7,0x402e,
0xba20,0xa83c,0x0985,0x4004,
0x0232,0xcddb,0x330c,0xbfc2,
0xb31d,0x456d,0x7f4e,0xbfa3,
0x061e,0x797d,0x94bf,0xbf4e,
};
#endif
#ifdef MIEEE
static unsigned short P1[36] = {
0x4010,0x38c7,0x9a3c,0x8edb,
0x403f,0x866d,0x92d5,0x7982,
0x404c,0x94d7,0x42a6,0x7fbf,
0x4046,0x0a4e,0x10ee,0x1ba9,
0x402d,0x5eb2,0x93ff,0x463e,
0x4001,0x7e39,0x7a6c,0xbfa1,
0xbfc1,0xf3e9,0x448d,0x9677,
0xbfa1,0xf115,0xee63,0x41c7,
0xbf4c,0x18de,0x6ccb,0xe78f,
};
static unsigned short Q1[32] = {
/*0x3ff0,0x0000,0x0000,0x0000,*/
0x402f,0x8f5a,0xa13b,0xd41b,
0x4046,0xb204,0x89ee,0x296f,
0x4044,0xa89a,0x228c,0x461c,
0x402e,0x15c7,0x9d87,0xd845,
0x4004,0x0985,0xa83c,0xba20,
0xbfc2,0x330c,0xcddb,0x0232,
0xbfa3,0x7f4e,0x456d,0xb31d,
0xbf4e,0x94bf,0x797d,0x061e,
};
#endif

/* Approximation for interval z = sqrt(-2 log y ) between 8 and 64
 * i.e., y between exp(-32) = 1.27e-14 and exp(-2048) = 3.67e-890.
 */

#ifdef UNK
static double P2[9] = {
  3.23774891776946035970E0,
  6.91522889068984211695E0,
  3.93881025292474443415E0,
  1.33303460815807542389E0,
  2.01485389549179081538E-1,
  1.23716634817820021358E-2,
  3.01581553508235416007E-4,
  2.65806974686737550832E-6,
  6.23974539184983293730E-9,
};
static double Q2[8] = {
/*  1.00000000000000000000E0,*/
  6.02427039364742014255E0,
  3.67983563856160859403E0,
  1.37702099489081330271E0,
  2.16236993594496635890E-1,
  1.34204006088543189037E-2,
  3.28014464682127739104E-4,
  2.89247864745380683936E-6,
  6.79019408009981274425E-9,
};
#endif
#ifdef DEC
static unsigned short P2[36] = {
0040517,0033507,0036236,0125641,
0040735,0044616,0014473,0140133,
0040574,0012567,0114535,0102541,
0040252,0120340,0143474,0150135,
0037516,0051057,0115361,0031211,
0036512,0131204,0101511,0125144,
0035236,0016627,0043160,0140216,
0033462,0060512,0060141,0010641,
0031326,0062541,0101304,0077706,
};
static unsigned short Q2[32] = {
/*0040200,0000000,0000000,0000000,*/
0040700,0143322,0132137,0040501,
0040553,0101155,0053221,0140257,
0040260,0041071,0052573,0010004,
0037535,0066472,0177261,0162330,
0036533,0160475,0066666,0036132,
0035253,0174533,0027771,0044027,
0033502,0016147,0117666,0063671,
0031351,0047455,0141663,0054751,
};
#endif
#ifdef IBMPC
static unsigned short P2[36] = {
0xd574,0xe793,0xe6e8,0x4009,
0x780b,0xc327,0xa931,0x401b,
0xb0ac,0xf32b,0x82ae,0x400f,
0x9a0c,0x18e7,0x541c,0x3ff5,
0x2651,0xf35e,0xca45,0x3fc9,
0x354d,0x9069,0x5650,0x3f89,
0x1812,0xe8ce,0xc3b2,0x3f33,
0x2234,0x4c0c,0x4c29,0x3ec6,
0x8ff9,0x3058,0xccac,0x3e3a,
};
static unsigned short Q2[32] = {
/*0x0000,0x0000,0x0000,0x3ff0,*/
0xe828,0x568b,0x18da,0x4018,
0x3816,0xaad2,0x704d,0x400d,
0x6200,0x2aaf,0x0847,0x3ff6,
0x3c9b,0x5fd6,0xada7,0x3fcb,
0xc78b,0xadb6,0x7c27,0x3f8b,
0x2903,0x65ff,0x7f2b,0x3f35,
0xccf7,0xf3f6,0x438c,0x3ec8,
0x6b3d,0xb876,0x29e5,0x3e3d,
};
#endif
#ifdef MIEEE
static unsigned short P2[36] = {
0x4009,0xe6e8,0xe793,0xd574,
0x401b,0xa931,0xc327,0x780b,
0x400f,0x82ae,0xf32b,0xb0ac,
0x3ff5,0x541c,0x18e7,0x9a0c,
0x3fc9,0xca45,0xf35e,0x2651,
0x3f89,0x5650,0x9069,0x354d,
0x3f33,0xc3b2,0xe8ce,0x1812,
0x3ec6,0x4c29,0x4c0c,0x2234,
0x3e3a,0xccac,0x3058,0x8ff9,
};
static unsigned short Q2[32] = {
/*0x3ff0,0x0000,0x0000,0x0000,*/
0x4018,0x18da,0x568b,0xe828,
0x400d,0x704d,0xaad2,0x3816,
0x3ff6,0x0847,0x2aaf,0x6200,
0x3fcb,0xada7,0x5fd6,0x3c9b,
0x3f8b,0x7c27,0xadb6,0xc78b,
0x3f35,0x7f2b,0x65ff,0x2903,
0x3ec8,0x438c,0xf3f6,0xccf7,
0x3e3d,0x29e5,0xb876,0x6b3d,
};
#endif

#ifndef ANSIPROT
double polevl(), p1evl(), log(), sqrt();
#endif

double ndtri(double y0)
{
  double x, y, z, y2, x0, x1;
  int code;

  if( y0 <= 0.0 )
    {
      mtherr( "ndtri", DOMAIN );
      return( -MAXNUM );
    }
  if( y0 >= 1.0 )
    {
      mtherr( "ndtri", DOMAIN );
      return( MAXNUM );
    }
  code = 1;
  y = y0;
  if( y > (1.0 - 0.13533528323661269189) ) /* 0.135... = exp(-2) */
    {
      y = 1.0 - y;
      code = 0;
    }
  
  if( y > 0.13533528323661269189 )
    {
      y = y - 0.5;
      y2 = y * y;
      x = y + y * (y2 * polevl( y2, P0, 4)/p1evl( y2, Q0, 8 ));
      x = x * s2pi; 
      return(x);
    }
  
  x = sqrt( -2.0 * log(y) );
  x0 = x - log(x)/x;

  z = 1.0/x;
  if( x < 8.0 ) /* y > exp(-32) = 1.2664165549e-14 */
    x1 = z * polevl( z, P1, 8 )/p1evl( z, Q1, 8 );
  else
    x1 = z * polevl( z, P2, 8 )/p1evl( z, Q2, 8 );
  x = x0 - x1;
  if( code != 0 )
    x = -x;
  return( x );
}

/*							incbet.c
 *
 *	Incomplete beta integral
 *
 *
 * SYNOPSIS:
 *
 * double a, b, x, y, incbet();
 *
 * y = incbet( a, b, x );
 *
 *
 * DESCRIPTION:
 *
 * Returns incomplete beta integral of the arguments, evaluated
 * from zero to x.  The function is defined as
 *
 *                  x
 *     -            -
 *    | (a+b)      | |  a-1     b-1
 *  -----------    |   t   (1-t)   dt.
 *   -     -     | |
 *  | (a) | (b)   -
 *                 0
 *
 * The domain of definition is 0 <= x <= 1.  In this
 * implementation a and b are restricted to positive values.
 * The integral from x to 1 may be obtained by the symmetry
 * relation
 *
 *    1 - incbet( a, b, x )  =  incbet( b, a, 1-x ).
 *
 * The integral is evaluated by a continued fraction expansion
 * or, when b*x is small, by a power series.
 *
 * ACCURACY:
 *
 * Tested at uniformly distributed random points (a,b,x) with a and b
 * in "domain" and x between 0 and 1.
 *                                        Relative error
 * arithmetic   domain     # trials      peak         rms
 *    IEEE      0,5         10000       6.9e-15     4.5e-16
 *    IEEE      0,85       250000       2.2e-13     1.7e-14
 *    IEEE      0,1000      30000       5.3e-12     6.3e-13
 *    IEEE      0,10000    250000       9.3e-11     7.1e-12
 *    IEEE      0,100000    10000       8.7e-10     4.8e-11
 * Outputs smaller than the IEEE gradual underflow threshold
 * were excluded from these statistics.
 *
 * ERROR MESSAGES:
 *   message         condition      value returned
 * incbet domain      x<0, x>1          0.0
 * incbet underflow                     0.0
 */


/*
Cephes Math Library, Release 2.3:  March, 1995
Copyright 1984, 1995 by Stephen L. Moshier
*/

/* #include "mconf.h" */

#ifdef DEC
#define MAXGAM 34.84425627277176174
#else
#define MAXGAM 171.624376956302725
#endif

extern double MACHEP, MINLOG, MAXLOG;
#ifdef ANSIPROT
static double incbcf(double, double, double);
static double incbd(double, double, double);
static double pseries(double, double, double);
#else
double gamma(), lgam(), exp(), log(), pow(), fabs();
static double incbcf(), incbd(), pseries();
#endif

static double big = 4.503599627370496e15;
static double biginv =  2.22044604925031308085e-16;


double incbet(double aa, double bb, double xx )
{
  double a, b, t, x, xc, w, y;
  int flag;

  if( aa <= 0.0 || bb <= 0.0 )
    goto domerr;
  
  if( (xx <= 0.0) || ( xx >= 1.0) )
    {
      if( xx == 0.0 )
	return(0.0);
      if( xx == 1.0 )
	return( 1.0 );
    domerr:
      mtherr( "incbet", DOMAIN );
      return( 0.0 );
    }
  
  flag = 0;
  if( (bb * xx) <= 1.0 && xx <= 0.95)
    {
      t = pseries(aa, bb, xx);
      goto done;
    }
  
  w = 1.0 - xx;
  
  /* Reverse a and b if x is greater than the mean. */
  if( xx > (aa/(aa+bb)) )
    {
      flag = 1;
      a = bb;
      b = aa;
      xc = xx;
      x = w;
    }
  else
    {
      a = aa;
      b = bb;
      xc = w;
      x = xx;
    }
  
  if( flag == 1 && (b * x) <= 1.0 && x <= 0.95)
    {
      t = pseries(a, b, x);
      goto done;
    }
  
  /* Choose expansion for better convergence. */
  y = x * (a+b-2.0) - (a-1.0);
  if( y < 0.0 )
    w = incbcf( a, b, x );
  else
    w = incbd( a, b, x ) / xc;
  
  /* Multiply w by the factor
      a      b   _             _     _
     x  (1-x)   | (a+b) / ( a | (a) | (b) ) .   */

  y = a * log(x);
  t = b * log(xc);
  if( (a+b) < MAXGAM && fabs(y) < MAXLOG && fabs(t) < MAXLOG )
    {
      t = pow(xc,b);
      t *= pow(x,a);
      t /= a;
      t *= w;
      t *= gamma(a+b) / (gamma(a) * gamma(b));
      goto done;
    }
  /* Resort to logarithms.  */
  y += t + lgam(a+b) - lgam(a) - lgam(b);
  y += log(w/a);
  if( y < MINLOG )
    t = 0.0;
  else
    t = exp(y);
  
 done:
  
  if( flag == 1 )
    {
      if( t <= MACHEP )
	t = 1.0 - MACHEP;
      else
	t = 1.0 - t;
    }
  return( t );
}

/* Continued fraction expansion #1
 * for incomplete beta integral
 */

static double incbcf(double a, double b, double x )
{
  double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
  double k1, k2, k3, k4, k5, k6, k7, k8;
  double r, t, ans, thresh;
  int n;
  
  k1 = a;
  k2 = a + b;
  k3 = a;
  k4 = a + 1.0;
  k5 = 1.0;
  k6 = b - 1.0;
  k7 = k4;
  k8 = a + 2.0;

  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0 * MACHEP;
  do
    {
      
      xk = -( x * k1 * k2 )/( k3 * k4 );
      pk = pkm1 +  pkm2 * xk;
      qk = qkm1 +  qkm2 * xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      xk = ( x * k5 * k6 )/( k7 * k8 );
      pk = pkm1 +  pkm2 * xk;
      qk = qkm1 +  qkm2 * xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      if( qk != 0 )
	r = pk/qk;
      if( r != 0 )
	{
	  t = fabs( (ans - r)/r );
	  ans = r;
	}
      else
	t = 1.0;
      
      if( t < thresh )
	goto cdone;
      
      k1 += 1.0;
      k2 += 1.0;
      k3 += 2.0;
      k4 += 2.0;
      k5 += 1.0;
      k6 -= 1.0;
      k7 += 2.0;
      k8 += 2.0;
      
      if( (fabs(qk) + fabs(pk)) > big )
	{
	  pkm2 *= biginv;
	  pkm1 *= biginv;
	  qkm2 *= biginv;
	  qkm1 *= biginv;
	}
      if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
	{
	  pkm2 *= big;
	  pkm1 *= big;
	  qkm2 *= big;
	  qkm1 *= big;
	}
    }
  while( ++n < 300 );
  
 cdone:
  return(ans);
}


/* Continued fraction expansion #2
 * for incomplete beta integral
 */

static double incbd(double a, double b, double x )
{
  double xk, pk, pkm1, pkm2, qk, qkm1, qkm2;
  double k1, k2, k3, k4, k5, k6, k7, k8;
  double r, t, ans, z, thresh;
  int n;
  
  k1 = a;
  k2 = b - 1.0;
  k3 = a;
  k4 = a + 1.0;
  k5 = 1.0;
  k6 = a + b;
  k7 = a + 1.0;;
  k8 = a + 2.0;
  
  pkm2 = 0.0;
  qkm2 = 1.0;
  pkm1 = 1.0;
  qkm1 = 1.0;
  z = x / (1.0-x);
  ans = 1.0;
  r = 1.0;
  n = 0;
  thresh = 3.0 * MACHEP;
  do
    {
      
      xk = -( z * k1 * k2 )/( k3 * k4 );
      pk = pkm1 +  pkm2 * xk;
      qk = qkm1 +  qkm2 * xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      xk = ( z * k5 * k6 )/( k7 * k8 );
      pk = pkm1 +  pkm2 * xk;
      qk = qkm1 +  qkm2 * xk;
      pkm2 = pkm1;
      pkm1 = pk;
      qkm2 = qkm1;
      qkm1 = qk;
      
      if( qk != 0 )
	r = pk/qk;
      if( r != 0 )
	{
	  t = fabs( (ans - r)/r );
	  ans = r;
	}
      else
	t = 1.0;
      
      if( t < thresh )
	goto cdone;
      
      k1 += 1.0;
      k2 -= 1.0;
      k3 += 2.0;
      k4 += 2.0;
      k5 += 1.0;
      k6 += 1.0;
      k7 += 2.0;
      k8 += 2.0;
      
      if( (fabs(qk) + fabs(pk)) > big )
	{
	  pkm2 *= biginv;
	  pkm1 *= biginv;
	  qkm2 *= biginv;
	  qkm1 *= biginv;
	}
      if( (fabs(qk) < biginv) || (fabs(pk) < biginv) )
	{
	  pkm2 *= big;
	  pkm1 *= big;
	  qkm2 *= big;
	  qkm1 *= big;
	}
    }
  while( ++n < 300 );
 cdone:
  return(ans);
}

/* Power series for incomplete beta integral.
   Use when b*x is small and x not too close to 1.  */

static double pseries(double a, double b, double x)
{
  double s, t, u, v, n, t1, z, ai;
  
  ai = 1.0 / a;
  u = (1.0 - b) * x;
  v = u / (a + 1.0);
  t1 = v;
  t = u;
  n = 2.0;
  s = 0.0;
  z = MACHEP * ai;
  while( fabs(v) > z )
    {
      u = (n - b) * x / n;
      t *= u;
      v = t / (a + n);
      s += v; 
      n += 1.0;
    }
  s += t1;
  s += ai;
  
  u = a * log(x);
  if( (a+b) < MAXGAM && fabs(u) < MAXLOG )
    {
      t = gamma(a+b)/(gamma(a)*gamma(b));
      s = s * t * pow(x,a);
    }
  else
    {
      t = lgam(a+b) - lgam(a) - lgam(b) + u + log(s);
      if( t < MINLOG )
	s = 0.0;
      else
	s = exp(t);
    }
  return(s);
}

/*							incbi()
 *
 *      Inverse of incomplete beta integral
 *
 *
 *
 * SYNOPSIS:
 *
 * double a, b, x, y, incbi();
 *
 * x = incbi( a, b, y );
 *
 *
 *
 * DESCRIPTION:
 *
 * Given y, the function finds x such that
 *
 *  incbet( a, b, x ) = y .
 *
 * The routine performs interval halving or Newton iterations to find the
 * root of incbet(a,b,x) - y = 0.
 *
 *
 * ACCURACY:
 *
 *                      Relative error:
 *                x     a,b
 * arithmetic   domain  domain  # trials    peak       rms
 *    IEEE      0,1    .5,10000   50000    5.8e-12   1.3e-13
 *    IEEE      0,1   .25,100    100000    1.8e-13   3.9e-15
 *    IEEE      0,1     0,5       50000    1.1e-12   5.5e-15
 *    VAX       0,1    .5,100     25000    3.5e-14   1.1e-15
 * With a and b constrained to half-integer or integer values:
 *    IEEE      0,1    .5,10000   50000    5.8e-12   1.1e-13
 *    IEEE      0,1    .5,100    100000    1.7e-14   7.9e-16
 * With a = .5, b constrained to half-integer or integer values:
 *    IEEE      0,1    .5,10000   10000    8.3e-11   1.0e-11
 */


/*
Cephes Math Library Release 2.4:  March,1996
Copyright 1984, 1996 by Stephen L. Moshier
*/

/* #include "mconf.h" */

extern double MACHEP, MAXNUM, MAXLOG, MINLOG;
#ifndef ANSIPROT
double ndtri(), exp(), fabs(), log(), sqrt(), lgam(), incbet();
#endif

double incbi(double aa, double bb, double yy0)
{
  double a, b, y0, d, y, x, x0, x1, lgm, yp, di, dithresh, yl, yh, xt;
  int i, rflg, dir, nflg;

  
  i = 0;
  if( yy0 <= 0 )
    return(0.0);
  if( yy0 >= 1.0 )
    return(1.0);
  x0 = 0.0;
  yl = 0.0;
  x1 = 1.0;
  yh = 1.0;
  nflg = 0;
  
  if( aa <= 1.0 || bb <= 1.0 )
    {
      dithresh = 1.0e-6;
      rflg = 0;
      a = aa;
      b = bb;
      y0 = yy0;
      x = a/(a+b);
      y = incbet( a, b, x );
      goto ihalve;
    }
  else
    {
      dithresh = 1.0e-4;
    }
  /* approximation to inverse function */

  yp = -ndtri(yy0);
  
  if( yy0 > 0.5 )
    {
      rflg = 1;
      a = bb;
      b = aa;
      y0 = 1.0 - yy0;
      yp = -yp;
    }
  else
    {
      rflg = 0;
      a = aa;
      b = bb;
      y0 = yy0;
    }
  
  lgm = (yp * yp - 3.0)/6.0;
  x = 2.0/( 1.0/(2.0*a-1.0)  +  1.0/(2.0*b-1.0) );
  d = yp * sqrt( x + lgm ) / x
    - ( 1.0/(2.0*b-1.0) - 1.0/(2.0*a-1.0) )
    * (lgm + 5.0/6.0 - 2.0/(3.0*x));
  d = 2.0 * d;
  if( d < MINLOG )
    {
      x = 1.0;
      goto under;
    }
  x = a/( a + b * exp(d) );
  y = incbet( a, b, x );
  yp = (y - y0)/y0;
  if( fabs(yp) < 0.2 )
    goto newt;

  /* Resort to interval halving if not close enough. */
 ihalve:

  dir = 0;
  di = 0.5;
  for( i=0; i<100; i++ )
    {
      if( i != 0 )
	{
	  x = x0  +  di * (x1 - x0);
	  if( x == 1.0 )
	    x = 1.0 - MACHEP;
	  if( x == 0.0 )
	    {
	      di = 0.5;
	      x = x0  +  di * (x1 - x0);
	      if( x == 0.0 )
		goto under;
	    }
	  y = incbet( a, b, x );
	  yp = (x1 - x0)/(x1 + x0);
	  if( fabs(yp) < dithresh )
	    goto newt;
	  yp = (y-y0)/y0;
	  if( fabs(yp) < dithresh )
	    goto newt;
	}
      if( y < y0 )
	{
	  x0 = x;
	  yl = y;
	  if( dir < 0 )
	    {
	      dir = 0;
	      di = 0.5;
	    }
	  else if( dir > 3 )
	    di = 1.0 - (1.0 - di) * (1.0 - di);
	  else if( dir > 1 )
	    di = 0.5 * di + 0.5; 
	  else
	    di = (y0 - y)/(yh - yl);
	  dir += 1;
	  if( x0 > 0.75 )
	    {
	      if( rflg == 1 )
		{
		  rflg = 0;
		  a = aa;
		  b = bb;
		  y0 = yy0;
		}
	      else
		{
		  rflg = 1;
		  a = bb;
		  b = aa;
		  y0 = 1.0 - yy0;
		}
	      x = 1.0 - x;
	      y = incbet( a, b, x );
	      x0 = 0.0;
	      yl = 0.0;
	      x1 = 1.0;
	      yh = 1.0;
	      goto ihalve;
	    }
	}
      else
	{
	  x1 = x;
	  if( rflg == 1 && x1 < MACHEP )
	    {
	      x = 0.0;
	      goto done;
	    }
	  yh = y;
	  if( dir > 0 )
	    {
	      dir = 0;
	      di = 0.5;
	    }
	  else if( dir < -3 )
	    di = di * di;
	  else if( dir < -1 )
	    di = 0.5 * di;
	  else
	    di = (y - y0)/(yh - yl);
	  dir -= 1;
	}
    }
  mtherr( "incbi", PLOSS );
  if( x0 >= 1.0 )
    {
      x = 1.0 - MACHEP;
      goto done;
    }
  if( x <= 0.0 )
    {
    under:
      mtherr( "incbi", UNDERFLOW );
      x = 0.0;
      goto done;
    }
  
 newt:
  
  if( nflg )
    goto done;
  nflg = 1;
  lgm = lgam(a+b) - lgam(a) - lgam(b);
  
  for( i=0; i<8; i++ )
    {
      /* Compute the function at this point. */
      if( i != 0 )
	y = incbet(a,b,x);
      if( y < yl )
	{
	  x = x0;
	  y = yl;
	}
      else if( y > yh )
	{
	  x = x1;
	  y = yh;
	}
      else if( y < y0 )
	{
	  x0 = x;
	  yl = y;
	}
      else
	{
	  x1 = x;
	  yh = y;
	}
      if( x == 1.0 || x == 0.0 )
	break;
      /* Compute the derivative of the function at this point. */
      d = (a - 1.0) * log(x) + (b - 1.0) * log(1.0-x) + lgm;
      if( d < MINLOG )
	goto done;
      if( d > MAXLOG )
		break;
      d = exp(d);
      /* Compute the step to the next approximation of x. */
      d = (y - y0)/d;
      xt = x - d;
      if( xt <= x0 )
	{
	  y = (x - x0) / (x1 - x0);
	  xt = x0 + 0.5 * y * (x - x0);
	  if( xt <= 0.0 )
	    break;
	}
      if( xt >= x1 )
	{
	  y = (x1 - x) / (x1 - x0);
	  xt = x1 - 0.5 * y * (x1 - x);
	  if( xt >= 1.0 )
	    break;
	}
      x = xt;
      if( fabs(d/x) < 128.0 * MACHEP )
	goto done;
    }
  /* Did not converge.  */
  dithresh = 256.0 * MACHEP;
  goto ihalve;
  
 done:
  
  if( rflg )
    {
      if( x <= MACHEP )
	x = 1.0 - MACHEP;
      else
	x = 1.0 - x;
    }
  return( x );
}
