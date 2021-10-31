prodcon: main.o given.o lock.o
	g++ -O -pthread main.o given.o lock.o -o prodcon

main.o: main.cpp lock.h given.h
	g++ -O -c -pthread main.cpp -std=c++2a

given.o: given.cpp given.h
	g++ -O -c given.cpp -std=c++2a

lock.o: lock.cpp lock.h given.h
	g++ -O -c -pthread lock.cpp -std=c++2a