#!/bin/bash
gcc main.c menu.h game.h maps.h specrooms.h  -lncurses  -I/opt/homebrew/include/SDL2 -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -lsqlite3
./a.out
