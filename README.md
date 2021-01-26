<!--
Copyright (C) 2020,2021  CismonX <admin@cismon.net>

Copying and distribution of this file, with or without modification, are
permitted in any medium without royalty, provided the copyright notice and
this notice are preserved. This file is offered as-is, without any warranty.
-->

# U6a

[![Build Status](https://drone.cismon.net/api/badges/esolangs/u6a/status.svg)](https://drone.cismon.net/esolangs/u6a)
[![License](https://img.shields.io/badge/license-GPL--3.0--or--later-blue.svg)](LICENSE)

Implementation of Unlambda, an esoteric programming language.

## Description

The U6a project provides a bytecode compiler and a runtime system for the [Unlambda](http://www.madore.org/~david/programs/unlambda/) programming language.

Ideas behind this implementation can be found [here](https://git.cismon.net/esolangs/u6a/wiki/Developer%27s-Notes-on-Implementing-Unlambda).

U6a is free software. You can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

## Getting Started

To install U6a from source, see [INSTALL](INSTALL).

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
