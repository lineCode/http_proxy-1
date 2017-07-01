main:main.cpp net.o http_parse.o utility.o
	g++ -c -std=c++11 main.cpp
	g++ -pthread main.o net.o http_parse.o utility.o 

net.o:net.cpp
	g++ -c -std=c++11 net.cpp -o net.o

http_parse.o:http_parse.cpp
	g++ -c -std=c++11 http_parse.cpp -o http_parse.o

utility.o:utility.cpp
	g++ -c -std=c++11 utility.cpp -o utility.o

clean:
	rm -f *.o
	rm -f a.out
