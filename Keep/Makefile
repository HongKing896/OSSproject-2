.PHONY : all
all : keep

.PHONY : clean
clean:
	rm -rf keep
	rm -rf keep.o

keep : keep.o
	gcc -o keep keep.o

keep.o : keep.c
	gcc -g -c keep.c -o keep.o
