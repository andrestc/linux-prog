build: dep
	cc -I "../lib/" tee.c -o tee.out
	cc -I "../lib/" hole.c -o hole.out
	cc -I "../lib/" cp.c -o cp.out

dep: 
	cd ../lib && $(MAKE)