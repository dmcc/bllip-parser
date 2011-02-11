// amoeba.h
//
// Based on the amebsa routine in Numerical Recipes

#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

// Multidimensional minimization of the function f(x) where x is a
// vector in ndim dimensions, by simulated annealing combined with the
// downhill simplex method of Nelder and Mead, as described in
// Numerical Recipes.

template <typename f_type, typename x_type, typename Float=float>
class amoeba_helper {

public:
  amoeba_helper(f_type& f, x_type& x, Float deltax,
		Float ftol, int maxiter) 
    : f(f), x(x), yb(f(x)), ftol(ftol), xb(x.size()), 
      ndim(x.size()), mpts(x.size()+1) 
  { 
    matrix p(mpts,ndim);
    vector y(mpts);
    
    for (unsigned j = 0; j < ndim; ++j)
      p[0][j] = xb[j] = x[j];
    y[0] = yb;
    
    Float temptr = 0;
    // load p and y
    for (unsigned i = 1; i <= ndim; ++i) {
      for (unsigned j = 0; j < ndim; ++j) 
	p[i][j] = x[j];
      x[i-1] += deltax;
      y[i] = f(x);
      x[i-1] -= deltax;
      p[i][i-1] += deltax;
      if (y[i] < yb)
	xb = p[i];
      temptr = std::max(fabsf(y[0]-y[i]), temptr);
    }
    // temptr /= 10;
    tt = -temptr;
    std::cout << "# temptr = " << temptr << std::endl;
    amebsa(p, y, maxiter);
    for (unsigned j = 0; j < ndim; ++j)
      x[j] = xb[j];
  }

  f_type& f;
  x_type& x;
  Float yb;      // best value seen so far

private:

  typedef boost::numeric::ublas::matrix<Float> matrix;
  typedef boost::numeric::ublas::vector<Float> vector;

  Float tt;
  const Float ftol;
  vector xb;
  unsigned ndim;
  unsigned mpts;
  
  static Float ran1() {
    return rand()/(RAND_MAX+1.0);
  }

  inline void get_psum(matrix& p, vector& psum)
  {
    for (unsigned n=0; n<ndim; n++) {
      Float sum = 0;
      for (unsigned m=0; m<mpts; m++) 
	sum += p[m][n];
      psum[n]=sum;
    }
  }
  
  // The input matrix p[0..ndim][0..ndim-1] has ndim+1 rows, each an
  // ndimdimensional vector which is a vertex of the starting
  // simplex. Also input are the following: the vector y[0..ndim],
  // whose components must be pre-initialized to the values of funk
  // evaluated at the ndim+1 vertices (rows) of p; ftol, the
  // fractional convergence tolerance to be achieved in the function
  // value for an early return; iter, and temptr. The routine makes
  // iter function evaluations at an annealing temperature temptr,
  // then returns. You should then decrease temptr according to your
  // annealing schedule, reset iter, and call the routine again
  // (leaving other arguments unaltered between calls). If iter is
  // returned with a positive value, then early convergence and return
  // occurred. If you initialize yb to a very large value on the first
  // call, then yb and pb[1..ndim] will subsequently return the best
  // function value and point ever encountered (even if it is no
  // longer a point in the simplex).

  void amebsa(matrix& p, vector& y, int& iter)
  {
    unsigned i, j;
    unsigned ihi, ilo, n;
    Float rtol,yhi,ylo,ynhi,ysave,yt,ytry;
  
    vector psum(ndim);

    get_psum(p, psum);
    for (;;) {
    
      // Determine which point is the highest (worst),
      // next-highest, and lowest (best).

      ilo=0;
      ihi=1;

      // Whenever we "look at" a vertex, it gets a random thermal
      // fluctuation.

      ynhi = ylo = y[0] + tt*log(ran1());
      yhi = y[1]+tt*log(ran1());
      if (ylo > yhi) {
	ihi=0;
	ilo=1;
	ynhi=yhi;
	yhi=ylo;
	ylo=ynhi;
      }

      // Loop over the points in the simplex.

      for (i=2; i<mpts; i++) {
	// More thermal fluctuations.
	yt = y[i]+tt*log(ran1());
	if (yt <= ylo) {
	  ilo = i;
	  ylo = yt;
	}
	if (yt > yhi) {
	  ynhi = yhi;
	  ihi = i;
	  yhi = yt;
	} 
	else if (yt > ynhi) {
	  ynhi = yt;
	}
      }

      // Compute the fractional range from highest to lowest and return
      // if satisfactory.

      rtol = 2.0*fabs(yhi-ylo)/(fabs(yhi)+fabs(ylo));
      if (rtol < ftol || iter < 0) {
	// If returning, put best point and value in slot 1.
	std::swap(y[0], y[ilo]);
	for (n=0; n<ndim; n++)
	  std::swap(p[0][n], p[ilo][n]);
	break;
      }
      iter -= 2;

      // Begin a new iteration. First extrapolate by a factor 1 through
      // the face of the simplex across from the high point, i.e.,
      // reflect the simplex from the high point.

      ytry = amotsa(p,y,psum,ihi,yhi,-1.0);
      if (ytry <= ylo) {
	// Gives a result better than the best point, so try an
	// additional extrapolation by a factor of 2.
	ytry = amotsa(p,y,psum,ihi,yhi,2.0);
      } else if (ytry >= ynhi) {
	// The reflected point is worse than the second-highest, so look
	// for an intermediate lower point, i.e., do a one-dimensional
	// contraction.
	ysave = yhi;
	ytry = amotsa(p,y,psum,ihi,yhi,0.5);
	if (ytry >= ysave) {
	  // Can't seem to get rid of that high point. Better contract
	  // around the lowest (best) point.
	  for (i=0; i<mpts; i++) {
	    if (i != ilo) {
	      for (j=0; j<ndim;j ++) {
		x[j] = psum[j] = 0.5*(p[i][j]+p[ilo][j]);
		p[i][j] = psum[j];
	      }
	      y[i] = f(x);
	    }
	  }
	  iter -= ndim;
	  // Recompute psum.
	  get_psum(p, psum);
	}
      } 
      else 
	++iter; // Correct the evaluation count.
    }
  } 
  
  // Extrapolates by a factor fac through the face of the simplex
  // across from the high point, tries it, and replaces the high point
  // if the new point is better.
  
  Float amotsa(matrix& p, vector& y, vector& psum, 
	       const int ihi, Float& yhi, const Float fac)
  {
    unsigned j;
    Float fac1,fac2,yflu,ytry;

    vector ptry(ndim);
    fac1 = (1.0-fac)/ndim;
    fac2 = fac1-fac;
    for (j=0; j<ndim;j ++)
      x[j] = ptry[j] = psum[j]*fac1-p[ihi][j]*fac2;
    ytry = f(x);
    if (ytry <= yb) {
      xb = ptry;
      yb = ytry;
    }
    
    // We added a thermal fluctuation to all the current vertices, but
    // we subtract it here, so as to give the simplex a thermal
    // Brownian motion: It likes to accept any suggested change.
    
    yflu = ytry-tt*log(ran1());
    if (yflu < yhi) {
      y[ihi] = ytry;
      yhi = yflu;
      for (j=0; j<ndim; j++) {
	psum[j] += ptry[j]-p[ihi][j];
	p[ihi][j] = ptry[j];
      }
    }
    return yflu;
  }
}; 


template <typename f_type, typename x_type>
double amoeba(f_type& f, x_type& x, double deltax=1, float ftol=1e-7, int maxiter=10000) {
  amoeba_helper<f_type,x_type> a(f, x, deltax, ftol, maxiter);
  return a.yb;
}
