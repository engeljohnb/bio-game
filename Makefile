FLAGS =  -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lSDL2 -I include 
all:
	gcc -g -o bio-game src/window.c src/main.c src/input.c src/actor.c src/graphics.c src/glad.c ${FLAGS} 

