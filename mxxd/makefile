.PHONY : all
all : mxxd

.PHONY : clean
clean:
	rm -rf mxxd
	rm -rf mxxd.o

mxxd : mxxd.o
	gcc -o mxxd mxxd.o

mxxd.o : mxxd.c
	gcc -g -c mxxd.c -o mxxd.o