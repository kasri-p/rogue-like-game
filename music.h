#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>
#include <ncurses.h>

void signup_login();

typedef struct
{
    const char *name;
    const char *path;
    Mix_Music *music;
} MusicTrack;

static MusicTrack playlist[] = {
    {"ELden Ring", "/Users/kasra/Desktop/songs/menu.mp3", NULL},
    {"Malenia, Blade Of Miquella", "/Users/kasra/Desktop/songs/Malenia.mp3", NULL},
    {"Radahn, The Promised Consort", "/Users/kasra/Desktop/songs/radahn.mp3", NULL},
    {"Jori", "/Users/kasra/Desktop/songs/jori.mp3", NULL},
    {"Enchanted Room", "/Users/kasra/Desktop/songs/jori.mp3", NULL},
    {"Lose Game", "/Users/kasra/Desktop/songs/jori.mp3", NULL},
    {"Win Game", "/Users/kasra/Desktop/songs/jori.mp3", NULL}};

static const int NUM_TRACKS = 6;
static int current_track_index = 0;
static int temp_track_index = 0;
static bool music_initialized = false;
static int music_volume = MIX_MAX_VOLUME;
static bool music_enabled = true;

bool init_music_system()
{
    if (music_initialized)
        return true;

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        SDL_Quit();
        return false;
    }

    music_initialized = true;
    return true;
}

void load_music(int index)
{
    if (index < 0 || index >= NUM_TRACKS)
        return;

    if (playlist[index].music == NULL)
    {
        playlist[index].music = Mix_LoadMUS(playlist[index].path);
        if (playlist[index].music == NULL)
        {
            printf("Failed to load music %s! SDL_mixer Error: %s\n",
                   playlist[index].name, Mix_GetError());
        }
    }
}

void play_music(int index)
{
    if (!music_initialized && !init_music_system())
        return;

    if (index < 0 || index >= NUM_TRACKS)
        return;

    load_music(index);

    if (playlist[index].music != NULL)
    {
        Mix_HaltMusic();
        Mix_VolumeMusic(music_volume);
        if (Mix_PlayMusic(playlist[index].music, -1) == -1)
        {
            printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
            return;
        }
        current_track_index = index;
    }
}

void music()
{
    play_music(2);
}

void toggle_music()
{
    if (!music_initialized)
        return;

    if (Mix_PlayingMusic())
    {
        if (Mix_PausedMusic())
        {
            Mix_ResumeMusic();
        }
        else
        {
            Mix_PauseMusic();
        }
    }
    else
    {
        play_music(current_track_index);
    }
}

void adjust_volume(int delta)
{
    music_volume = music_volume + delta;
    if (music_volume < 0)
        music_volume = 0;
    if (music_volume > MIX_MAX_VOLUME)
        music_volume = MIX_MAX_VOLUME;
    Mix_VolumeMusic(music_volume);
}

void draw_music_selector(int selected)
{
    clear();
    boarder();

    attron(COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);
    mvprintw(LINES / 4 - 3, COLS / 2 - 16, "╔══════════════════════════════╗");
    mvprintw(LINES / 4 - 2, COLS / 2 - 16, "║    🎵 Music Selection 🎵     ║");
    mvprintw(LINES / 4 - 1, COLS / 2 - 16, "╚══════════════════════════════╝");
    attroff(COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);

    mvprintw(LINES / 4 + 1, COLS / 2 - 18, "┌─────────── Now Playing ───────────┐");
    mvprintw(LINES / 4 + 2, COLS / 2 - 18, "│                                   │");
    mvprintw(LINES / 4 + 3, COLS / 2 - 18, "│  %s", playlist[current_track_index].name);
    mvprintw(LINES / 4 + 4, COLS / 2 - 18, "│                                   │");
    mvprintw(LINES / 4 + 5, COLS / 2 - 18, "└───────────────────────────────────┘");
    mvprintw(LINES / 4 + 3, COLS / 2 + 18, "|");

    int start_y = LINES / 2 - 2;

    mvprintw(start_y - 1, COLS / 2 - 20, "┌──────────────── Track List ────────────────┐");

    for (int i = 0; i < 4; i++)
    {
        mvprintw(start_y + i, COLS / 2 - 18, "│");

        if (i == selected)
        {
            attron(A_REVERSE | A_BOLD | COLOR_PAIR(2));
            mvprintw(start_y + i, COLS / 2 - 18, "♫ %-40s", playlist[i].name);
            attroff(A_REVERSE | A_BOLD | COLOR_PAIR(2));
        }
        else
        {
            mvprintw(start_y + i, COLS / 2 - 20, "  %-40s", playlist[i].name);
        }
    }

    mvprintw(start_y + 4, COLS / 2 - 20,
             "└───────────────────────────────────────────┘");

    mvprintw(LINES - 4, COLS / 2 - 21, "╭─────────────── Controls ───────────────╮");
    mvprintw(LINES - 3, COLS / 2 - 21, "│  ↑↓: Select   Enter: Play   Esc: Back  │");
    mvprintw(LINES - 2, COLS / 2 - 21, "╰────────────────────────────────────────╯");
}

void music_selector()
{
    int selected = current_track_index;

    while (1)
    {
        draw_music_selector(selected);
        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            selected = (selected == 0) ? 4 - 1 : selected - 1;
            break;
        case KEY_DOWN:
            selected = (selected == 4 - 1) ? 0 : selected + 1;
            break;
        case '\n':
            temp_track_index = selected;
            play_music(selected);
            break;
        case 27: // ESC
            for (int i = 0; i < LINES; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    mvprintw(i, j, " ");
                }
            }
            return;
        }
    }
}

void music_setting(int from, char username[], int score)
{
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);    
    init_pair(2, COLOR_BLACK, COLOR_CYAN);    
    init_pair(3, COLOR_MAGENTA, COLOR_BLACK); 

    clear();
    boarder();

    const char *options[] = {
        "♪ Toggle Music",
        "♫ Change Music",
        "🔊 Volume Up",
        "🔉 Volume Down",
        "↩ Back"};
    int num_options = 5;
    int selected = 0;

    while (1)
    {
        attron(COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);
        mvprintw(LINES / 4, COLS / 2 - 10, "♪ Music Settings ♪");
        attroff(COLOR_PAIR(1) | A_BOLD | A_UNDERLINE);

        mvprintw(LINES / 4 + 2, COLS / 2 - 17, "┌──────────────────────────────┐");
        mvprintw(LINES / 4 + 3, COLS / 2 - 17, "│ Current Volume: %-14d │",
                 (music_volume * 100) / MIX_MAX_VOLUME);
        mvprintw(LINES / 4 + 4, COLS / 2 - 17, "│ Music Status: %-16s │",
                 Mix_PlayingMusic() ? (Mix_PausedMusic() ? "Paused" : "Playing") : "Stopped");
        mvprintw(LINES / 4 + 5, COLS / 2 - 17, "│ Current Track: %-15s │",
                 playlist[current_track_index].name);
        mvprintw(LINES / 4 + 6, COLS / 2 - 17, "└──────────────────────────────┘");

        // Menu options with enhanced selection
        for (int i = 0; i < num_options; i++)
        {
            if (i == selected)
            {
                attron(A_REVERSE | A_BOLD | COLOR_PAIR(2));
                mvprintw(LINES / 2 + i * 2, COLS / 2 - strlen(options[i]) / 2, "► %s ◄", options[i]);
                attroff(A_REVERSE | A_BOLD | COLOR_PAIR(2));
            }
            else
            {
                mvprintw(LINES / 2 + i * 2, COLS / 2 - strlen(options[i]) / 2, "  %s  ", options[i]);
            }
        }

        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            selected = (selected == 0) ? num_options - 1 : selected - 1;
            break;
        case KEY_DOWN:
            selected = (selected == num_options - 1) ? 0 : selected + 1;
            break;
        case '\n':
            switch (selected)
            {
            case 0:
                toggle_music();
                break;
            case 1:
                music_selector();
                clear();
                break;
            case 2:
                adjust_volume(MIX_MAX_VOLUME / 10);
                break;
            case 3:
                adjust_volume(-MIX_MAX_VOLUME / 10);
                break;
            case 4:
                clear();
                if (from == 1)
                    signup_login();
                else if (from == 2)
                    pre_game_menu(username);
                else if (from == 3)
                    pause_menu(username, hero_x, hero_y, score);
            }
            break;
        case 27:
            clear();
            return;
        }
    }
}