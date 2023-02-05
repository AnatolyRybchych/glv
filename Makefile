
CFLAGS		:= -fPIC -Iinclude $(shell pkg-config --cflags freetype2 sdl2) -Wall -Wextra -Werror -pedantic
LIBS		:= $(shell pkg-config --libs freetype2 gl freetype2 sdl2) -lm

GLV_DDL		:= lib/libglv.so

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

build_dll: $(addprefix obj/, $(objects))
	gcc -shared -o $(GLV_DDL) $(CFLAGS) $^ $(LIBS)

#sudo required
install_linux: build_dll
	ln -sf /usr/include/freetype2/* ./include
	cp -r ./include/* /usr/include/
	cp -r ./lib/* /usr/lib/

#sudo required
uninstall_linux:
	rm  /usr/include/glv.h
	rm -r /usr/include/glv/*
	rm /usr/lib/libglv.so


obj/%.o:src/%.c
	@mkdir -p ./obj
	gcc -c $(CFLAGS) -o $@ $^
