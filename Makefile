main: main.c 
	gcc -g -lncurses -lmenu main.c -o main

.PHONY: clean

clean:
	rm -f main
