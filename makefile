LLVM_CONFIG := $(shell brew --prefix llvm)/bin/llvm-config
CXXFLAGS := -g -O2 `$(LLVM_CONFIG) --cxxflags`
LDFLAGS := `$(LLVM_CONFIG) --ldflags --libs core support irreader`

compiler.o: main.o lexer.o parser.o 
	clang++ -std=c++17 -o compiler main.o lexer.o parser.o $(LDFLAGS)

main.o: main.cpp lexer.h parser.h ast_printer.h interpreter.h code_gen.h
	clang++ -std=c++17 -c main.cpp $(CXXFLAGS)

parser.o: parser.cpp parser.h lexer.h binary.h unary.h primary.h 
	clang++ -std=c++17 -c parser.cpp $(CXXFLAGS)

lexer.o: lexer.cpp lexer.h
	clang++ -std=c++17 -c lexer.cpp $(CXXFLAGS)

clean:
	rm -rf *.o compiler