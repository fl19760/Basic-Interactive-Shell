all: esh

esh: esh.c
	gcc -o esh esh.c -Wall -Werror

clean:
	rm esh