#include "iRRAM.h"

using namespace iRRAM;

using std::setw;

/* Compute iterated system x=3.75*x*(1-x) (Kulisch) */

void compute()
{
	int count;
	cout << "\nIterated functions system: x = 3.75*x*(1-x)\n";
	cout << "How many values: ";
	cin >> count;
	cout << "\n";

	REAL xr = 0.5, cr = 3.75;
	double xd = 0.5, cd = 3.75;
	cout << setw(20);
	for (long i = 1; i <= count; i++) {
		cout << setRflags(float_form::absolute) << xr << " ";
		cout << xd << " ";
		cout << setRflags(float_form::show) << xr - REAL(xd);
		cout << " " << i << "\n";
		xr = cr * xr * (1 - xr);
		xd = cd * xd * (1 - xd);
	}
}
