# Bird
Our goal is to create a programming language that can interoperate with JavaScript, provide a web-first interface, and maintain the speed of WebAssembly. When building a website, there is only one real option for a client-side programming language: JavaScript. As websites become more complicated and more business logic is run on the web, JavaScript’s faults become more apparent; it’s just too slow. WebAssembly is an alternative that can be run on all major browsers, but it serves as a compilation target for other languages like C or C++. These languages are great but weren’t designed with the web in mind. The rise of WebAssembly as a web-compatible compilation target and the lack of a proper syntax to write fast, client-side code demands a new language. 


# Building The Project
For ease of development over multiple operating systems, the project should be run in a container. There is a `.devcontainer` folder that outlines the proper container. 

Our project depends on `LLVM 18.1.8` and `GooglTest` and it's built with `CMake 3.20.0` using `C++17`.

After running the project in a container, follow these commands:

Enter the build folder: 
```
cd build
```

Generate a makefile:
```
cmake ..
```

Run the makefile:
```
make
```

Run the executable:

repl mode:
```
./compiler 
```
OR

compiler mode:
```
./compiler /path/to/your/code 
```

OR

interpreter mode:
```
./compiler -i /path/to/your/code 
```

# Testing
All tests live in the `tests` folder. Each sub folder that ends in `*_suite` contains a suite of tests. Any file in the `tests` folder than ends in `*_test.cpp`, will be built. 

To build and run the tests:

Enter the build folder
```
cd build
```

Generate the makefile with the `TESTS` option:
```
cmake .. -DTESTS=ON
```

Run the makefile:
```
make
```

Run the tests:
```
ctest --verbose
```

Note that the `TEST` option is persistent, so be sure to turn it off after testing to decrease your development build times.

```
cmake .. -DTESTS=OFF
```

# Authors
Cole Carley

Aidan Kirk

Nicholas Langley

Nathan Wright

