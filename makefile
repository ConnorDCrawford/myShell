# Connor Crawford, 915047545 - Operating Systems Project 2

HEADERS = shell.h builtins.h redirect.h
SOURCE = main.c shell.c builtins.c redirect.c

default: myShell

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

myShell: $(SOURCE)
	gcc $(SOURCE) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f stego