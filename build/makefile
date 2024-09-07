LLVM_CONFIG := $(shell brew --prefix llvm)/bin/llvm-config
CXXFLAGS := -g -O2 `$(LLVM_CONFIG) --cxxflags`
LDFLAGS := `$(LLVM_CONFIG) --ldflags --libs core support irreader`

compiler.o: main.o lexer.o parser.o 
	clang++ -std=c++17 -o compiler main.o lexer.o parser.o $(LDFLAGS)

main.o: ../main.cpp ../include/lexer.h ../include/parser.h 
	clang++ -std=c++17 -c ../main.cpp $(CXXFLAGS)

parser.o: ../src/parser.cpp ../include/parser.h ../include/lexer.h
	clang++ -std=c++17 -c ../src/parser.cpp $(CXXFLAGS)

lexer.o: ../src/lexer.cpp ../include/lexer.h
	clang++ -std=c++17 -c ../src/lexer.cpp $(CXXFLAGS)

clean:
	rm -rf *.o compiler