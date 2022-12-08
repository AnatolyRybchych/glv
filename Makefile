

CARGS	:= -ggdb -Iinclude -Wall -Wextra -Werror -pedantic

objects	+= main.o
objects	+= glv.o
objects	+= mvp.o
objects	+= builtin_shaders.o
objects	+= glv_mgr.o

build:$(addprefix obj/, $(objects))
	gcc $(CARGS) -o run $^ -lGL -lSDL2 -lm

obj/%.o:src/%.c
	gcc -c $(CARGS) -o $@ $^

run: build
	./run

gdb: build
	gdb ./run