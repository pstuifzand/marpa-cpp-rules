
all: rules testmarpa testmarpa2

test.cpp: rules marpa.txt
	./rules > $@

test2.cpp: testmarpa marpa.txt
	./testmarpa > $@

testmarpa: test.cpp marpa.cpp errors.cpp read_file.o
	gcc marpa.cpp test.cpp errors.cpp read_file.o libmarpa.a -o $@ -lstdc++ -std=c++11 -g

testmarpa2: test2.cpp marpa.cpp errors.cpp read_file.o
	gcc marpa.cpp test2.cpp errors.cpp read_file.o libmarpa.a -o $@ -lstdc++ -std=c++11 -g

clean:
	rm -f errors.o rules.o read_file.o
	rm -f rules testmarpa testmarpa2

read_file.o: read_file.cpp
	gcc -c -o $@ $< -std=c++11 -Wall -g -lstdc++

rules.o: rules.cpp marpa.h symbol_table.h
	gcc -c -o $@ $< -std=c++11 -Wall -g -lstdc++

errors.o: errors.cpp
	gcc -c -o $@ $< -std=c++11 -Wall -g

marpa.o: marpa.cpp marpa.h
	gcc -c -o $@ $< -std=c++11 -Wall -g

rules: rules.o errors.o read_file.o marpa.o
	gcc -o $@ $^ libmarpa.a -lstdc++ -g

