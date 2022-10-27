CFLAGS += -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -D_DEBUG -D_EJUDGE_CLIENT_SIDE
CC = g++

all: list.exe


list.exe: main.o list.o listdump.o
	$(CC) -o list.exe main.o list.o listdump.o $(CFLAGS)

main.o: main.cpp
	$(CC) -o main.o main.cpp -c $(CFLAGS)

list.o: list.cpp 
	$(CC) -o list.o list.cpp -c $(CFLAGS)

listdump.o: listdump.cpp 
	$(CC) -o listdump.o listdump.cpp -c $(CFLAGS)

clean:
	rm *.o
	clear
	
.PHONY: clean