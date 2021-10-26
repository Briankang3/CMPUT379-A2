prodcon: main.o given.o lock.o
	g++ -g -pthread main.o given.o lock.o -o prodcon

main.o: main.cpp lock.h given.h
	g++ -g -c -pthread main.cpp -std=c++2a

given.o: given.cpp given.h
	g++ -g -c given.cpp -std=c++2a

lock.o: lock.h given.h
	g++ -g -c -pthread -std=c++2a