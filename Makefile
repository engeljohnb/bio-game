FLAGS =  -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lSDL2 -Iinclude -Iinclude/cglm/include
FILES = src/window.c src/main.c src/input.c src/actor.c src/graphics.c src/glad.c src/utils.c src/camera.c src/time.c
all:
	gcc -g -o bio-game ${FILES} ${FLAGS} 

