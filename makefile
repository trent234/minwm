# makefile for minwm
# trent wilson
# 03 july 2020

# in case i have custom headers on my system
# INCLUDE = -I /usr/local/include -I.

# in case i have custom libs on my system 
# LIB = -L /usr/local/lib

# looks like only a subset of libs need to be linked.
# run fltk-config for more options if needed
# yaaaa something didn't work during link and adding full list fixed
# LINK = -lfltk -lX11 -lXext #-ltarga
LINK = -lncurses -lX11

# which were uncommented by me and added to OBJ below 
# OBJ = TargaImage.o

minwm: # $(OBJ)
#	g++ -ggdb -Wall -o Project1 Main.cpp $(OBJ) $(INCLUDE) $(LIB) $(LINK) 
	gcc -ggdb -Wall -o minwm minwm.c  $(LINK) 

# TargaImage.o: TargaImage.cpp TargaImage.h
#	g++ -ggdb -Wall -c -o TargaImage.o TargaImage.cpp $(INCLUDE)

# clean:
#	@for obj in $(OBJ); do\
#		if test -f $$obj; then rm $$obj; fi; done
#	@if (test -f Project1); then rm Project1; fi;

