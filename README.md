# Authors
Cole Carley

Aiden Kirk

Nicholas Langley


# GRAMMAR

### Keywords

`var`: variable declaration

`print`: print to screen

### Statements

Stmt -> DeclStmt | ExprStmt | While | Block;

DeclStmt -> "let" Expr ";"

ExprStmt -> Expr ";"

While -> "while" Expr Stmt

Block -> '{' Stmt* '}'


### Expressions

Expr -> Term

Term -> (Factor ("+" | "-" ) Factor) | Term

Factor -> (Unary ("*" | "/" ) Uary) | Unary

Unary -> ("-" Unary) | Primary 

Primary -> \<int\> | \<identifier\>
