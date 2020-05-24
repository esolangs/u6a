# u6a

[![Travis CI](https://travis-ci.com/esolangs/u6a.svg)](https://travis-ci.com/esolangs/u6a)
[![Codecov](https://codecov.io/gh/esolangs/u6a/branch/master/graphs/badge.svg)](https://codecov.io/gh/esolangs/u6a)
[![LICENSE](https://img.shields.io/badge/licence-GPLv3-blue.svg)](LICENSE)

Implementation of Unlambda, an esoteric programming language.

## Description

The u6a project provides a bytecode compiler and a runtime system for the [Unlambda](http://www.madore.org/~david/programs/unlambda/) programming language.

Ideas behind this implementation can be found [here](https://github.com/esolangs/u6a/wiki/Developer's-Notes-on-Implementing-Unlambda).

## Getting Started

Building:

```bash
# (If not already) Install the required build tools.
sudo apt install build-essential automake
# (If you want to run tests) Install DejaGnu.
sudo apt install dejagnu
# Generate configuration script.
autoreconf --install
# Execute configuration script with desired options.
./configure --prefix=$HOME
# Compile source code and generate executables.
make
# (Optional) Run tests.
make check
# (Optional) Install executables and man pages.
make install
```

Usage:

```bash
# Compile an Unlambda source file into bytecode.
u6ac -o foo.unl.bc foo.unl
# Execute the bytecode file.
u6a foo.unl.bc
```

See [**u6ac**(1)](man/u6ac.1) and [**u6a**(1)](man/u6a.1) man pages for details.

## Future Plans

* Interactive debugger: `u6adb`
* More compile-time optimizations
* More test cases
* LLVM backend for `u6ac`
