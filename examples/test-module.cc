
#include <iRRAM.h>

using namespace iRRAM;

REAL f(REAL x, REAL y) { return x*y; }

void compute()
{
	REAL x, y;
	int p;
	cout << "enter x: "; cin >> x;
	cout << "enter y: "; cin >> y;
	cout << "enter p: "; cin >> p;
	cout << "\n";
	cout << "modulus(multiplication, " << p << ", " << x << ", " << y
	     << ") = " << modulus(f, p, x, y) << "\n";
	cout << "module(square, " << x << ", " << p
	     << ") = " << module([](const REAL &x){ return x*x; }, x, p)
	     << "\n";
}
