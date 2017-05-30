build: dep
	cc -I "../lib/" tee.c -o tee.out
	cc -I "../lib/" hole.c -o hole.out

dep: 
	cd ../lib && $(MAKE)