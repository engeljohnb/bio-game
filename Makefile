FLAGS =  -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lSDL2 -lassimp -Iinclude -Iinclude/cglm/include
all:
	gcc -g -o bio-game -Wpedantic -Wextra -Wall -std=gnu11 -Wstrict-prototypes -Wold-style-definition src/*.c ${FLAGS} 


