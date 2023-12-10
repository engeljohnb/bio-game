FLAGS =  -Wpedantic -Wall -Wextra -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lSDL2 -lassimp -Iinclude -Iinclude/cglm/include -Wno-unused-variable -std=gnu11 \
	 -Wno-stringop-truncation -Wstrict-prototypes -Wold-style-definition
all:
	gcc -g -o bio-game src/*.c ${FLAGS} 


