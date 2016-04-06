
#include <iRRAM/TaylorModel.hh>

using namespace iRRAM;


unsigned long long TM::max_id=0;
unsigned TM::default_sweep=1;
int TM::prec_diff=0;
unsigned TM::sweepto = default_sweep;


void TM::round() {
	REAL e;
	my_to_formal_ball(to_real(),c0,e);
	c.clear();
	c.push_back(I({max_id++,e}));
	//  cerr << "#new max_id "<<max_id<<" (round)\n";
}


void TM::round0() {
	REAL s=c0,e;
	std::vector<I> cnew;
	if (c.size()>0){
		unsigned long long maxc=c[0].id;
		sizetype maxsize=c[0].ci.vsize;
		for (const I &i : c){
			if (sizetype_less(maxsize,i.ci.vsize)){
	maxc=i.id; maxsize=i.ci.vsize;
			}	     
		}  
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
		for (const I &i : c) {
			if (i.id==maxc){
	cnew.push_back(i);
			} else {
	s += i.ci * zero_one;
			}
		}
	}
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
//  cerr << "#new max_id "<<max_id<<" (round0)\n";
	c=cnew;
}


void TM::round1() {
	std::vector<I> cnew;
	unsigned long long minc= max_id+2;
	if (c.size()>0){
		minc=c[0].id;
		sizetype minsize=c[0].ci.vsize;
		for (const I &i : c){
			if (sizetype_less(i.ci.vsize,minsize)){
	minc=i.id; minsize=i.ci.vsize;
			}	     
		}  
	}
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
	REAL s=c0;
		for (const I &i : c) {
			if (i.id==minc){
	s += i.ci * zero_one;
			} else {
	cnew.push_back(i);
			}
		}
	REAL e;
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
//  cerr << "#new max_id "<<max_id<<" (round1)\n";
	c=cnew;
}


void TM::round2() {
	unsigned long long min1id=max_id+2;
	unsigned long long min2id=max_id+2;
	if (c.size()>1){
		min1id=c[0].id;
		min2id=c[1].id;
		sizetype min1size=c[0].ci.vsize;
		sizetype min2size=c[1].ci.vsize;
		if (sizetype_less(min2size,min1size)){
			min1id=c[1].id;
			min2id=c[0].id;
			min1size=c[1].ci.vsize;
			min2size=c[0].ci.vsize;
		}
		for (const I &i : c){
			if (sizetype_less(i.ci.vsize,min2size)){
				if (sizetype_less(i.ci.vsize,min1size)){
	min2id=min1id; min2size=min1size;
	min1id=i.id;   min1size=i.ci.vsize;
			}	else {
	min2id=i.id; min2size=i.ci.vsize;
			}
		}}
	}  
	std::vector<I> cnew;
	REAL s=c0;
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
	for (const I &i : c) {
		if ((i.id==min1id)||(i.id==min2id)){
			s += i.ci * zero_one;
		} else {
			cnew.push_back(i);
		}
	}
	REAL e;
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
	c=cnew;
//  cerr << "#new max_id "<<max_id<<" (round2)\n";
}


void TM::round5() {
	REAL s=c0,e;
	std::vector<I> cnew;
	if (c.size()>0){
		unsigned long long maxc=c[0].id;
		sizetype maxsize=c[0].ci.vsize;
		for (const I &i : c){
			if (sizetype_less(maxsize,i.ci.vsize)){
	maxc=i.id; maxsize=i.ci.vsize;
			}	     
		}
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
		for (const I &i : c) {
			if (i.id==maxc ){
	sizetype cis= i.ci.vsize;
	sizetype_shift(cis,cis,-30);
	if (sizetype_less(i.ci.error,cis)){
		cnew.push_back(i);
	} else {
		s += i.ci * zero_one;
	}
			} else {
	s += i.ci * zero_one;
			}
		}
	}
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
//  cerr << "#new max_id "<<max_id<<" (round0)\n";
	c=cnew;
}


void TM::round6() {
	REAL s=c0,e;
	std::vector<I> cnew;
	if (c.size()>0){
		unsigned long long maxc=0;
		sizetype maxsize;
		sizetype_exact(maxsize);
		for (const I &i : c){
			if (sizetype_less(maxsize,i.ci.vsize)){
	sizetype cis= i.ci.vsize;
	sizetype_shift(cis,cis,-30);
	if (sizetype_less(i.ci.error,cis)){
	maxc=i.id; maxsize=i.ci.vsize;
	}
			}	     
		}
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
		for (const I &i : c) {
			if (i.id==maxc ){
	sizetype cis= i.ci.vsize;
	sizetype_shift(cis,cis,-30);
	if (sizetype_less(i.ci.error,cis)){
		cnew.push_back(i);
	} else {
		s += i.ci * zero_one;
	}
			} else {
	s += i.ci * zero_one;
			}
		}
	}
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
//  cerr << "#new max_id "<<max_id<<" (round0)\n";
	c=cnew;
}


void TM::round7() {
	unsigned long long max1id=max_id+2;
	unsigned long long max2id=max_id+2;
	if (c.size()>1){
		sizetype max1size=c[0].ci.vsize;
		sizetype max2size=c[1].ci.vsize;
		if (sizetype_less(max1size,max2size)){
			max1size=c[1].ci.vsize;
			max2size=c[0].ci.vsize;
		}
		for (const I &i : c){
			sizetype cis= i.ci.vsize;
			sizetype_shift(cis,cis,-30);
			if (sizetype_less(i.ci.error,cis) && sizetype_less(max2size,i.ci.vsize)){
				if (sizetype_less(max1size,i.ci.vsize)){
	max2id=max1id; max2size=max1size;
	max1id=i.id;   max1size=i.ci.vsize;
			}	else {
	max2id=i.id; max2size=i.ci.vsize;
			}
		}}
	}  
	std::vector<I> cnew;
	REAL s=c0;
	REAL zero_one=0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);
		
		for (const I &i : c) {
			if ((i.id==max1id)||(i.id==max2id)){
	sizetype cis= i.ci.vsize;
	sizetype_shift(cis,cis,-30);
	if (sizetype_less(i.ci.error,cis)){
		cnew.push_back(i);
	} else {
		s += i.ci * zero_one;
	}
			} else {
	s += i.ci * zero_one;
			}
		}
	REAL e;
	my_to_formal_ball(s,c0,e);
	cnew.push_back(I({max_id++,e}));
	c=cnew;
//  cerr << "#new max_id "<<max_id<<" (round2)\n";
}




bool TM::test2() {
	if (c.size()>0){
		sizetype maxsize = c0.error;
		for (unsigned i=0; i<c.size(); i++) {
			if (sizetype_less(maxsize,c[i].ci.vsize))
				return false;
		}
	} else
		return false;
	return true;
}

bool TM::test3() {
	if (c.size()>0) {
		sizetype minsize;
		sizetype_shift(minsize,c0.error,30);
		for (unsigned i=0; i<c.size(); i++)
			if (sizetype_less(c[i].ci.vsize,minsize))
				return true;
	} 
	return false;  
}

bool TM::test4() {
	if (c.size() > 0) {
		sizetype errorsum = c[0].ci.vsize;
		for (unsigned i=1;i< c.size();i++)
			sizetype_inc(errorsum, c[i].ci.vsize);
		return sizetype_less(errorsum,c0.error);
	};
	return true;
}



bool TM::test5() {
	if (c.size()>0){
		sizetype errorsum;
		sizetype_exact(errorsum);
		for (unsigned i=0;i< c.size();i++){
			 sizetype cis=c[i].ci.vsize;
			 sizetype_inc(errorsum,cis);
			 sizetype_shift(cis,cis,-20);
			 if (sizetype_less(cis,c[i].ci.error)) {
	 cerr << "large error! "<< c[i].id<<"\n"; 
	 return true;
			} 
		}
	return sizetype_less(errorsum,c0.error); 
	};
	return true;
}

bool TM::test6() {
	if (c.size()>0){
		sizetype errorsum;
		sizetype_exact(errorsum);
		for (unsigned i=0;i< c.size();i++){
			 sizetype cis=c[i].ci.vsize;
			 sizetype_inc(errorsum,cis);
			 sizetype_shift(cis,cis,-20);
			if (sizetype_less(cis,c[i].ci.error)) {
	 cerr << "large error! "<< c[i].id<<"\n"; 
	 return true;
			} 
		}
	sizetype_shift(errorsum,errorsum,-15);  
	return sizetype_less(errorsum,c0.error); 
	};
	return true;
}

bool TM::test7() {
	if (c.size()>0){
		sizetype c0e=c0.error;
		sizetype_shift(c0e,c0e,10);  
		for (unsigned i=0;i< c.size();i++){
			 sizetype cis=c[i].ci.vsize;
			 if (sizetype_less(cis,c0e)){
	 cerr << "compare polish: "<< c[i].id<<"\n"; 
	 return true;
			 }
			 sizetype_shift(cis,cis,-25);  
			 if (sizetype_less(cis,c[i].ci.error)) {
	 cerr << "large error polish: "<< c[i].id<<"\n"; 
	 return true;
			} 
		}
		return false;
	};
	return true;
}


void TM::check() {
	unsigned minc=0;
	if (c.size()>0) {
		sizetype maxsize = c0.vsize;
		sizetype minsize = c[0].ci.vsize;
		for (unsigned i=0; i<c.size(); i++) {
			if (sizetype_less(maxsize,c[i].ci.vsize))
				return;
			if (sizetype_less(c[i].ci.vsize,minsize)) {
				minc    = i;
				minsize = c[i].ci.vsize;
			}
		}
	} else
		return;

	REAL zero_one = 0;
	sizetype l;
	sizetype_set(l,1,0);
	zero_one.seterror(l);

	REAL e;
	my_to_formal_ball(c0+c[minc].ci*zero_one,c0,e);
	c[minc].id = max_id++;
	c[minc].ci = e;
//  cerr << "max_id "<<max_id<<" (check)\n";
}

namespace iRRAM {

TM & TM::operator+=(const TM &tm)
{
	{ stiff code(prec_diff); c0+=tm.c0; }
	for (const I &i : tm.c) {
		for (I &j : c) {
			if (i.id == j.id) {
				j.ci += i.ci;
				goto ok;
			}
		}
		c.push_back(i);
ok:;
	}
	return *this;
}

TM & TM::operator-=(const TM &tm)
{
	{ stiff code(prec_diff); c0 -= tm.c0; }
	for (const I &i : tm.c) {
		for (I &j : c) {
			if (i.id == j.id) {
				j.ci -= i.ci;
				goto ok;
			}
		}
		c.emplace_back(i.id,-i.ci);
ok:;
	}
	return *this;
}

TM & TM::operator*=(const REAL &r)
{
	{ stiff code(prec_diff); c0 *=r; }
	for (I &l : c)
			l.ci *= r;
	return *this;
}

TM operator*(TM q, const TM &r)
{
	REAL q_real = q.to_real();
	TM &f = q*=r.c0;
	if (TM::sweepto == 0) {
		for (const TM::I &i : r.c)
			f.c0 += q_real*i.ci;
	} else {
		TM g(REAL(0));
		for (const TM::I &i : r.c)
			g.c.emplace_back(i.id,q_real*i.ci);
		f += g;
	}
	return f;
}

#if 1
TM inverse(TM r)
{
	/* r =: r0 + [ri] */

	sizetype domain_of_inverse_f;
	static_cast<REAL>(r).geterror(domain_of_inverse_f);

	REAL r0_inv = 1/r.c0;
	r.c0 = 0;
	r *= r0_inv;           /*  [ri]/r0     */

	TM q2 = square(r);     /* ([ri]/r0)^2  */
	r.c0 = -1;
	r *= -r0_inv;          /* (1-[ri]/r0)/r0 */

	/* taylor sum: r * \sum_{j=0}^\infty ([ri]/r0)^{2*j} */
	/* development point: r0, evaluated on r0+[ri] = domain_pf_inverse_f */

	const unsigned N = 10; /* TODO */

	TM q = q2;             /* ([ri]/r0)^{2*j} */
	TM s(1);               /* \sum for j=0 */
	s += q;                /*      for j=1 */

	/* Geometric series with ratio c=([ri]/r0)^2 = (1-c^N)/(1-c),
	 * truncation error therefore: c^N/(1-c) */
	for (unsigned j=2; j<N; j++)
		s += q *= q2;

	/* TODO: add truncation error */

	return r * s;
}
#endif

}
