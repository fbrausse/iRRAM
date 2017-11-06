/*

TaylorModel.h - Prototype for an implementation of Taylor Models in iRRAM

Copyright (C) 2015 Norbert Mueller, Franz Brausse
 
This file is part of the iRRAM Library.
 
The iRRAM Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
 
The iRRAM Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.
 
You should have received a copy of the GNU Library General Public License
along with the iRRAM Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. 
*/

/* iRRAM version of simplified Taylor Models.
 * Each error symbol is implemented as a REAL variable, (i.e. an interval!)
 * 
 * Version info: 2015-09-13
 * Changes: Take an extra variable to split the coefficient c0 into center and radius
 * 
 * 
 * 
 * TODO: 
 * (a) This version is restricted to only a subset of interesting functions
 * and to order 1. 
 * (b) Currently, there is no division of models, and no versions for
 * trigonometric functions.
 * (c) We should assign error symbols to basic constants like pi, ln(2) and e (?)
 *
 * 
 * TODO: ID's of error symbols:
 *  not yet implemented // 2k   -  usual error symbols, for lambda_k
 *  not yet implemented // 2k+1 -  for square of lambda_k
 * 
 * TODO: The test procedures have to be checked how far they still are reasonable
 * 
 * 
 * 
 */


#ifndef iRRAM_TaylorModel_H
#define iRRAM_TaylorModel_H


#include "iRRAM.h"
#include <vector>
#include <memory>       /* std::shared_ptr */


namespace iRRAM {


class TM {

public: 
  class error_symbol {
  public:
  unsigned long long id;
  unsigned long long ref_count;
//   error_symbol(){cerr<<"es ? +\n";}
//  ~error_symbol(){cerr<<"es "<<id<<" -\n";}
};

private:


/* The max_id has to be unique to distinguish different error symbols.
 * TODO: For multithreaded versions of the iRRAM, this should be atomic!
 */
  static unsigned long long max_id;
  static unsigned default_sweep;
  static int prec_diff;
/* helper function to destruct the internal representation of REAL x into
 * center and radius, where both are REAL themselves.
 * 
 * TODO: write a version that does not have to transform a double interval into
 * a multiple precision interval first.
 * 
 * TODO: There is a transformation of the error mantissa into a signed version.
 * Maybe we should a conversion of unsigned to REAL
 */


public: 
  class I {
  public:
    unsigned long long id;
    std::shared_ptr<error_symbol> es;
    REAL ci;
    I(){
      id=0;
      es=NULL;
      ci=REAL(0);
    }
    I(const REAL& x){
      id=max_id++;
      es=std::make_shared<error_symbol>();
      ci=x;
      es->id=id;
//      cerr<<"rf0 "<< es.use_count()<<" # "<<es->id<<"\n";
    }
    I(const I& i,const REAL& x){
      id=i.id;
      es=i.es;
      ci=x;
//      cerr<<"rf1 "<< es.use_count()<<" # "<<es->id<<"\n";
    }
    I(const I& i){
      id=i.id;
      es=i.es;
      ci=i.ci;
//      cerr<<"rf2 "<< es.use_count()<<" # "<<es->id<<"\n";
    }
    I(I&& i){
      id=i.id;
      es=i.es;
      ci=i.ci;
//      cerr<<"rf3 "<< es.use_count()<<" # "<<es->id<<"\n";
    }
    I& operator = (const I& i){
      id=i.id;
      es=i.es;
      ci=i.ci;
//      cerr<<"rf4 "<< es.use_count()<<" # "<<es->id<<"\n"; 
      return *this;
    };
    
    ~I(){
//      cerr<<"rf- "<< es.use_count()<<" # "<<es->id<<"\n";
    }
  };

static void my_to_formal_ball(const REAL&x, REAL&center, REAL&radius){
  DYADIC d;
  sizetype r;
  x.to_formal_ball(d,r);
  center=REAL(d);
  radius=scale(REAL(signed(r.mantissa)),r.exponent);
}  


public:              /* TODO: This should be private in the new future! */
  REAL c0;           /* center of the Taylor Model */
  REAL e0;           /* [c0 +- e0 ] is the basic coefficient of the Taylor model */
  std::vector<I> c;  /* error symbols with their IDs */
  unsigned sweepto;  /* For testing purposes: where should sweeping be directed to? */

//   TM(TM&& t): c(std::move(t.c)),sweepto(t.sweepto){c0=t.c0;}
//   TM(const TM& t): c(t.c),sweepto(t.sweepto){c0=t.c0;}
public:

/* default constructor */
TM(){}

// TM& operator = (const TM& tm){
//       c0=tm.c0;
//       e0=tm.e0;
//       c= tm.c;
//       sweepto=tm.sweepto;
//       return *this;
// };



/* Constructor for conversion from REAL to TM
 * This does NOT introduce an error symbol!
 * The purpose is just to allow an automatic conversion and 
 * to simplify the formulation of algorithms.
 */
TM(const REAL& x,const unsigned swpto=default_sweep):sweepto(swpto){my_to_formal_ball(x,c0,e0);}

~TM(){c.clear();};

// conversion from a TM to a REAL
REAL to_real() const {
	REAL zero_one=0;
	sizetype zo;
	sizetype_set(zo,1,0);
	zero_one.seterror(zo);
	REAL s=c0 + e0*zero_one;
	for (const I &i : c) {
		s += i.ci * zero_one;
	}
	return s;
}

void to_int(REAL& center, REAL& error) const {
  my_to_formal_ball(c0,center,error);
  error += e0;
    for (const I &i : c) {
	error += abs(i.ci);
    }
}

// explicit cast from a TM to a REAL
explicit inline operator REAL(void) const{
	return to_real();
}

static void set_default_sweep(unsigned swp){default_sweep=swp;}
static void set_prec_diff    (unsigned prd){prec_diff=prd;}


void geterror_info(sizetype& error){
	c0.geterror(error); 
	sizetype error2;
	e0.getsize(error2);
	sizetype_inc(error,error2);	
	for (I &i : c){
		i.ci.getsize(error2);
		sizetype_inc(error,error2);
	}
}




//merge the rounding error and  all but the three largest error symbols into a new error symbol
void round8(){
  unsigned long long max1id=max_id+2;
  unsigned long long max2id=max_id+2;
  unsigned long long max3id=max_id+2;
  if (c.size()>0){
    sizetype max1size;
    sizetype_exact(max1size);
    sizetype max2size;
    sizetype_exact(max2size);
    sizetype max3size;
    sizetype_exact(max3size);
    for (const I &i : c){
      sizetype cis= i.ci.vsize;
      sizetype_shift(cis,cis,-30);
      if (i.es->ref_count> 1) {
      if (sizetype_less(i.ci.error,cis) && sizetype_less(max3size,i.ci.vsize)){
        if (sizetype_less(max1size,i.ci.vsize)){
	max3id=max2id; max3size=max2size;
	max2id=max1id; max2size=max1size;
	max1id=i.id;   max1size=i.ci.vsize;
      }	else if (sizetype_less(max2size,i.ci.vsize)){
	max3id=max2id; max3size=max2size;
	max2id=i.id; max2size=i.ci.vsize;
      } else{
	max3id=i.id; max3size=i.ci.vsize;
      }
    }}}
  }  

  REAL center,error;
  std::vector<I> cnew;
  my_to_formal_ball(c0,center,error);
  error += e0;
  for (const I &i : c) {
    if (i.es->ref_count==1) cerr << i.es->id<< " single use error symbol\n";
    if ((i.id==max1id)||(i.id==max2id)||(i.id==max3id)){
      cnew.push_back(I(i));
    } else {
      i.es->ref_count--;
      error += abs(i.ci);
    }
  }  
//  cerr<<"a\n";
  REAL ec,er;
  my_to_formal_ball(error,ec,er);
  cnew.push_back(I(ec+er));
//  cerr<<"b\n";
  c=cnew;
  e0=0;
  c0=center;
//  cerr<<"c\n";
}






//test whether the rounding error is larger than the error of one of the error symbols
// or whether an error symbol is close to exhaustion (i.e. it's own error is too large) 
/* private */ bool test7(){
  if (c.size()>0){
   sizetype c0e=c0.error;
   sizetype_inc(c0e,e0.vsize);
   sizetype_shift(c0e,c0e,10);  
    for (unsigned i=0;i< c.size();i++){
       sizetype cis=c[i].ci.vsize;
       if (sizetype_less(cis,c0e)){
//	 cerr << "compare polish: "<< c[i].id<<"\n"; 
	 return true;
       }
       sizetype_shift(cis,cis,-25);  
       if (sizetype_less(cis,c[i].ci.error)) {
//	 cerr << "large error polish: "<< c[i].id<<"\n"; 
	 return true;
      } 
    }
    return false;
  };
  return true;
}




/* polish: methods to test whether polishing is needed*/

inline static bool polish_test(TM &x){
  return x.test7();
}

inline static bool polish_test(std::vector<TM>& x){
  for (unsigned i=0; i< x.size(); i++){
    if (polish_test(x[i])) return true;
  }
  return false;
}

template<class PARAM1, class PARAM2, class... PARAMS>
inline static bool polish_test(PARAM1 &x,PARAM2& y, PARAMS&... z ){
  return polish_test(x) || polish_test(y,z...) ;
}



/* polish: methods to prepare polishing by resetting the ref_counts*/

inline static void polish_reset(TM &x){
  for (const I &i : x.c) {
    i.es->ref_count=0;
  }
}

inline static void polish_reset(std::vector<TM>& x){
  for (unsigned j=0; j< x.size(); j++) polish_reset(x[j]);
}

template<class PARAM1, class PARAM2, class... PARAMS>
inline static void polish_reset(PARAM1 &x,PARAM2& y, PARAMS&... z ){
  polish_reset(x);
  polish_reset(y,z...);
}


/* polish: methods to prepare polishing by recomputing the ref_counts*/

inline static void polish_prepare(TM &x){
  for (const I &i : x.c) {
    i.es->ref_count++;
  }
}

inline static void polish_prepare(std::vector<TM>& x){
  for (unsigned j=0; j< x.size(); j++) polish_prepare(x[j]);
}

template<class PARAM1, class PARAM2, class... PARAMS>
inline static void polish_prepare(PARAM1 &x,PARAM2& y, PARAMS&... z ){
  polish_prepare(x);
  polish_prepare(y,z...);
}


/* polish: methods to actually perform the polishing*/

inline static void polish_perform(TM &x){
  x.round8();
}

inline static void polish_perform(std::vector<TM>& x){
  for (unsigned i=0; i< x.size(); i++){
    x[i].round8();
  }
}

template<class PARAM1, class PARAM2, class... PARAMS>
inline static void polish_perform(PARAM1 &x,PARAM2& y, PARAMS&... z ){
  polish_perform(x);
  polish_perform(y,z...);
}


/* polish: main routine */

template<class... PARAMS>
inline static void polish(PARAMS&... x){ 
  if (polish_test(x...)){
    polish_reset(x...);
    polish_prepare(x...);
    polish_perform(x...);   
  }
}



























TM & operator+=(const TM &tm)
	{
// 	cerr << "Parameter 1 "<< (*this) << "\n";
// 	cerr << "Parameter 2 "<< tm << "\n";
	{ stiff code(prec_diff); c0+=tm.c0; e0 += tm.e0;}
	for (const I &i : tm.c) {
		for (I &j : c) {
			if (i.id == j.id) {
				j.ci += i.ci;
				goto ok;
			}
		}
		c.push_back(I(i));
ok:;
	}
// 	cerr << "Addition "<< (*this) << "\n";
	return *this;
}

TM & operator-=(const TM &tm){
// 	cerr << "Parameter 1 "<< (*this) << "\n";
// 	cerr << "Parameter 2 "<< tm << "\n";
	{ stiff code(prec_diff); c0 -= tm.c0; e0 += tm.e0;}
	for (const I &i : tm.c) {
		for (I &j : c) {
			if (i.id == j.id) {
				j.ci -= i.ci;
				goto ok;
			}
		}
		c.push_back(I(i,-i.ci));
ok:;
	}
// 	cerr << "Differenz "<< (*this) << "\n";
	return *this;
}

TM& operator *= (const REAL &r){
	{ stiff code(prec_diff); c0 *=r; e0 *= abs(r);}
	for (I &l : c)
		  l.ci *= r;
	return *this;
}


friend TM  operator * (const TM &q, const TM &r){
	TM f=q*r.c0;
	REAL q_real = q.to_real();
	f.e0 += abs(q_real)*r.e0;
	if (q.sweepto==0){// old sweep into the basic coefficient
	  for (const I &i : r.c){
	    f.e0 += abs(q_real*i.ci);
	  }
	  return f;
	} else {// new sweep keeping linear dependency
	  TM g=TM(REAL(0),q.sweepto);
	  for (const I &i : r.c) 
	    g.c.push_back(I(i,q_real*i.ci));
	  return f+g;
	}
}


friend TM operator +  (const TM &q, const TM &r){ return TM(q) += r; }
friend TM operator -  (const TM &q, const TM &r){ return TM(q) -= r; }
friend TM operator *  (const TM &q, const REAL &r){ return TM(q) *= r; }


friend orstream & operator<<(orstream &o, const TM &p)
{
	o << swrite(p.c0,20,iRRAM_float_show) <<" swpt: "<< p.sweepto <<" i.e. "<< p.c0.vsize.mantissa<<"*2^" <<p.c0.vsize.exponent<<" +- "<< p.c0.error.mantissa<<"*2^" <<p.c0.error.exponent;
	o << "\n   + (" << swrite(p.e0,25,iRRAM_float_show)<< ")";
	o << "*[X]";
	o <<" i.e. "<< p.e0.vsize.mantissa<<"*2^" <<p.e0.vsize.exponent<<" +- "<< p.e0.error.mantissa<<"*2^" <<p.e0.error.exponent;
	for (const I &i : p.c) {
		o << "\n   + (" << swrite(i.ci,25,iRRAM_float_show)<< ")";
		o << "*["<< i.id << "]";
		o <<" i.e. "<< i.ci.vsize.mantissa<<"*2^" <<i.ci.vsize.exponent<<" +- "<< i.ci.error.mantissa<<"*2^" <<i.ci.error.exponent<< " refcount "<< i.es.use_count()<< " : "<< i.es->ref_count;
	}
	return o;
	}


}; // !class TM 

unsigned long long TM::max_id=0;
unsigned TM::default_sweep=1;
int TM::prec_diff=0;


} // !namespace iRRAM

#endif /* !  iRRAM_TaylorModel_H */
