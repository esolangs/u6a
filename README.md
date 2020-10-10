<!--
Copyright (C) 2020  CismonX <admin@cismon.net>

Copying and distribution of this file, with or without modification, are
permitted in any medium without royalty, provided the copyright notice and
this notice are preserved. This file is offered as-is, without any warranty.
-->

# u6a

[![Travis CI](https://travis-ci.com/esolangs/u6a.svg)](https://travis-ci.com/esolangs/u6a)
[![Codecov](https://codecov.io/gh/esolangs/u6a/branch/master/graphs/badge.svg)](https://codecov.io/gh/esolangs/u6a)
[![LICENSE](https://img.shields.io/badge/licence-GPL--3.0--or--later-blue.svg)](LICENSE)

Implementation of Unlambda, an esoteric programming language.

## Description

The u6a project provides a bytecode compiler and a runtime system for the [Unlambda](http://www.madore.org/~david/programs/unlambda/) programming language.

Ideas behind this implementation can be found [here](https://github.com/esolangs/u6a/wiki/Developer's-Notes-on-Implementing-Unlambda).

## Getting Started

To install u6a from source, see [INSTALL](INSTALL).

Usage (See [**u6ac**(1)](man/u6ac.1) and [**u6a**(1)](man/u6a.1) man pages for details):

```bash
# Compile an Unlambda source file into bytecode.
u6ac -o foo.unl.bc foo.unl
# Execute the bytecode file.
u6a foo.unl.bc
```

## Future Plans

* Interactive debugger: `u6adb`
* More compile-time optimizations
* More test cases
* LLVM backend for `u6ac`
