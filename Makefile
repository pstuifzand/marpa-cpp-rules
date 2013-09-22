
all: rules

clean:
	rm -f errors.o rules.o read_file.o
	rm -f rules

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

