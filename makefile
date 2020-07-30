# makefile for minwm
# trent wilson
# 29 july 2020
# commented out stuff is guide for more complex source dir of the future

# in case i have custom headers on my system
# INCLUDE = -I /usr/local/include -I.

# in case i have custom libs on my system 
# LIB = -L /usr/local/lib

LINK = -lncurses -lX11

# our object files that are passed to the main file to create the executable binary
# OBJ = example.o

minwm: minwm.c # $(OBJ)
#	cc -ggdb -Wall -o minwm minwm.c $(OBJ) $(INCLUDE) $(LIB) $(LINK) 
	cc -ggdb -Wall -o minwm minwm.c  $(LINK) 

# example.o: example.c example.h
#	cc -ggdb -Wall -c -o example.o example.c $(INCLUDE)

 clean:
#	@for obj in $(OBJ); do\
#		if test -f $$obj; then rm $$obj; fi; done
	@if (test -f minwm); then rm minwm; fi;
