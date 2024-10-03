# Authors
Cole Carley

Aiden Kirk

Nicholas Langley

Nathan Wright

# Building The Project
For ease of development over multiple operating systems, the project should be run in a container. There is a `.devcontainer` folder that outlines the proper container. 

Our project depends on `LLVM 18.1.8` and it's built with `CMake 3.20.0` using `C++17`.

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

Run the executable in either repl mode or compile mode:
```
./compiler
```
OR
```
./compiler /path/to/your/code
```


# GRAMMAR

### Keywords

`var`: variable declaration

`const`: constant variable declaration

`print`: print to screen

### Statements

Stmt -> DeclStmt | ExprStmt | PrintStmt | ConstStmt | Block;

DeclStmt -> "var" \<identifier\> ":" \<type_identifier\> "=" Expr ";"

ExprStmt -> Expr ";"

PrintStmt -> "print" Expr ";"

Block -> '{' Stmt* '}'

ConstStmt -> "const" \<identifier\> ":" \<type_identifier\> "=" Expr ";"


### Expressions

Expr -> Term

Term -> Factor (("+" | "-" ) Factor) *

Factor -> Unary (("*" | "/" ) Uary) *

Unary -> ("-" Unary) | Primary

Primary -> \<int_literal\> | \<identifier\> | \<string_literal\> |  \<float_literal\> | \<bool_literal\>

### Terminals

\<int_literals\> -> [0-9]*

\<float_literals\> -> [0.9]+[\.][0.9]*

\<identifier\> -> [_a-zA-Z]\([_a-zA-Z0-9]*\)

\<bool_literal\> -> true | false

\<string_literal\> -> "[.]*"

\<type_identifier\> -> int | float | bool | str