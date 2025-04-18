#include <stdio.h>
// #include <AudioToolbox/AudioToolbox.h>
#include <unistd.h>
#include <SDL.h>
#include<SDL_mixer.h>
// #include <SDL_mixer>

int main(int argc, char *argv[])
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        return 1;
    }

    // Load music
    Mix_Music *music = Mix_LoadMUS("/Users/kasra/Desktop/Boreal.wav"); // Replace with your music file
    if (music == NULL)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Play music infinitely
    if (Mix_PlayMusic(music, -1) == -1)
    {
        printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Wait for user input to exit
    printf("Music playing. Press Enter to stop...\n");
    getchar();

    // Cleanup
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}