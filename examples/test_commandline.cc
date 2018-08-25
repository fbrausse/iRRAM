
#include <iRRAM/lib.h>

using namespace iRRAM;

static int iRRAM_compute(const int & argc, char ** const & argv)
{
	REAL x1, x2;
	if (argc >= 2)
		x1 = REAL(argv[1]);
	if (argc >= 3)
		x2 = REAL(argv[2]);
	cout << x1 << "\n" << x2 << "\n";

	return 0;
}

int main(int argc, char ** argv)
{
	iRRAM_initialize2(&argc, argv);
	return iRRAM::exec(iRRAM_compute, argc, argv);
}
