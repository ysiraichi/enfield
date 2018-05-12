# Enfield

[![Build Status](https://travis-ci.org/ysiraichi/enfield.svg?branch=master)](https://travis-ci.org/ysiraichi/enfield)

This project was built on top of [Bison](https://www.gnu.org/software/bison/) (v3.0.2) 
and [Flex](https://github.com/westes/flex) (2.5.39).

Check out the [documentation here!!](https://ysiraichi.github.io/enfield/)

## Building

Enfield uses CMake. So, in order to build this project, issue the following commands:

```
$ mkdir build && cd build
$ cmake ../
$ make
```

## Testing

Enfield uses the [Google test framework](https://github.com/google/googletest) to test its components.
To enable automated tests, you should issue the ```cmake``` command as follows:

```
$ cmake ../ -DENABLE_TESTS=on
$ make && make test
```

It is possible to specify the root folder of the GTest framework. In order to do that, you should
pass to ```cmake``` the option ```-DGTEST_ROOT=<path-to-gtest-libs>```.

## Hacking

Even though this project is pretty new, it was designed to be extensible. So, here are a few tips
in order to implement your own algorithm to your desired architecture. Below, I'll list some classes
that are important to be aware of.

(Note that this is a 'begginers guide', so you can do more stuff once you learn the code)

* ```efd::QModule```: The core class of enfield. It holds the AST preprocessed to be easier to use, as well as some other useful methods for modifying the AST. i.e.: ```insertSwapAfter```; ```insertNodeAfter```; ```replaceAllRegsWith```, etc.

* ```efd::QbitAllocator```: This is the base class for implementing allocators. In order to implement your own, you should extend this class and implement the method ```solveDependencies```. This method is responsible for inserting swaps based on the dependencies (that are given by parameter).

* ```IBMQX2.def```: This is not a class per se. Take a look in this file in order to create the specification of other architectures. You should use macros like ```EFD_ARCHITECTURE```, and inserting the line: ```#include "enfield/Arch/YourArch.def"``` in the files ```enfield/Arch/Architecture.h``` and ```lib/Arch/Architecture.cpp``` right bellow the ```IBMQX2.def```.
