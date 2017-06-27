build: dep
	cc -I "../lib/" large_file.c -o large_file.out
	cc -I "../lib/" append.c -o append.out

dep: 
	cd ../lib && $(MAKE)