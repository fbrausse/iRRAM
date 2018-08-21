
#include <unistd.h>
#include <thread>
#include <future>

#include <iRRAM/lib.h>

using namespace iRRAM;

using std::string;

REAL f(const REAL & x) { return x + 1; }

class my_arg_type
{
public:
	string s;
	long long n;
};

double compute(const my_arg_type & arg)
{
	REAL x(arg.s); // with no automatic conversion as a std::string may also
	               // represent a COMPLEX
	for (long long i = 0; i < arg.n; i++) {
		x = f(x);
	}
	return x.as_double(); // now with explicit conversion
}

int main(int argc, char ** argv)
{
	iRRAM_initialize(argc, argv);
	cout << "Starting an example with two threads...\n";

	std::future<double> t_1 =
	        std::async(std::launch::async, &iRRAM::exec<double()>, [] {
		        return compute({"0.3456789", 345678});
	        });
	std::future<double> t_2 =
	        std::async(std::launch::async, &iRRAM::exec<double()>, [] {
		        return compute({"0.9876543", 987654});
	        });

	// just do something else or just wait...
	// but don't touch the result variables!
	// Here we just print, whether the process are already finished...
	cout << "Threads are running now\n"
	     << "They will finish in a few seconds...\n";
	for (int i = 1; i <= 10; i++) {
		cout << "Ready: "
		     << (t_1.wait_for(std::chrono::duration<int>()) ==
		         std::future_status::ready)
		     << (t_2.wait_for(std::chrono::duration<int>()) ==
		         std::future_status::ready)
		     << "\n";
		usleep(200000);
	}

	// now print the results
	cout << std::setprecision(17);

	double d_1 = t_1.get();
	cout << "First result:  " << d_1 << "\n";

	double d_2 = t_2.get();
	cout << "Second result: " << d_2 << "\n";
}
