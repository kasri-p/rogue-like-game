cmake_minimum_required(VERSION 3.30)
project(rogue C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(rogue
    game.h
    main.c
    maps.h
    menu.h
    music.h
    specrooms.h
    )
