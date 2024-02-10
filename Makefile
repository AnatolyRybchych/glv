
CFLAGS		:= -Iinclude $(shell pkg-config --cflags freetype2 sdl2) -Wall -Wextra -Werror -pedantic
LIBS		:= $(shell pkg-config --libs freetype2 gl freetype2 sdl2) -lm

out			:= run

objects		+= main.o

objects		+= glv.o
objects		+= mvp.o
objects		+= vec.o
objects		+= line.o
objects		+= coords.o
objects		+= builtin_shaders.o
objects		+= glv_mgr.o
objects		+= text_view.o
objects		+= text_input.o
objects		+= stack_panel.o
objects		+= canvas.o
objects		+= margin.o
objects		+= menu_panel.o
objects		+= popup_panel.o
objects		+= background.o

build: $(addprefix obj/,$(objects))
	gcc $(CFLAGS) -o $(out) $^ $(LIBS)

obj/%.o:src/%.c
	@mkdir -p ./obj
	gcc -c $(CFLAGS) -o $@ $^

run: build
	./$(out)
