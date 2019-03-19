iRRAM
=====

Exact real arithmetic in C++.

Doxygen API documentation: http://fbrausse.github.io/iRRAM

Installation
------------
The following commands install the packages required for the compilation instructions below.
* Ubuntu:
  ```
  $ apt-get install libgmp-dev libmpfr-dev autoconf gcc g++ make libtool
  ```
* Gentoo:
  ```
  $ emerge dev-libs/gmp dev-libs/mpfr sys-devel/libtool
  ```
* OpenSUSE:
  ```
  $ zypper in gmp-devel mpfr-devel autoconf libtool make
  ```

Compilation
-----------
First the `configure` script needs to be generated. This is done by invoking
```
$ autoreconf -i
```
Next, in order to install locally, issue the following commands.
```
$ mkdir installed
$ ./configure --prefix=`realpath installed`
$ make -j install
```

PIVP solving
------------
A preliminary version can be found here: https://github.com/fbrausse/iRRAM-bigsteps

