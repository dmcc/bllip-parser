// Licensed under the Apache License, Version 2.0 (the "License"); you may
// not use this file except in compliance with the License.  You may obtain
// a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.

// tao-optimizer.h
//
// Mark Johnson, 17th July 2005
//
// A C++ interface to the TAO 1.8 optimization system.  This requires that
// you have installed PETSc 2.3 and TAO, and have initialized the environment
// variables (e.g., in your Makefile)
//
// setenv TAO_DIR /usr/local/share/tao
// setenv PETSC_DIR /usr/local/share/petsc
// setenv PETSC_ARCH `$PETSC_DIR/bin/petscarch`
// setenv BOPT O_c++
//
// A single tao_environment object must exist while any optimizers exist.
//
// You must pass argc, argv to the initialization of tao_environment.
//
// TAO does crazy things to your working directory and your environment,
// which I try to correct here.  Sigh.
//
// The tao_optimizer object's [] can be used to change the starting point
// of optimization.
//
// INSTALLING PETSC AND TAO
//
// I downloaded Petsc and Tao into /usr/local/share and then did the following
/*

tar zxvf ~/sources/TAO/petsc-2.3.3-p8.tar.gz
tar zxvf ~/sources/TAO/tao-1.9.tar.gz
ln -s petsc-2.3.3-p8 petsc
ln -s tao-1.9 tao
cd petsc
# config/configure.py --with-clanguage="C++" --with-x=0 --with-mpi=0 --with-shared --download-f-blas-lapack=1 --CFLAGS="-march=native -O6 -finline-functions -mfpmath=sse -msse2 -mmmx" --CXXFLAGS="-march=native -O6 -finline-functions -mfpmath=sse -msse2 -mmmx" --FC=gfortran --FFLAGS="-march=native -O6 -finline-functions -mfpmath=sse -msse2 -mmmx"
config/configure.py --with-clanguage="C++" --with-x=0 --with-mpi=0 --with-shared --download-f-blas-lapack=1 --CFLAGS="-O6 -finline-functions" --CXXFLAGS="-O6 -finline-functions" --FC=gfortran --FFLAGS="-O6 -finline-functions"
make all
cd ../tao
make all

*/

#ifndef TAO_OPTIMIZER_H
#define TAO_OPTIMIZER_H

#include <cassert>
#include <iostream>
#include <signal.h>  // for signal()
#include <unistd.h>  // for chdir() and getcwd()

#include "tao.h"    // Unfortunately this pollutes the namespace

//! tao_environment{} initializes and maintains the context for PETSc and Tao.
//! It must be created before any Tao optimizer objects are created, and
//! it cannot be destroyed until all such objects are destroyed.
//! It requires the argc and argv arguments that main() was called with.
//
struct tao_environment {

  tao_environment(int& argc, char**& argv, const char* help = "") {

    // Initialize TAO and PETSc

    char* cwd = getcwd(NULL, 0);   // save current working directory

    int info;
    info = PetscInitialize(&argc, &argv, (char *) 0, help); assert(info == 0);
    info = TaoInitialize(&argc, &argv, (char *) 0, help); assert(info == 0);

    // undo changes that Petsc/TAO makes -- sigh
    
    signal(SIGINT, SIG_DFL);       // turn on interrupts
    chdir(cwd);                  // reset current working directory
    free(cwd);

  }  //tao_environment::tao_environment()

  ~tao_environment() {
    TaoFinalize();
    PetscFinalize();
  }  //tao_environment::~tao_environment()

  //! get_double_option() returns the double value associated with the command line
  //! option name.
  //
  double get_double_option(const char name[], PetscReal value = 0.0) {
    int info = PetscOptionsGetReal(PETSC_NULL, name, &value, PETSC_NULL);
    assert(info == 0);
    return value;
  }  // tao_environment::get_double_option()

 
  //! get_int_option() returns the int value associated with the command line
  //! option name.
  //
  int get_int_option(const char name[], int value = 0) {
    int info = PetscOptionsGetInt(PETSC_NULL, name, &value, PETSC_NULL);
    assert(info == 0);
    return value;
  }  // tao_environment::get_int_option()

  //! get_bool_option() returns true iff the command line option appeared
  //
  bool get_bool_option(const char name[]) {
    PetscTruth value = PETSC_FALSE;
    int info = PetscOptionsGetTruth(PETSC_NULL, name, &value, PETSC_NULL);
    assert(info == 0);
    return value;
  }  // tao_environment::get_bool_option()

  //! get_cstr_option() returns the string value associated with the
  //! command line option name.  Returns NULL if option not found.
  //
  const char* get_cstr_option(const char name[]) {
    static char cstr[512];
    PetscTruth flg = PETSC_FALSE;
    int info = PetscOptionsGetString(PETSC_NULL, name, cstr, 511, &flg);
    assert(info == 0);
    if (flg)
      return cstr;
    else
      return NULL;
  }  // tao_environment::get_cstr_option()

};  // tao_environment{}


//! tao_optimizer_base{} contains code that is common to both constrained and
//! unconstrained optimization.
//
class tao_optimizer_base {

protected:

  Vec             X;                //!< PETSc representation of solution vector  
  double*         x;                //!< local copy of solution vector

  TAO_SOLVER      tao;
  TAO_APPLICATION tao_application;
  
public:

  //! returns a reference to the ith coordinate of the optimal point.
  //! Use this to set the starting point before calling optimize or
  //! retrieve the optimum after optimization.
  //
  inline double& operator[] (size_t i) const {
    return x[i];
  }  // tao_optimizer_base::operator[]()

  //! returns the dimension of the space over which optimization is performed
  //
  inline size_t size() const {
    int nx;
    int info = VecGetSize(X, &nx); assert(info == 0);
    assert(nx >= 0);
    return nx;
  }  // tao_optimizer_base::size()

  //! Returns the termination reason
  //
  const char* termination_reason() const {
    TaoTerminateReason reason;
    int info = TaoGetTerminationReason(tao, &reason); assert(info == 0);
    switch (reason) {
    case 2: 
      return "TAO_CONVERGED_ATOL (2), (res <= atol)";
      break;
    case 3:
      return "TAO_CONVERGED_RTOL (3), (res/res0 <= rtol)";
      break;
    case 4:
      return "TAO_CONVERGED_TRTOL (4), (xdiff <= trtol)";
      break;
    case 5:
      return "TAO_CONVERGED_MINF (5), (f <= fmin)";
      break;
    case 6:
      return "TAO_CONVERGED_USER (6), (user defined)";
      break;
    case -2:
      return "TAO_DIVERGED_MAXITS (-2), (its>maxits)";
      break;
    case -4:
      return "TAO_DIVERGED_NAN (-4), (Numerical problems)";
      break;
    case -5:
      return "TAO_DIVERGED_MAXFCN (-5), (nfunc > maxnfuncts)";
      break;
    case -6:
      return "TAO_DIVERGED_LS_FAILURE (-6), (line search failure)";
      break;
    case -7:
      return "TAO_DIVERGED_TR_REDUCTION (-7)";
      break;
    case -8:
      return "TAO_DIVERGED_USER (-8), (user defined)";
      break;
    case 0:
      return "TAO_CONTINUE_ITERATING  (0)";
      break;
    }
    return "Unrecognized termination reason";
  }  // tao_optimizer_base::termination_reason()  

  //! tolerances_type{} holds the tolerances used by a TAO optimizer.
  //
  struct tolerances_type {
    double fatol;	//!< absolute convergence tolerance
    double frtol;	//!< relative convergence tolerance
    double catol;	//!< trust region convergence tolerance
    double crtol;	//!< convergence of the function evaluates less than this tolerance
    double gatol;	//!< the absolute gradient tolerance
    double grtol;	//!< the relative gradient tolerance
    double gttol;	//!< the gradient reduction tolerance
  };  // tao_optimizer_base::tolerances_type{}

  //! get_tolerances() returns the convergence tolerances used by TAO
  //
  void get_tolerances(struct tolerances_type& t) const 
  {
    int info = TaoGetTolerances(tao, &t.fatol, &t.frtol, &t.catol, &t.crtol); assert(info == 0);
    info = TaoGetGradientTolerances(tao, &t.gatol, &t.grtol, &t.gttol); assert(info == 0);
  }  // tao_optimizer_base::get_tolerances()
    

  ~tao_optimizer_base()
  {
    // Free TAO data structures

    int info;
    info = VecDestroy(X); assert(info == 0);
    info = TaoDestroy(tao); assert(info == 0);
    info = TaoApplicationDestroy(tao_application); assert(info == 0); 
  }  // tao_optimizer_base::~tao_optimizer_base()

};  // tao_optimizer_base{}

inline std::ostream& operator<< (std::ostream& os, const tao_optimizer_base::tolerances_type& t)
{
  return os << "fatol = " << t.fatol << ", frtol = " << t.frtol 
	    << ", catol = " << t.catol << ", crtol = " << t.crtol 
	    << ", gatol = " << t.gatol <<  ", grtol = " << t.grtol
	    << ", gttol = " << t.gttol;
}  // operator<<(,)

//! tao_optimizer{} performs unconstrained optimization of a user-specified function.
//
template <typename f_df_type>
class tao_optimizer : public tao_optimizer_base {

  //! Callback used by Tao
  
  static int callback(TAO_APPLICATION tao_application, Vec X, double* fx, Vec DF_DX, 
		      void* data)
  {
    typedef f_df_type* f_df_ptr_type;
    f_df_type& f_df = *f_df_ptr_type(data);
    int info;
    
    // Get pointers to vector data

    double* x;
    info = VecGetArray(X, &x); assert(info == 0);
    double* df_dx;
    info = VecGetArray(DF_DX, &df_dx); assert(info == 0);

    int nx;
    info = VecGetSize(X, &nx); assert(info == 0);
    int ndf_dx;
    info = VecGetSize(DF_DX, &ndf_dx); assert(info == 0);
    assert(nx == ndf_dx);

    // Compute f(x) and df_dx

    *fx = f_df(nx, x, df_dx);

    // Restore vectors

    info = VecRestoreArray(X, &x); assert(info == 0);
    info = VecRestoreArray(DF_DX, &df_dx); assert(info == 0);
    
    return 0;
  }  // tao_optimizer::callback()

public:

  //! Sets the function to be optimized and initializes the optimizer
  //
  tao_optimizer(int n,             //!< Dimension of solution vector
		f_df_type& f_df    //!< f(n, x, df_dx) returns f(x) and sets df/dx
		                   //!< where f is the function to be minimized
		                   //!< x (an array of size n) is the point at which f is evaluated
		                   //!< n is the dimensionality of x
		                   //!< df/dx is the derivative of f at x (an array of size n)
		)
  {
    int info;

    // Allocate vectors
    
    info = VecCreateSeq(PETSC_COMM_SELF, n, &X); assert(info == 0);
    {
      PetscScalar zero = 0.0;
      info = VecSet(X, zero); assert(info == 0);
    }

    // Create TAO solver

    info = TaoCreate(PETSC_COMM_SELF, "tao_lmvm", &tao); assert(info == 0);
    info = TaoApplicationCreate(PETSC_COMM_SELF, &tao_application);
    assert(info == 0);

    info = TaoSetFromOptions(tao); assert(info == 0);
    info = TaoSetMethodFromOptions(tao); assert(info == 0);

    info = TaoAppSetInitialSolutionVec(tao_application, X); assert(info == 0);

    // Set routine for function evaluation and gradient

    info = TaoAppSetObjectiveAndGradientRoutine(tao_application,
						callback, (void *) (&f_df));
    assert(info == 0);

    // Load the initial values into x

    info = VecGetArray(X, &x); assert(info == 0);

  }  // tao_optimizer::tao_optimizer()

  //! Actually runs the optimization
  //
  tao_optimizer& optimize() {

    // Restore the solution vector

    int info;
    info = VecRestoreArray(X, &x); assert(info == 0);
    
    // Run the solver

    info = TaoSolveApplication(tao_application, tao); assert(info == 0);

    // Load the solution into x

    info = VecGetArray(X, &x); assert(info == 0);
    
    return *this;
  }  // tao_optimizer::optimize()

};  // tao_optimizer{}


//! writes out a tao_optimizer{}
//
template <typename f_df_type>
std::ostream& operator<< (std::ostream& os, const tao_optimizer<f_df_type>& x)
{
  os << '(';
  if (x.size() > 0) 
    os << x[0];
  for (size_t i = 1; i < x.size(); ++i) 
    os << ' ' << x[i];
  return os << ')';
}


//! tao_constrained_optimizer() performs unconstrained optimization of a user-specified function.
//
template <typename f_df_type>
class tao_constrained_optimizer : public tao_optimizer_base {

  //! Callback used by Tao
  
  static int callback(TAO_APPLICATION tao_application, Vec X, double* fx, Vec DF_DX, 
		      void* data)
  {
    typedef f_df_type* f_df_ptr_type;
    f_df_type& f_df = *f_df_ptr_type(data);
    int info;
    
    // Get pointers to vector data

    double* x;
    info = VecGetArray(X, &x); assert(info == 0);
    double* df_dx;
    info = VecGetArray(DF_DX, &df_dx); assert(info == 0);

    int nx;
    info = VecGetSize(X, &nx); assert(info == 0);
    int ndf_dx;
    info = VecGetSize(DF_DX, &ndf_dx); assert(info == 0);
    assert(nx == ndf_dx);

    // Compute f(x) and df_dx

    *fx = f_df(nx, x, df_dx);

    // Restore vectors

    info = VecRestoreArray(X, &x); assert(info == 0);
    info = VecRestoreArray(DF_DX, &df_dx); assert(info == 0);
    
    return 0;
  }  // tao_constrained_optimizer::callback()

  Vec             XL;               //!< PETSc representation of lower bounds
  Vec             XU;               //!< PETSc representation of upper bounds

public:

  double*         lower_bound;      //!< local copy of lower bounds
  double*         upper_bound;      //!< local copy of upper bounds

  //! Sets the function to be optimized and initializes the constrained_optimizer
  //
  tao_constrained_optimizer(int n,           //!< Dimension of solution vector
			    f_df_type& f_df  //!< f(n, x, df_dx) returns f(x) and sets df/dx
			                     //!< where f is the function to be minimized
			                     //!< x (an array of size n) is the point at which f is evaluated
		                             //!< n is the dimensionality of x
		                             //!< df/dx is the derivative of f at x (an array of size n)
			    )
  {
    int info;

    // Allocate vectors
    
    info = VecCreateSeq(PETSC_COMM_SELF, n, &X); assert(info == 0);
    info = VecCreateSeq(PETSC_COMM_SELF, n, &XL); assert(info == 0);
    info = VecCreateSeq(PETSC_COMM_SELF, n, &XU); assert(info == 0);
    {
      PetscScalar zero = 0.0;
      info = VecSet(X, zero); assert(info == 0);
    }

    // Create TAO solver

    info = TaoCreate(PETSC_COMM_SELF, "tao_blmvm", &tao); assert(info == 0);
    info = TaoApplicationCreate(PETSC_COMM_SELF, &tao_application);
    assert(info == 0);

    info = TaoSetFromOptions(tao); assert(info == 0);
    info = TaoSetMethodFromOptions(tao); assert(info == 0);

    info = TaoAppSetInitialSolutionVec(tao_application, X); assert(info == 0);

    // Set routine for function evaluation and gradient

    info = TaoAppSetObjectiveAndGradientRoutine(tao_application,
						callback, (void *) (&f_df));
    assert(info == 0);

    // Load the initial conditions into x, xu, xl

    info = VecGetArray(X, &x); assert(info == 0);
    info = VecGetArray(XL, &lower_bound); assert(info == 0);
    info = VecGetArray(XU, &upper_bound); assert(info == 0);

  }  // tao_constrained_optimizer::tao_constrained_optimizer()


  //! Actually runs the optimization
  //
  tao_constrained_optimizer& optimize() {

    // Restore the initial conditions vectors

    int info;
    info = VecRestoreArray(X, &x); assert(info == 0);
    info = VecRestoreArray(XL, &lower_bound); assert(info == 0);
    info = VecRestoreArray(XU, &upper_bound); assert(info == 0);
    
    TaoAppSetVariableBounds(tao_application, XL, XU);

    // Run the solver

    info = TaoSolveApplication(tao_application, tao); assert(info == 0);

    // Load the solution into x

    info = VecGetArray(X, &x); assert(info == 0);

    return *this;
  }  // tao_constrained_optimizer::optimize()


  ~tao_constrained_optimizer()
  {
    // Free TAO data structures

    int info;
    info = VecDestroy(XL); assert(info == 0);
    info = VecDestroy(XU); assert(info == 0);
  }  // tao_constrained_optimizer::~tao_constrained_optimizer()
  
};  // tao_constrained_optimizer{}


//! writes out a tao_constrained_optimizer{}
//
template <typename f_df_type>
std::ostream& operator<< (std::ostream& os, const tao_constrained_optimizer<f_df_type>& x)
{
  os << '(';
  if (x.size() > 0) 
    os << x[0];
  for (size_t i = 1; i < x.size(); ++i) 
    os << ' ' << x[i];
  return os << ')';
}

//! numerical_derivative{} computes a numerical approximation to the derivative
//! of f. 
//
template <typename f_type>
struct numerical_derivative {

  f_type& f;	//!< f(n, x), where x[n]
  double  dx;	//!< step size used in derivative calculation

  numerical_derivative(f_type& f, 	     //!< f(n, x), where x[n]
		       double dx = 1e-7	     //!< step size used in derivative calculation
		       ) : f(f), dx(dx) { }

  //! Computes the numerical approximation to derivative, returns function value at x.
  //
  double operator() (size_t nx,              //!< dimension of x
		     double x[],             //!< point at which to calculate derivative
		     double df_dx[]          //!< derivative
		     ) {
    double fx = f(nx, x);
    for (size_t i = 0; i < nx; ++i) {
      double xi = x[i];
      x[i] += dx;
      df_dx[i] = (f(nx, x) - fx)/dx;
      x[i] = xi;
    }
    return fx;
  }  // numerical_derivative::operator()

}; // numerical_derivative{}

#endif // TAO_OPTIMIZER
