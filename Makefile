
INCLUDE		:= -I/usr/include/freetype2/
CARGS		:= $(INCLUDE) -ggdb -Iinclude -Wall -Wextra -Werror -pedantic

objects		+= main.o
objects		+= glv.o
objects		+= mvp.o
objects		+= vec.o
objects		+= line.o
objects		+= coords.o
objects		+= builtin_shaders.o
objects		+= glv_mgr.o

#standard views 
objects		+= text_view.o
objects		+= text_input.o
objects		+= stack_panel.o
objects		+= canvas.o
objects		+= margin.o
objects		+= menu_panel.o
objects		+= popup_panel.o
objects		+= background.o

build:$(addprefix obj/, $(objects))
	gcc $(CARGS) -o run $^ -lGL -lSDL2 -lm -lfreetype

obj/%.o:src/%.c
	@mkdir -p ./obj
	gcc -c $(CARGS) -o $@ $^

run: build
	./run

gdb: build
	gdb ./run