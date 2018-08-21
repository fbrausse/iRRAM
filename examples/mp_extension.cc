#include "iRRAM/lib.h"  // basic includes
#include "iRRAM/core.h" // additional includes for templates

using namespace iRRAM;

// iRRAM-routine to compute a DYADIC cosine
DYADIC compute_cos(const DYADIC & x, const int & p)
{
	return approx(cos(REAL(x)), p);
}

// main routine that internally calls iRRAM three times:
int main(int argc, char ** argv)
{
	iRRAM_initialize(argc, argv);
	DYADIC d = 2.0, d2;
	int p = -10;

	auto compute = [&]() { return compute_cos(d, p); };
	d2 = iRRAM::exec(compute);

	cout << setRwidth(100);
	cout << iRRAM::exec(compute) << "\n";
}
