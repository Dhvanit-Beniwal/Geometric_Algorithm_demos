.SILENT:
exe : main.cpp algorithm.hpp
	g++ -c main.cpp -o main.o
	g++ main.o -o exe -lsfml-graphics -lsfml-window -lsfml-system
run : exe
	./exe
clean :
	rm *.o exe
