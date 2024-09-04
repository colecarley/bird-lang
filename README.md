
# GRAMMAR

### Statements

Stmt -> DeclStmt | ExprStmt;

DeclStmt -> "let" Expr ";"

ExprStmt -> Expr ";"

### Expressions

Expr -> Factor

Factor -> (Term ("+" | "/" ) Factor) | Term

Term -> (Unary ("*" | "/" ) Term) | Unary

Unary -> ("-" Unary) | Primary 

Primary -> \<number\> | \<identifier\>
