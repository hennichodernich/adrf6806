/*
** find rational approximation to given real number
** David Eppstein / UC Irvine / 8 Aug 1993
**
** With corrections from Arno Formella, May 2008
**
** usage: a.out r d
**   r is real number to approx
**   d is the maximum denominator allowed
**
** based on the theory of continued fractions
** if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
** then best approximation is found by truncating this series
** (with some adjustments in the last term).
**
** Note the fraction can be recovered as the first column of the matrix
**  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
**  ( 1  0 ) ( 1  0 ) ( 1  0 )
** Instead of keeping the sequence of continued fraction terms,
** we just keep the last partial product of these matrices.
*/

#include <stdio.h>
#include <math.h>

void frap(double x, int maxden, int *num, int *den)
{
    long m[2][2];
    double startx, error;
    long ai;

    startx = x;

    /* initialize matrix */
    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    /* loop finding terms until denom gets too big */
    while (m[1][0] *  ( ai = (long)x ) + m[1][1] <= maxden) {
	long t;
	t = m[0][0] * ai + m[0][1];
	m[0][1] = m[0][0];
	m[0][0] = t;
	t = m[1][0] * ai + m[1][1];
	m[1][1] = m[1][0];
	m[1][0] = t;
        if(x==(double)ai) break;     // AF: division by zero
	x = 1/(x - (double) ai);
        if(x>(double)0x7FFFFFFF) break;  // AF: representation failure
    } 

    /* now remaining x is between 0 and 1/ai */
    /* approx as either 0 or 1/m where m is max that will fit in maxden */
    /* first try zero */
    *num = m[0][0];
    *den = m[1][0];

    //printf("%ld/%ld, error = %e\n", m[0][0], m[1][0], startx - ((double) m[0][0] / (double) m[1][0]));
    error = fabs(startx - ((double) m[0][0] / (double) m[1][0]));

    /* now try other possibility */
    ai = (maxden - m[1][1]) / m[1][0];
    m[0][0] = m[0][0] * ai + m[0][1];
    m[1][0] = m[1][0] * ai + m[1][1];
	//    printf("%ld/%ld, error = %e\n", m[0][0], m[1][0], startx - ((double) m[0][0] / (double) m[1][0]));
    if (fabs(startx - ((double) m[0][0] / (double) m[1][0])) < error)
    {
	    *num = m[0][0];
	    *den = m[1][0];
    }
}
