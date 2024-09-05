compiler.o: main.o lexer.o parser.o 
	clang++ -std=c++1z -o compiler main.o lexer.o parser.o 

main.o: main.cpp lexer.h parser.h ast_printer.h interpreter.h
	clang++ -std=c++1z -c main.cpp 

parser.o: parser.cpp parser.h lexer.h binary.h unary.h primary.h 
	clang++ -std=c++1z -c parser.cpp 

lexer.o: lexer.cpp lexer.h
	clang++ -std=c++1z -c lexer.cpp

clean:
	rm -rf *.o compiler