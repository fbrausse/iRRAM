/*

REALmain.cc -- implementation of main initialization routine
               for the iRRAM library
 
Copyright (C) 2003-2008 Norbert Mueller
 
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
#include <cmath>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <vector>

#include <cfenv>

#include <iRRAM/lib.h>


namespace iRRAM {


/* for debugging (time measuring):*/
#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#include <psapi.h>
void resources(double &time, unsigned int &memory)
{
	FILETIME creation_time, exit_time, kernel_time, user_time;
	if (GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time, &kernel_time, &user_time)) {
		time = ((kernel_time.dwLowDateTime + ((uint64_t)kernel_time.dwHighDateTime << 32))
		        + (user_time.dwLowDateTime + ((uint64_t)  user_time.dwHighDateTime << 32))
		       ) / 1e7;
	} else {
		time = 0;
	}

	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		memory = pmc.WorkingSetSize;
	} else {
		memory = 0;
	}
}
#else
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

void resources(double& time, unsigned int& memory){
 struct rusage r;
 getrusage (RUSAGE_SELF,&r);
 time = r.ru_utime.tv_sec+0.000001*r.ru_utime.tv_usec;
// the following is not yet evaluated by linux.... 
 memory = r.ru_ixrss+r.ru_idrss+r.ru_isrss;
}
#endif
/* end of debugging aids */

void show_statistics()
{
  cerr << "   MP-objects in use:  "<<MP_var_count<<"\n"; 
#ifdef	MP_space_count
  cerr << "   MP-memory in use:   "<<MP_space_count<<"\n"; 
  cerr << "   max MP-memory used: "<<MP_max_space_count<<"\n"; 
#endif
  cerr << "   total alloc'ed MPFR: "
       << state->ext_mpfr_cache.total_alloc_var_count << "\n";
  cerr << "   total free'd   MPFR: "
       << state->ext_mpfr_cache.total_freed_var_count << "\n";
  double time;
  unsigned int memory;
  resources(time,memory);
  cerr << "   CPU-Time for ln2:   "<<ln2_time<<" s\n";
  cerr << "   CPU-Time for pi:    "<<pi_time<<" s\n";
  cerr << "   total CPU-Time:     "<<time<<" s\n";
  //cerr << "   total Memory:       "<<memory/1024<<" KB\n";
  if (actual_stack().prec_step != iRRAM_DEFAULT_PREC_START)
    cerr << "   basic precision:    "<<actual_stack().actual_prec
		<<"["<<actual_stack().prec_step<<"]\n"; 
  else 
  cerr << "   basic precision:    double\n"; 
  if ( state->max_prec != 1) 
    cerr << "   maximal precision:  "<<iRRAM_prec_array[state->max_prec]
		<<"["<<state->max_prec<<"]\n"; 
  else 
    cerr << "   maximal precision:  double\n"; 
}

internal::run::run(state_t &st)
: st(st)
, code(st.prec_start, stiff::abs{})
{
	ITERATION_DATA &actual_stack = st.ACTUAL_STACK;

	if (iRRAM_unlikely(st.debug > 0)) {
//		std::stringstream s;
//		s << std::this_thread::get_id();
		cerr << "\niRRAM (version " << iRRAM_VERSION_rt
		     << ", backend " << iRRAM_BACKENDS << ")"
//		     << " thread " << s.str()
		     << " starting...\n";
	}

	st.cache_address = new mv_cache;

	// set the correct rounding mode for REAL using double intervals):
	fesetround(FE_DOWNWARD);

	st.cache_active = new cachelist;

	if (iRRAM_unlikely(st.debug > 0))
		st.max_prec = actual_stack.prec_step;

	actual_stack.prec_policy = 1;
	actual_stack.inlimit = 0;
	st.highlevel = (actual_stack.prec_step > iRRAM_DEFAULT_PREC_START);
}

void internal::run::loop_init()
{
	ITERATION_DATA &actual_stack = st.ACTUAL_STACK;
	iRRAM::cout.rewind();
	for (int n = 0; n < st.max_active; n++)
		st.cache_active->id[n]->rewind();

	st.inReiterate = false;
	assert(actual_stack.inlimit == 0);
	assert(st.highlevel == (actual_stack.prec_step > iRRAM_DEFAULT_PREC_START));
}

void internal::run::loop_fini(int p_end)
{
	ITERATION_DATA &actual_stack = st.ACTUAL_STACK;
	assert(st.highlevel == (actual_stack.prec_step > iRRAM_DEFAULT_PREC_START));

	int prec_skip = 0;
	do {
		prec_skip++;
		code.inc_step(4);
	} while ((actual_stack.actual_prec > p_end) &&
		 (prec_skip != st.prec_skip));

	assert(actual_stack.inlimit == 0);

	if (iRRAM_unlikely(st.debug > 0)) {
		show_statistics();
		if (st.max_prec <= actual_stack.prec_step)
			st.max_prec = actual_stack.prec_step;
		cerr << "increasing precision bound to "
		     << actual_stack.actual_prec << "["
		     << actual_stack.prec_step << "]\n";
	}
}

internal::run::~run()
{
	iRRAM::cout.reset();
	for (int n = 0; n < st.max_active; n++)
		st.cache_active->id[n]->clear();

	st.max_active = 0;
	delete st.cache_active;
	delete st.cache_address;

	if (iRRAM_unlikely(st.debug > 0)) {
		show_statistics();
		cerr << "iRRAM ending \n";
	}
}

} // namespace iRRAM

extern "C" int iRRAM_exec(void (*f)(void *), void *data)
{
	using namespace iRRAM;
	try {
		return exec([f,data](){ f(data); return iRRAM_success; });
	} catch (const iRRAM_Numerical_Exception &ex) {
		return ex.type;
	}
}

extern "C" int iRRAM_parse_args(struct iRRAM_init_options *opts, int *argc, char **argv)
{
	auto debug_enabled = [&opts](int level){ return opts->debug >= level; };

	for (int i = 1; i < *argc; i += 1) {
		if (!strcmp(argv[i], "-d")) {
			opts->debug = 1;
			iRRAM_DEBUG2(1, "Debugging Mode\n");
		} else
		if (!strncmp(argv[i], "--debug=", 8)) {
			opts->debug = atoi(&(argv[i][8]));
			iRRAM_DEBUG2(1, "Debugging Level %d\n", opts->debug);
		} else
		if (!strncmp(argv[i], "--prec_init=", 12)) {
			opts->starting_prec = atoi(&(argv[i][12]));
			iRRAM_DEBUG2(1, "Initialising precision to 2^(%d)\n",
			             opts->starting_prec);
		} else
		if (!strncmp(argv[i], "--prec_inc=", 11)) {
			int hi;
			hi = atoi(&(argv[i][11]));
			if (hi > 0)
				opts->prec_inc = -hi;
			iRRAM_DEBUG2(
			        1,
			        "Initialising precision increment to %d bits\n",
			        -opts->prec_inc);
		} else
		if (!strncmp(argv[i], "--prec_factor=", 14)) {
			double hd;
			hd = atof(&(argv[i][14]));
			if (hd > 1.0)
				opts->prec_factor = hd;
			iRRAM_DEBUG2(1, "Initialising precision factor to %f\n",
			             opts->prec_factor);
		} else
		if (!strncmp(argv[i], "--prec_skip=", 12)) {
			int hi;
			hi = atoi(&(argv[i][12]));
			if (hi > 0)
				opts->prec_skip = hi;
			iRRAM_DEBUG2(1, "Changed heuristic for precision "
			                "changes to skip at most %d steps\n",
			             opts->prec_skip);
		} else
		if (!strncmp(argv[i], "--prec_start=", 13)) {
			int hi;
			hi = atoi(&(argv[i][13]));
			if (hi > 0)
				opts->prec_start = hi;
			iRRAM_DEBUG2(1,
			             "Changed inital precision step to %d \n",
			             opts->prec_start);
		} else
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			fprintf(stderr,
"Runtime parameters for the iRRAM library:\n"
"--prec_init=n   [%4d] starting precision\n"
"--prec_inc=n    [%4d] basic increment for precision changes\n"
"--prec_factor=x [%4g] basic factor for precision changes\n"
"--prec_skip=n   [%4d] bound for precision increments skipped by heuristic\n"
"--prec_start=n  [%4d] initial precision level\n"
"--debug=n       [%4d] level of limits up to which debugging should happen\n"
"-d                     debug mode, with level 1\n"
"-h / --help            this help message\n",
			        opts->starting_prec,
			        opts->prec_inc,
			        opts->prec_factor,
			        opts->prec_skip,
			        opts->prec_start,
			        opts->debug);
		} else
			continue;
		/* argument handled, remove from list */
		for (int j = i+1; j < *argc; j++)
			argv[j-1] = argv[j];
		argv[--*argc] = NULL;
		--i;
	}
	return iRRAM_success;
}

static void init_prec_array(int (&prec_array)[iRRAM_prec_steps],
                            int starting_prec, int _prec_inc,
                            double prec_factor)
{
	using namespace iRRAM;

	prec_array[0] = 2100000000;
	prec_array[1] = starting_prec;
	double factor = std::sqrt(std::sqrt(prec_factor));
	int prec_inc = _prec_inc;
	if (state->debug)
		cerr << "Basic precision bounds: "
		     << "double[1]";
	for (int i = 2; i < iRRAM_prec_steps; i++) {
		prec_array[i] = starting_prec + prec_inc;
		prec_inc = int(prec_inc * factor) + _prec_inc;
		if (prec_array[i] >= prec_array[i - 1])
			prec_array[i] = prec_array[i - 1];
		else if (state->debug && ((i % 5 == 0) || (i < 10)))
			cerr << " " << prec_array[i] << "[" << i << "]";
	}
	if (state->debug)
		cerr << "\n";
}

extern "C" void iRRAM_initialize3(const struct iRRAM_init_options *opts)
{
	using namespace iRRAM;

	state->debug = opts->debug;
	state->prec_skip = opts->prec_skip;
	state->prec_start = opts->prec_start;

	MP_initialize;
	init_prec_array(state->prec_array, opts->starting_prec, opts->prec_inc,
	                opts->prec_factor);
}

extern "C" void iRRAM_initialize2(int *argc, char **argv)
{
	struct iRRAM_init_options opts = iRRAM_INIT_OPTIONS_INIT;
	iRRAM_parse_args(&opts, argc, argv);
	iRRAM_initialize3(&opts);
}

/* API compatibility with versions <= 2014.01 */
extern "C" void iRRAM_initialize(int argc, char **argv)
{
	std::vector<char *> args(argv, argv+argc+1);
	iRRAM_initialize2(&argc, &args[0]);
}

extern "C" void iRRAM_finalize()
{
	using namespace iRRAM;
	MP_finalize;
}
