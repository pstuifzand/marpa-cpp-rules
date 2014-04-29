CXXFLAGS=-std=c++11 -g
CXXLDFLAGS=-lstdc++ -g libmarpa.a

all: rules rules2 rules3 testmarpa testmarpa2 calc calctree comma literal diff balanced

test.cpp: rules2 marpa.txt
	./rules2 marpa.txt > $@

test2.cpp: testmarpa marpa.txt
	./testmarpa marpa.txt > $@

calc.cpp: testmarpa calc.txt
	./testmarpa calc.txt > $@

calctree.cpp: testmarpa calctree.txt
	./testmarpa calctree.txt > $@

comma.cpp: testmarpa comma.txt
	./testmarpa comma.txt > $@

literal.cpp: testmarpa literal.txt
	./testmarpa literal.txt > $@

balanced.cpp: testmarpa balanced.txt
	./testmarpa balanced.txt > $@

diff.cpp: testmarpa diff.txt
	./testmarpa diff.txt > $@

testmarpa: test.cpp errors.cpp read_file.o
	gcc test.cpp errors.cpp read_file.o -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

testmarpa2: test2.cpp errors.cpp read_file.o
	gcc test2.cpp errors.cpp read_file.o -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

calc: calc.cpp errors.cpp read_file.o
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

calctree: calctree.cpp errors.cpp read_file.o
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

comma: comma.cpp errors.cpp read_file.o
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

literal: literal.cpp errors.cpp read_file.o
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

balanced: balanced.cpp errors.cpp read_file.o
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

diff: diff.cpp errors.cpp
	gcc $^ -o $@ $(CXXLDFLAGS) $(CXXFLAGS)

clean:
	rm -f errors.o rules.o rules2.o rules3.o read_file.o
	rm -f comma.o literal.o diff.o balanced.o
	rm -f test.cpp test2.cpp calc.cpp calctree.cpp diff.cpp literal.cpp comma.cpp balanced.cpp
	rm -f rules rules2 rules3 testmarpa testmarpa2 calc calctree literal diff comma balanced

read_file.o: read_file.cpp
	gcc -c -o $@ $< -std=c++11 -Wall -g -lstdc++

rules.o: rules.cpp marpa-cpp/marpa.hpp symbol_table.h
	gcc -c -o $@ $< $(CXXFLAGS)

rules2.o: rules2.cpp marpa-cpp/marpa.hpp symbol_table.h
	gcc -c -o $@ $< $(CXXFLAGS)

rules3.o: rules3.cpp marpa-cpp/marpa.hpp symbol_table.h evaluator.h grammar_bits.h error.h read_file.h
	gcc -c -o $@ $< $(CXXFLAGS)

errors.o: errors.cpp
	gcc -c -o $@ $< -std=c++11 -Wall -g

rules: rules.o errors.o read_file.o
	gcc -o $@ $^ $(CXXLDFLAGS)

rules2: rules2.o errors.o read_file.o
	gcc -o $@ $^ $(CXXLDFLAGS)

rules3: rules3.o errors.o read_file.o
	gcc -o $@ $^ $(CXXLDFLAGS) -g

