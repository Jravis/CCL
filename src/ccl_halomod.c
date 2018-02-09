#include "ccl_core.h"
#include "ccl_utils.h"
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "gsl/gsl_integration.h"
#include "gsl/gsl_interp.h"
#include "gsl/gsl_spline.h"
//#include "gsl/gsl_interp2d.h"
//#include "gsl/gsl_spline2d.h"
#include "ccl_placeholder.h"
#include "ccl_background.h"
#include "ccl_power.h"
#include "ccl_error.h"
#include "class.h"
#include "ccl_params.h"
#include "ccl_emu17.h"
#include "ccl_emu17_params.h"

// TODO: rename parameters for consistency.
// TODO: check if r_Delta is something equivalent in CCL.
double u_nfw_c(double c,double k, double m, double aa){
  // analytic FT of NFW profile, from Cooray & Sheth 2001
  double x, xu;
  double f1, f2, f3;
  x = k * r_Delta(m,aa)/c; // x = k*rv/c = k*rs = ks
  xu = (1.+c)*x; // xu = ks*(1+c)
  f1 = sin(x)*(gsl_sf_Si(xu)-gsl_sf_Si(x));
  f2 = cos(x)*(gsl_sf_Ci(xu)-gsl_sf_Ci(x));
  f3 = sinl(c*x)/xu;
  fc = log(1.+c)-c/(1.+c);
  return (f1+f2-f3)/fc;
}


double inner_I0j (ccl_cosmology *cosmo, double halomass, void *para, int status){
  double *array = (double *) para;
  long double u = 1.0; //the number one?
  double a= array[6]; //Array of ...
  double c = conc(halomass,a); //The halo concentration for this mass and scale factor
  int l;
  int j = (int)(array[5]);
  for (l = 0; l< j; l++){
    u = u*u_nfw_c(c,array[l],halomass,a);
  }
  // TODO: mass function should be the CCL call - check units due to changes (Msun vs Msun/h, etc)
  return ccl_massfunc(cosmo, halomass, a, odelta, status)*halomass*pow(halomass/(RHO_CRITICAL*cosmo.params->Omega_m),(double)j)*u;
}

// TODO: we need to change this to the CCL style integrator.
// TODO: pass the cosmology construct.
double I0j (int j, double k1, double k2, double k3, double k4,double a){
  double array[7] = {k1,k2,k3,k4,0.,(double)j,a};
  return int_gsl_integrate_medium_precision(inner_I0j,(void*)array,log(limits.M_min),log(limits.M_max),NULL, 2000);
}

// TODO: pass the cosmology construct around.
double p_1h(double k, double a)
{
  return I0j(2,k,k,0.,0.,a);
}

// TODO: parameter renaming as necessary.
double conc(double m, double a)
{
  return 9.*pow(nu(m,a),-.29)*pow(growfac(a)/growfac(1.),1.15);// Bhattacharya et al. 2011, Delta = 200 rho_{mean} (Table 2)
  //return 10.14*pow(m/2.e+12,-0.081)*pow(a,1.01); //Duffy et al. 2008 (Delta = 200 mean)
}