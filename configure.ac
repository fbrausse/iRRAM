
dnl Process this file with autoconf to produce a configure script.
AC_INIT([iRRAM],[2014_01],[],[iRRAM],[irram.uni-trier.de])

iRRAM_VERSION=2014_01
iRRAM_shared_version=14:1:0
AC_SUBST(iRRAM_VERSION)
AC_SUBST(iRRAM_shared_version)

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([iRRAM_VERSION.in])
AC_CANONICAL_HOST

GMPLDFLAGS=
GMPCPPFLAGS=
MPFRLDFLAGS=
MPFRCPPFLAGS=

dnl TODO: should be AC_ARG_ENABLE
AC_ARG_WITH([backends],
	[AS_HELP_STRING([--with-backends=MP],[additional backends for MP arithmetic @<:@available: MPFR, default: MPFR@:>@])],
	[with_backends=$withval])
AC_ARG_WITH([gmp],
	[AS_HELP_STRING([--with-gmp=DIR],[GMP install directory])],
	[GMPCPPFLAGS="-I$withval/include"
	 GMPLDFLAGS="-L$withval/lib"])
AC_ARG_WITH([gmp-include],
	[AS_HELP_STRING([--with-gmp-include=DIR],[GMP include directory])],
	[GMPCPPFLAGS="-I$withval"])
AC_ARG_WITH([gmp-lib],
	[AS_HELP_STRING([--with-gmp-lib=DIR],[GMP lib directory])],
	[GMPLDFLAGS="-L$withval"])
AC_ARG_WITH([mpfr],
	[AS_HELP_STRING([--with-mfpr=DIR],[MPFR install directory])],
	[MPFRCPPFLAGS="-I$withval/include"
	 MPFRLDFLAGS="-L$withval/lib"])
AC_ARG_WITH([mpfr-include],
	[AS_HELP_STRING([--with-mpfr-include=DIR],[MPFR include directory])],
	[MPFRCPPFLAGS="-I$withval"])
AC_ARG_WITH([mpfr-lib],
	[AS_HELP_STRING([--with-mpfr-lib=DIR],[MPFR lib directory])],
	[MPFRLDFLAGS="-L$withval"])
AC_ARG_WITH([flint],
	[AS_HELP_STRING([--with-flint=DIR],[Flint install directory])],
	[FLINTCPPFLAGS="-I$withval/include"
	 FLINTLDFLAGS="-L$withval/lib"])
AC_ARG_WITH([flint-include],
	[AS_HELP_STRING([--with-flint-include=DIR],[Flint include directory])],
	[FLINTCPPFLAGS="-I$withval"])
AC_ARG_WITH([flint-lib],
	[AS_HELP_STRING([--with-flint-lib=DIR],[Flint lib directory])],
	[FLINTLDFLAGS="-L$withval"])

dnl Profiling
AC_ARG_ENABLE([profiling],
	[AS_HELP_STRING([--enable-profiling],[build with profiler support @<:@default=no@:>@])])

dnl Threading
dnl missing: lang, pthread, win
AC_ARG_ENABLE([tls],
  [AS_HELP_STRING([[--disable-tls[=ARG]]],
    [select thread-local storage support @<:@default: enable if supported, possible: yes, no, KEYWORD@:>@])],,[enable_tls=detect])

AC_ARG_ENABLE([flint-integer],
	[AS_HELP_STRING([[--enable-flint-integer]],[Use FLINT as backend for iRRAM::INTEGER])],,
	[enable_flint_integer=no])

AM_INIT_AUTOMAKE([nostdinc -Wall subdir-objects])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

dnl Checks for programs.
AC_PROG_MAKE_SET
AM_PROG_AR

LT_INIT([win32-dll])
AC_SUBST(LIBTOOL)
AC_SUBST(LIBTOOL_DEPS)
AC_SUBST(LD)

# these macros (may) modify *FLAGS, so we'll save those
save_CFLAGS=$CFLAGS
save_CXXFLAGS=$CXXFLAGS
CFLAGS=
CXXFLAGS=
AC_PROG_CC_C99
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([14],[noext])
AM_CFLAGS="$AM_CFLAGS $CFLAGS"
AM_CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
CFLAGS=$save_CFLAGS
CXXFLAGS=$save_CXXFLAGS
AS_IF([test -z "$CFLAGS"],[CFLAGS="-g -O2"])
AS_IF([test -z "$CXXFLAGS"],[CXXFLAGS="-g -O2"])

dnl ----------------------------------------------------------------------------
dnl check thread-local support
dnl ----------------------------------------------------------------------------
tls=
AS_CASE(["$enable_tls"],
  [no],[],
  [detect|yes],[
    save_CFLAGS=$CFLAGS
    save_CXXFLAGS=$CXXFLAGS
    save_LIBS=$LIBS

    CFLAGS="$AM_CFLAGS $CFLAGS"
    CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
    LIBS="$LDADD $LIBS"

    IRRAM_TLS

    CFLAGS=$save_CFLAGS
    CXXFLAGS=$save_CXXFLAGS
    LIBS=$save_LIBS

    tls=$irram_tls
  ],
  [tls=$enable_tls])
AS_IF([test "$tls" = thread_local],[tls_std=1],[tls_std=0])

AS_IF([test -z "$tls"],
  [AS_CASE([x$enable_tls],
     [xyes],[AC_MSG_ERROR([TLS requested, but could not detect any supported thread-local storage class specifier])],
     [xdetect],[AC_MSG_WARN([could not detect any supported thread-local storage class specifier; disabling thread support])])
   AC_SUBST([iRRAM_HAVE_TLS],[0])],
  [AC_MSG_NOTICE([using $tls as thread-local storage class specifier])
   AC_SUBST([iRRAM_HAVE_TLS],[1])
   AC_LANG_PUSH([C++])
   AX_CHECK_COMPILE_FLAG([-fno-extern-tls-init],
     [AM_CXXFLAGS="$AM_CXXFLAGS -fno-extern-tls-init"],
     [AC_MSG_NOTICE([C++ compiler does not support -fno-extern-tls-init -> slower TLS-access])])
   AC_LANG_POP([C++])])
AC_SUBST([iRRAM_TLS],[$tls])
AC_SUBST([iRRAM_TLS_STD],[$tls_std])


dnl ----------------------------------------------------------------------------
dnl check profiling support
dnl ----------------------------------------------------------------------------
AS_CASE(["$enable_profiling"],[yes|gprof],[
   AC_LANG_PUSH([C])
   AX_CHECK_COMPILE_FLAG([-pg],
     [PROFILING_CFLAGS=-pg]
     [AX_CHECK_COMPILE_FLAG([-fno-inline],
       [PROFILING_CFLAGS="$PROFILING_CFLAGS -fno-inline"],
       [AC_MSG_WARN([profiling requested, but cannot disable inlining on $CC])],
       [$PROFILING_CFLAGS])])
   AC_LANG_POP([C])

   AS_IF([test -n "$PROFILING_CFLAGS"],[
      AC_LANG_PUSH([C++])
      AX_CHECK_COMPILE_FLAG([-pg],
        [PROFILING_CXXFLAGS=-pg]
        [AX_CHECK_COMPILE_FLAG([-fno-inline],
          [PROFILING_CXXFLAGS="$PROFILING_CXXFLAGS -fno-inline"],
          [AC_MSG_WARN([profiling requested, but cannot disable inlining on $CXX])],
          [$PROFILING_CXXFLAGS])])
      AC_LANG_POP([C++])])

   AS_IF([test -n "$PROFILING_CFLAGS" -a "$PROFILING_CXXFLAGS" = "$PROFILING_CFLAGS"],
     [AX_CHECK_LINK_FLAG([$PROFILING_CFLAGS],
       [AM_CFLAGS="$AM_CFLAGS $PROFILING_CFLAGS"]
       [AM_CXXFLAGS="$AM_CXXFLAGS $PROFILING_CFLAGS"]
       [AM_LDFLAGS="$AM_LDFLAGS $PROFILING_CFLAGS"],
       [AC_MSG_ERROR([linker does not support profiling flags $PROFILING_CFLAGS])])],
     [AC_MSG_ERROR([profiling requested but not supported])])
])

dnl ----------------------------------------------------------------------------
dnl use some warning flags if supported
dnl ----------------------------------------------------------------------------
AC_LANG_PUSH([C])
WARN_CFLAGS=
AX_CHECK_COMPILE_FLAG([-Wall],
  [WARN_CFLAGS="-Wall"]
  [AX_CHECK_COMPILE_FLAG([-Wextra],[WARN_CFLAGS="$WARN_CFLAGS -Wextra"],,[$WARN_CFLAGS])]
  [AX_CHECK_COMPILE_FLAG([-Wmissing-prototypes],[WARN_CFLAGS="$WARN_CFLAGS -Wmissing-prototypes"],,[$WARN_CFLAGS])]
  [AX_CHECK_COMPILE_FLAG([-pedantic],[WARN_CFLAGS="$WARN_CFLAGS -pedantic"],,[$WARN_CFLAGS])])
AM_CFLAGS="$AM_CFLAGS $WARN_CFLAGS"
AC_LANG_POP([C])

AC_LANG_PUSH([C++])
WARN_CXXFLAGS=
AX_CHECK_COMPILE_FLAG([-Wall],
  [WARN_CXXFLAGS="-Wall"]
  [AX_CHECK_COMPILE_FLAG([-Wextra],[WARN_CXXFLAGS="$WARN_CXXFLAGS -Wextra"],,[$WARN_CFLAGS])]
  [AX_CHECK_COMPILE_FLAG([-Wmissing-declarations],[WARN_CXXFLAGS="$WARN_CXXFLAGS -Wmissing-declarations"],,[$WARN_CXXFLAGS])]
  [AX_CHECK_COMPILE_FLAG([-pedantic],[WARN_CXXFLAGS="$WARN_CXXFLAGS -pedantic"],,[$WARN_CXXFLAGS])])
AM_CXXFLAGS="$AM_CXXFLAGS $WARN_CXXFLAGS"
AC_LANG_POP([C++])

dnl ----------------------------------------------------------------------------

AC_LANG([C++])

AS_CASE([$host],
	[*-mingw*],[
	dnl Use ANSI printf modifiers in mingw32.
	dnl See <http://sourceforge.net/p/mingw-w64/wiki2/gnu%20printf/>
	AM_CPPFLAGS="$AM_CPPFLAGS -D__USE_MINGW_ANSI_STDIO=1 -DPSAPI_VERSION=1"

	dnl Do not use dynamic linking on Windows
	#AM_LDFLAGS="-all-static $AM_LDFLAGS"

	save_CPPFLAGS=$CPPFLAGS
	CPPFLAGS="$AM_CPPFLAGS $CPPFLAGS"
	AC_CHECK_HEADERS([windows.h psapi.h],,[AC_MSG_ERROR([required Windows system headers not found])],[[
#ifdef HAVE_WINDOWS_H
# include <windows.h>
#endif]])

	save_LIBS=$LIBS
	LIBS=-lpsapi
	AC_CHECK_DECL([GetProcessMemoryInfo(HANDLE,PPROCESS_MEMORY_COUNTERS,DWORD)],,
	  [AC_MSG_ERROR([unable to find library containing GetProcessMemoryInfo().])],
	  [[
#include <windows.h>
#include <psapi.h>
	  ]])
	LIBS_INST="$LIBS_INST $LIBS"
	LIBS="$LDADD $save_LIBS $LIBS_INST"

	CPPFLAGS=$save_CPPFLAGS
])

dnl Test backends

AM_CPPFLAGS="$AM_CPPFLAGS $GMPCPPFLAGS $MPFRCPPFLAGS $FLINTCPPFLAGS"
AM_LDFLAGS="$AM_LDFLAGS $GMPLDFLAGS $MPFRLDFLAGS $FLINTLDFLAGS"

CPPFLAGS_INST="$AM_CPPFLAGS $CPPFLAGS"

save_CPPFLAGS="$CPPFLAGS"
save_CFLAGS="$CFLAGS"
save_CXXFLAGS="$CXXFLAGS"
save_LDFLAGS="$LDFLAGS"
CPPFLAGS="$AM_CPPFLAGS $CPPFLAGS"
CFLAGS="$AM_CFLAGS $CFLAGS"
CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
LDFLAGS="$AM_LDFLAGS $LDFLAGS"

AC_SEARCH_LIBS([sin],[m],[AS_CASE([$ac_cv_search_sin],["none required"],[],[LIBS_INST="$ac_cv_search_sin"])])

LIBS="$LDADD $LIBS_INST"

AS_BOX([Checking GMP headers/lib...])
AC_CHECK_HEADER(gmp.h,,[AC_MSG_ERROR([GNU Multiple Precision Arithmetic library (<https://gmplib.org/>) is required.])])

IRRAM_GMP_VERSION_CHECK(4,1,0)

LIBS="-lgmp $LIBS"

AC_DEFUN([gmp_link_check],[
	AC_LANG_PUSH([$1])
	AC_MSG_CHECKING([whether $1-linking against GMP works])
	AC_LINK_IFELSE([AC_LANG_SOURCE([[
#ifdef __cplusplus
# include <cstddef> /* MinGW's GMP header doesn't include this but uses ::max_align_t */
#endif
#include <gmp.h>
int main() {
	mpz_init(0);
	return 0;
}]])],[AC_MSG_RESULT([yes])],[AC_MSG_ERROR([$1-linking to GMP failed.])])
	AC_LANG_POP([$1])
])
gmp_link_check([C])
gmp_link_check([C++])
AC_SUBST([iRRAM_HAVE_GMP_C],1)

LIBS_INST="-lgmp $LIBS_INST"
LIBS="$LDADD $LIBS_INST"


AS_BOX([Checking FLINT fmpz headers/lib...])
AC_CHECK_HEADERS([flint/fmpz.h],
	[AC_SEARCH_LIBS([flint_cleanup],[flint],
		[have_flint_integer=yes])])
AS_IF([test x$have_flint_integer = xyes],,
	[AS_IF([test x$enable_flint_integer = xyes],
		[AC_MSG_ERROR([error: --enable-flint-integer=$enable_flint_integer requested, but FLINT library not found])])
	 enable_flint_integer=no])
AS_IF([test x$enable_flint_integer = xyes],
	[AC_MSG_NOTICE([Linking iRRAM against FLINT])
	 AS_IF([test "$ac_cv_search_flint_cleanup" != "none required"],
		[LIBS_INST="$ac_cv_search_flint_cleanup $LIBS_INST"
		 LIBS="$LDADD $LIBS_INST"])],
	[AC_MSG_NOTICE([Not linking iRRAM against FLINT])])
AM_CONDITIONAL([ENABLE_FLINT_INTEGER],[test x$enable_flint_integer = xyes])

# TODO: also preferred for MPFR: mpfr_get_prec(x) and mpfr_get_exp(x) as macros

with_backends=`echo "$with_backends" | $SED 's#,\+# #g' | LC_ALL=C tr 'a-z' 'A-Z'`
with_backends=`echo $with_backends`
if test -z "$with_backends"; then
	AC_MSG_NOTICE([No backend given, defaulting to MPFR backend])
	with_backends="MPFR"
fi
iRRAM_BACKEND_MPFR=0
BACKENDS=""
for be in $with_backends; do
	if test $be == MPFR; then
		if test $iRRAM_BACKEND_MPFR -ne 0; then
			continue
		fi
		AS_BOX([Checking MPFR headers/lib])
		AC_CHECK_HEADER([mpfr.h],[],
			[AC_MSG_ERROR([MPFR headers not found! You need to install MPFR first!],101)])
		AC_CHECK_LIB([mpfr],[mpfr_get_version],
			[LIBS_INST="-lmpfr $LIBS_INST"],
			[AC_MSG_ERROR([MPFR library not usable! Please check your installation of MPFR.],101)])
		LIBS="$LDADD $LIBS_INST"
		AC_MSG_RESULT([MPFR backend: found])
		EXTRAEXAMPLES="$EXTRAEXAMPLES timings-MPFR-iRRAM test-MPFR-iRRAM"
		iRRAM_BACKEND_MPFR=1
		BACKENDS="$BACKENDS MPFR"
	else
		AC_MSG_ERROR([Backend '$be' is unavailable!])
	fi
done
BACKENDS=`echo $BACKENDS`
AC_SUBST(iRRAM_BACKEND_MPFR)
AM_CONDITIONAL([WITH_BACKEND_MPFR],[test x$iRRAM_BACKEND_MPFR = x1])
AC_SUBST(iRRAM_BACKENDS,[`echo ${BACKENDS} | tr ' ' ,`])

CPPFLAGS="$save_CPPFLAGS"
CFLAGS="$save_CFLAGS"
CXXFLAGS="$save_CXXFLAGS"
LDFLAGS="$save_LDFLAGS"

AM_CPPFLAGS="$AM_CPPFLAGS -I\$(top_srcdir)/include -I\$(top_builddir)/include"

AC_SUBST(CPPFLAGS_INST)
AC_SUBST(AM_CPPFLAGS)
AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CXXFLAGS)
AC_SUBST(LDADD)
AC_SUBST(AM_LDFLAGS)
AC_SUBST(LIBS_INST)
AC_SUBST(RPATH)

dnl These are always set automatically
#AC_SUBST(CPPFLAGS)
#AC_SUBST(CFLAGS)
#AC_SUBST(CXXFLAGS)
#AC_SUBST(LDFLAGS)
#AC_SUBST(LIBS)

AC_SUBST(EXTRAEXAMPLES)

MAKEFILES=""

AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([iRRAM_VERSION include/iRRAM/version.h])
AC_CONFIG_FILES([Makefile include/Makefile tests/Makefile examples/Makefile doc/Makefile])
AC_OUTPUT

AC_MSG_NOTICE([

CC      : $CC
CXX     : $CXX
CPPFLAGS: $AM_CPPFLAGS $CPPFLAGS
CFLAGS  : $AM_CFLAGS $CFLAGS
CXXFLAGS: $AM_CXXFLAGS $CXXFLAGS
LDFLAGS : $AM_LDFLAGS $LDFLAGS
shared  : $enable_shared
static  : $enable_static
TLS     : $tls
FLINT   : $enable_flint_integer

])
