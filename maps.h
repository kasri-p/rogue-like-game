#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>
#include <time.h>
#include <locale.h>
#include <stdbool.h>
#include "specrooms.h"

void boarder();
void login_page();
void pre_game_menu(char username[]);
void pause_menu(char username[], int hero_x, int hero_y, int score);
void clear_error();
void maps(char username[]);
void continue_game(char username[]);
void save_game(char username[], int score);
int load_game(char username[], int *score);
void check_save_exists(char username[]);
void draw_map();

time_t speed_spell_end_time = 0;
time_t health_spell_end_time = 0;
time_t damage_spell_end_time = 0;
int last_dx = 0, last_dy = 0;

void add_traps() {
    for (int f = 0; f < 4; f++)
    {
        for (int r = 0; r < 6; r++)
        {
            int num_traps = 1 + rand() % 4;
            for (int t = 0; t < num_traps; t++)
            {
                int trap_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int trap_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);

                if (floors[f].map[trap_y][trap_x] == '.' &&
                    floors[f].map[trap_y][trap_x] != '+' &&
                    floors[f].map[trap_y][trap_x] != '<' &&
                    floors[f].map[trap_y][trap_x] != '>')
                {
                    floors[f].temp_map[trap_y][trap_x] = '^';
                }
            }
        }
    }
}

void add_spells()
{
    for (int f = 0; f < 4; f++)
    {
        int num_speed = rand() % 3;
        int num_health = rand() % 3;
        int num_damage = rand() % 3;

        for (int i = 0; i < num_speed; i++)
        {
            int room = rand() % 6;
            int spell_x = floors[f].rooms[room].x + 1 + rand() % (floors[f].rooms[room].width - 2);
            int spell_y = floors[f].rooms[room].y + 1 + rand() % (floors[f].rooms[room].height - 2);
            if (spell_x == floors[f].rooms[room].right_door_x)
            {
                spell_x -= 1;
            }
            if (floors[f].map[spell_y][spell_x] == '.' && floors[f].map[spell_y][spell_x - 1] != '%' &&
                floors[f].map[spell_y][spell_x - 1] != '~' && floors[f].map[spell_y][spell_x - 1] != '!' &&
                floors[f].map[spell_y][spell_x - 1] != 'H' && floors[f].map[spell_y][spell_x - 1] != 'K')
                floors[f].map[spell_y][spell_x] = '~';
        }

        for (int i = 0; i < num_health; i++)
        {
            int room = rand() % 6;
            int spell_x = floors[f].rooms[room].x + 1 + rand() % (floors[f].rooms[room].width - 2);
            int spell_y = floors[f].rooms[room].y + 1 + rand() % (floors[f].rooms[room].height - 2);
            if (spell_x == floors[f].rooms[room].right_door_x)
            {
                spell_x -= 1;
            }
            if (floors[f].map[spell_y][spell_x] == '.' && floors[f].map[spell_y][spell_x - 1] != '%' &&
                floors[f].map[spell_y][spell_x - 1] != '~' && floors[f].map[spell_y][spell_x - 1] != '!' &&
                floors[f].map[spell_y][spell_x - 1] != 'H' && floors[f].map[spell_y][spell_x - 1] != 'K')
                floors[f].map[spell_y][spell_x] = 'H';
        }

        for (int i = 0; i < num_damage; i++)
        {
            int room = rand() % 6;
            int spell_x = floors[f].rooms[room].x + 1 + rand() % (floors[f].rooms[room].width - 2);
            int spell_y = floors[f].rooms[room].y + 1 + rand() % (floors[f].rooms[room].height - 2);
            if (spell_x == floors[f].rooms[room].right_door_x)
            {
                spell_x -= 1;
            }
            if (floors[f].map[spell_y][spell_x] == '.' && floors[f].map[spell_y][spell_x - 1] != '%' &&
                floors[f].map[spell_y][spell_x - 1] != '~' && floors[f].map[spell_y][spell_x - 1] != '!' &&
                floors[f].map[spell_y][spell_x - 1] != 'H' && floors[f].map[spell_y][spell_x - 1] != 'K')
                floors[f].map[spell_y][spell_x] = '!';
        }
    }
}

void add_golds()
{
    for (int f = 0; f < 4; f++)
    {
        for (int r = 0; r < 6; r++)
        {
            int num_gold = rand() % 4;
            for (int g = 0; g < num_gold; g++)
            {
                int gold_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int gold_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);

                if (floors[f].map[gold_y][gold_x] == '.')
                    floors[f].map[gold_y][gold_x] = '$';
            }
        }

        int room_index = rand() % 6;
        int gold_x = floors[f].rooms[room_index].x + 1 + rand() % (floors[f].rooms[room_index].width - 2);
        int gold_y = floors[f].rooms[room_index].y + 1 + rand() % (floors[f].rooms[room_index].height - 2);

        if (floors[f].map[gold_y][gold_x] == '.')
            floors[f].map[gold_y][gold_x] = 'b';
    }

    for (int f = 0; f < 4; f++)
    {
        for (int r = 0; r < 6; r++)
        {
            if (rand() % 10 == 0)
            {
                int dagger_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int dagger_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);
                if (floors[f].map[dagger_y][dagger_x] == '.')
                {
                    floors[f].map[dagger_y][dagger_x] = '"';
                }
            }

            if (rand() % 10 == 0)
            {
                int arrow_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int arrow_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);
                if (floors[f].map[arrow_y][arrow_x] == '.')
                {
                    floors[f].map[arrow_y][arrow_x] = 'A';
                }
            }

            if (rand() % 10 == 0)
            {
                int wand_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int wand_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);
                if (floors[f].map[wand_y][wand_x] == '.')
                {
                    floors[f].map[wand_y][wand_x] = 'W';
                }
            }
            if (rand() % 10 == 0)
            {
                int sword_x = floors[f].rooms[r].x + 1 + rand() % (floors[f].rooms[r].width - 2);
                int sword_y = floors[f].rooms[r].y + 1 + rand() % (floors[f].rooms[r].height - 2);
                if (floors[f].map[sword_y][sword_x] == '.')
                {
                    floors[f].map[sword_y][sword_x] = 's';
                }
            }
        }
    }
}

void init_bag()
{
    bag.damage_spells = 0;
    bag.gold_count = 0;
    bag.has_ancient_key = 0;
    bag.health_spells = 0;
    bag.speed_spells = 0;

    bag.mace = true;
    bag.magic_wand = false;
    bag.sword = false;
    bag.normal_arrow = false;
    bag.dagger = false;

    bag.wand_count = 0;
    bag.dagger_count = 0;
    bag.arrow_count = 0;
}

Floor *create_floor()
{
    Floor *floor = (Floor *)malloc(sizeof(Floor));

    floor->map = (char **)malloc(HEIGHT * sizeof(char *));
    floor->temp_map = (char **)malloc(HEIGHT * sizeof(char *));
    floor->discovered_map = (char **)malloc(HEIGHT * sizeof(char *));

    for (int i = 0; i < HEIGHT; i++)
    {
        floor->map[i] = (char *)malloc(WIDTH * sizeof(char));
        floor->temp_map[i] = (char *)malloc(WIDTH * sizeof(char));
        floor->discovered_map[i] = (char *)malloc(WIDTH * sizeof(char));
    }

    floor->num_rooms = 0;

    for (int i = 0; i < 6; i++)
    {
        floor->rooms[i] = (Room){0};
    }

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            floor->map[y][x] = ' ';
            floor->temp_map[y][x] = ' ';
        }
    }
    floor->floor_discovered = false;
    return floor;
}

void init_floor_map(Floor *floor)
{
    if (floor == NULL || floor->map == NULL)
    {
        return;
    }

    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            floor->map[y][x] = ' ';
}

void init_map()
{
    for (int y = 0; y < HEIGHT + 2; y++)
        for (int x = 0; x < WIDTH; x++)
            map[y][x] = ' ';
}

int is_room_valid(Room room, Floor *floor)
{
    if (room.x < 1 || room.y < 1 ||
        room.x + room.width >= WIDTH - 1 ||
        room.y + room.height >= HEIGHT - 1)
        return 0;

    for (int i = 0; i < floor->num_rooms; i++)
    {
        Room other = floor->rooms[i];
        if (room.x <= other.x + other.width + 2 &&
            room.x + room.width + 2 >= other.x &&
            room.y <= other.y + other.height + 2 &&
            room.y + room.height + 2 >= other.y)
            return 0;
    }
    return 1;
}

void add_door(Room *room, int room_index, Floor *floor)
{
    int door_y = room->y + 1 + rand() % (room->height - 2);

    if (room_index == 0 || room_index == 5)
    {
        room->right_door_x = room->x + room->width - 1;
        room->right_door_y = door_y;
        floor->map[room->right_door_y][room->right_door_x] = '+';
    }
    else
    {
        room->left_door_x = room->x;
        room->left_door_y = door_y;
        floor->map[room->left_door_y][room->left_door_x] = '+';

        door_y = room->y + 1 + rand() % (room->height - 2);
        room->right_door_x = room->x + room->width - 1;
        room->right_door_y = door_y;
        floor->map[room->right_door_y][room->right_door_x] = '+';
    }
}

void add_room(Room room, int room_index, Floor *floor)
{
    for (int x = room.x; x < room.x + room.width; x++)
    {
        floor->map[room.y][x] = '-';
        floor->map[room.y + room.height - 1][x] = '-';
    }
    for (int y = room.y; y < room.y + room.height; y++)
    {
        floor->map[y][room.x] = '|';
        floor->map[y][room.x + room.width - 1] = '|';
    }

    for (int y = room.y + 1; y < room.y + room.height - 1; y++)
        for (int x = room.x + 1; x < room.x + room.width - 1; x++)
            floor->map[y][x] = '.';

    int num_emojis = rand() % 2 + 1;
    for (int i = 0; i < num_emojis; i++)
    {
        int emoji_x = room.x + 2 + rand() % (room.width - 4);
        int emoji_y = room.y + 2 + rand() % (room.height - 4);

        if (floor->map[emoji_y][emoji_x] == '.' &&
            floor->map[emoji_y][emoji_x - 1] != '%')
        {
            floor->map[emoji_y][emoji_x] = '%';
        }
    }

    int num_windows = rand() % 3;
    for (int i = 0; i < num_windows; i++)
    {
        int wall = rand() % 4;
        int window_x, window_y;

        switch (wall)
        {
        case 0:
            window_x = room.x + 1 + rand() % (room.width - 2);
            window_y = room.y;
            break;
        case 1:
            window_x = room.x + room.width - 1;
            window_y = room.y + 1 + rand() % (room.height - 2);
            break;
        case 2:
            window_x = room.x + 1 + rand() % (room.width - 2);
            window_y = room.y + room.height - 1;
            break;
        case 3:
            window_x = room.x;
            window_y = room.y + 1 + rand() % (room.height - 2);
            break;
        }

        if (floor->map[window_y][window_x] == '|' ||
            floor->map[window_y][window_x] == '-')
        {
            floor->map[window_y][window_x] = '=';
        }
    }

    add_door(&room, room_index, floor);

    int num_pillars = rand() % 3 + 1;
    for (int i = 0; i < num_pillars; i++)
    {
        int pillar_x = room.x + 1 + rand() % (room.width - 2);
        int pillar_y = room.y + 1 + rand() % (room.height - 2);

        if (floor->map[pillar_y][pillar_x] == '.' && (floor->map[pillar_y][pillar_x + 1] != '+') && (floor->map[pillar_y][pillar_x - 1] != '+'))
            floor->map[pillar_y][pillar_x] = 'O';
    }
    room.is_room_nightmare = false;
    room.is_room_locked = false;

    floor->rooms[floor->num_rooms++] = room;
}

void connect_rooms(Floor *floor)
{
    for (int i = 0; i < floor->num_rooms - 1; i++)
    {
        Room r1 = floor->rooms[i];
        Room r2 = floor->rooms[i + 1];

        int x1, y1, x2, y2;

        if (i == 2)
        {
            x1 = r1.right_door_x;
            y1 = r1.right_door_y;
            x2 = r2.right_door_x;
            y2 = r2.right_door_y;
        }
        else if (i < 3)
        {
            x1 = r1.right_door_x;
            y1 = r1.right_door_y;
            x2 = r2.left_door_x;
            y2 = r2.left_door_y;
        }
        else
        {
            x1 = r1.left_door_x;
            y1 = r1.left_door_y;
            x2 = r2.right_door_x;
            y2 = r2.right_door_y;
        }

        if (i != 2)
        {
            int mid_x = (x1 + x2) / 2;
            if (x1 <= mid_x)
            {
                for (int x = x1; x <= mid_x; x++)
                    if (floor->map[y1][x] == ' ')
                        floor->map[y1][x] = '#';
            }
            else
            {
                for (int x = x1; x >= mid_x; x--)
                    if (floor->map[y1][x] == ' ')
                        floor->map[y1][x] = '#';
            }

            if (y1 <= y2)
            {
                for (int y = y1; y <= y2; y++)
                    if (floor->map[y][mid_x] == ' ')
                        floor->map[y][mid_x] = '#';
            }
            else
            {
                for (int y = y1; y >= y2; y--)
                    if (floor->map[y][mid_x] == ' ')
                        floor->map[y][mid_x] = '#';
            }

            if (mid_x <= x2)
            {
                for (int x = mid_x; x <= x2; x++)
                    if (floor->map[y2][x] == ' ')
                        floor->map[y2][x] = '#';
            }
            else
            {
                for (int x = mid_x; x >= x2; x--)
                    if (floor->map[y2][x] == ' ')
                        floor->map[y2][x] = '#';
            }
        }
        else
        {
            if (x1 < x2)
            {
                for (int x = x1 + 1; x <= x2 + 1; x++)
                {
                    if (floor->map[y1][x] == ' ')
                        floor->map[y1][x] = '#';
                }
                for (int y = y1; y <= y2; y++)
                {
                    if (floor->map[y][x2 + 1] == ' ')
                        floor->map[y][x2 + 1] = '#';
                }
            }
            else
            {
                for (int y = y1; y <= y2; y++)
                {
                    floor->map[y][x1 + 1] = '#';
                }
                for (int x = x1; x > x2; x--)
                {
                    floor->map[y2][x] = '#';
                }
            }
        }
    }
}

void initialize_map()
{
    map = malloc(HEIGHT * sizeof(char *));
    temp_map = malloc(HEIGHT * sizeof(char *));
    discovered_map = malloc(HEIGHT * sizeof(char *));

    for (int i = 0; i < HEIGHT; i++)
    {
        map[i] = malloc(WIDTH * sizeof(char));
        temp_map[i] = malloc(WIDTH * sizeof(char));
        discovered_map[i] = malloc(WIDTH * sizeof(char));
        memset(discovered_map[i], 0, WIDTH * sizeof(char));
    }
}

void cleanup_game_memory()
{
    if (map != NULL)
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            free(map[i]);
        }
        free(map);
        map = NULL;
    }
    if (temp_map != NULL)
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            free(temp_map[i]);
        }
        free(temp_map);
        temp_map = NULL;
    }
    num_rooms = 0;
    if (discovered_map != NULL)
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            free(discovered_map[i]);
        }
        free(discovered_map);
        discovered_map = NULL;
    }
    for (int f = 0; f < 4; f++)
    {
        for (int i = 0; i < enemy_counts[f]; i++)
        {
            if (enemies[f][i] != NULL)
            {
                free(enemies[f][i]);
                enemies[f][i] = NULL;
            }
        }
        enemy_counts[f] = 0;
    }
}

void generate_map()
{
    srand(time(NULL));

    int section_width = WIDTH / 3;
    int section_height = HEIGHT / 2;

    int section_positions[6][2] = {{0, 0},
                                   {1, 0},
                                   {2, 0},
                                   {2, 1},
                                   {1, 1},
                                   {0, 1}};

    for (int f = 0; f < 4; f++)
    {
        Floor *current_floor = create_floor();

        for (int i = 0; i < 6; i++)
        {
            int col = section_positions[i][0];
            int row = section_positions[i][1];

            int section_start_x = col * section_width;
            int section_start_y = row * section_height;

            int max_width = section_width - 4;
            int max_height = section_height - 4;
            int width = MIN_ROOM_SIZE + rand() % (max_width - MIN_ROOM_SIZE);
            int height = MIN_ROOM_SIZE + rand() % (max_height - MIN_ROOM_SIZE);

            int x = section_start_x + 2 + rand() % (section_width - width - 4);
            int y = section_start_y + 2 + rand() % (section_height - height - 4);

            Room room = {x, y, width, height, 0, 0, 0, 0};
            add_room(room, i, current_floor);
        }

        connect_rooms(current_floor);

        if (f > 0)
        {

            int key_room = 1 + rand() % 4;
            Room room = current_floor->rooms[key_room];
            int attempts = 0;
            bool key_placed = false;
            while (!key_placed && attempts < 50)
            {
                int key_x = room.x + 1 + rand() % (room.width - 2);
                int key_y = room.y + 1 + rand() % (room.height - 2);

                if (current_floor->map[key_y][key_x] == '.' &&
                    current_floor->map[key_y][key_x - 1] != '%' &&
                    current_floor->map[key_y][key_x - 2] != '%')
                {
                    current_floor->map[key_y][key_x] = 'K';
                    key_placed = true;
                }
                attempts++;
            }
        }

        if (f < 3)
        {
            Room last_room = current_floor->rooms[5];
            int stair_x = last_room.x + 1 + rand() % (last_room.width - 2);
            int stair_y = last_room.y + 1 + rand() % (last_room.height - 2);
            current_floor->rooms[5].up_stair_x = stair_x;
            current_floor->rooms[5].up_stair_y = stair_y;
            current_floor->map[stair_y][stair_x] = '<';
        }

        if (f > 0)
        {
            Room first_room = current_floor->rooms[0];
            int stair_x = first_room.x + 1 + rand() % (first_room.width - 2);
            int stair_y = first_room.y + 1 + rand() % (first_room.height - 2);
            current_floor->rooms[0].down_stair_x = stair_x;
            current_floor->rooms[0].down_stair_y = stair_y;
            current_floor->map[stair_y][stair_x] = '>';
        }

        if (f == 3)
        {
            Room last_room = current_floor->rooms[5];
            int crown_x = last_room.x + 1 + rand() % (last_room.width - 2);
            int crown_y = last_room.y + 1 + rand() % (last_room.height - 2);

            if (current_floor->map[crown_y][crown_x] == '.')
            {
                current_floor->map[crown_y][crown_x] = 'C';
            }
        }

        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                current_floor->temp_map[i][j] = current_floor->map[i][j];
            }
        }

        floors[f] = *current_floor;

        Room first_room = floors[f].rooms[0];
        for (int y = first_room.y; y < first_room.y + first_room.height; y++)
        {
            for (int x = first_room.x; x < first_room.x + first_room.width; x++)
            {
                floors[f].map[y][x] = floors[f].temp_map[y][x];
            }
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            for (int x = 0; x < WIDTH; x++)
            {
                floors[f].map[y][x] = floors[f].temp_map[y][x];
            }
        }

        free(current_floor);
    }
}

void switch_floor()
{
    cleanup_game_memory();
    initialize_map();
    initialize_enemies();

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            map[y][x] = floors[current_floor].map[y][x];
            temp_map[y][x] = floors[current_floor].temp_map[y][x];
            discovered_map[y][x] = floors[current_floor].discovered_map[y][x];
        }
    }

    if (current_floor > 0 && map[hero_y][hero_x] == '>')
    {
        hero_x = floors[current_floor].rooms[0].up_stair_x;
        hero_y = floors[current_floor].rooms[0].up_stair_y;
    }
    else if (current_floor < 3 && map[hero_y][hero_x] == '<')
    {
        hero_x = floors[current_floor].rooms[5].down_stair_x;
        hero_y = floors[current_floor].rooms[5].down_stair_y;
    }
    if (!floors[current_floor].floor_discovered)
    {
        Room first_room = floors[current_floor].rooms[0];
        for (int y = first_room.y; y < first_room.y + first_room.height; y++)
        {
            for (int x = first_room.x; x < first_room.x + first_room.width; x++)
            {
                discovered_map[y][x] = 1;
                floors[current_floor].discovered_map[y][x] = 1;
            }
        }
        floors[current_floor].floor_discovered = true;
    }

    draw_map();

    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            temp_map[i][j] = map[i][j];
        }
    }
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (floors[current_floor].temp_map[i][j] == '?')
            {
                temp_map[i][j] = floors[current_floor].temp_map[i][j];
            }
        }
    }
}

void draw_map()
{
    start_color();
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(LINES - 1, i, " ");
    }
    mvprintw(LINES - 1, 2, "Health: %d/100 Hunger: %d/100", health, hunger);
    mvprintw(LINES - 1, 33, "Level: %d/4", current_floor + 1);
    mvprintw(LINES - 1, 45, "Gold: %d", bag.gold_count);
    init_pair(167, COLOR_YELLOW, COLOR_BLACK);
    init_pair(70, COLOR_BLUE, COLOR_BLACK);

    if (time(NULL) < speed_spell_end_time)
    {
        int remaining_time = speed_spell_end_time - time(NULL);
        mvprintw(LINES - 1, 60, "Speed Spell: %d s", remaining_time);
    }
    else
    {
        int remaining_time = 0;
        mvprintw(LINES - 1, 60, "Speed Spell: %d s", remaining_time);
    }
    if (time(NULL) < damage_spell_end_time)
    {
        int remaining_time = damage_spell_end_time - time(NULL);
        mvprintw(LINES - 1, 79, "Damage Spell: %d s", remaining_time);
    }
    else
    {
        int remaining_time = 0;
        mvprintw(LINES - 1, 79, "Damage Spell: %d s", remaining_time);
    }
    if (time(NULL) < health_spell_end_time)
    {
        int remaining_time = health_spell_end_time - time(NULL);
        mvprintw(LINES - 1, 98, "Health Spell: %d s", remaining_time);
    }
    else
    {
        int remaining_time = 0;
        mvprintw(LINES - 1, 98, "Damage Spell: %d s", remaining_time);
    }

    draw_enemies();
    for (int i = 1; i < HEIGHT - 1; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (!discovered_map[i][j])
            {
                mvaddch(i, j, ' ');
                continue;
            }

            if (map[i][j] == '@')
            {
                init_pair(6, current_hero_color == 1 ? COLOR_WHITE : current_hero_color == 2 ? COLOR_RED
                                                                 : current_hero_color == 3   ? COLOR_GREEN
                                                                 : current_hero_color == 4   ? COLOR_BLUE
                                                                 : current_hero_color == 5   ? COLOR_MAGENTA
                                                                                             : COLOR_CYAN,
                          COLOR_BLACK);
                attron(COLOR_PAIR(6));
                mvaddch(i, j, map[i][j]);
                attroff(COLOR_PAIR(6));
            }
            else if (map[i][j] == '%')
            {
                char emoji[] = "🌮";
                mvprintw(i, j - 1, "%s", emoji);
            }
            else if (map[i][j] == 'K')
            {
                char key[] = "∆";
                attron(COLOR_PAIR(167));
                mvprintw(i, j, "%s", key);
                attroff(COLOR_PAIR(167));
            }
            else if (map[i][j] == 'b')
            {
                init_pair(81, COLOR_BLACK, COLOR_YELLOW);
                attron(COLOR_PAIR(81));
                mvaddch(i, j, '$');
                attroff(COLOR_PAIR(81));
            }
            else if (map[i][j] == '~')
            {
                char spell[] = "💨";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if (map[i][j] == 'H')
            {
                char spell[] = "💊";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if (map[i][j] == '!')
            {
                char spell[] = "💀";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if (map[i][j] == 'd' || map[i][j] == '"')
            {
                char dagger[] = "◊";
                mvprintw(i, j, "%s", dagger);
            }
            else if (map[i][j] == '*' || map[i][j] == 'W')
            {
                attron(COLOR_PAIR(9));
                char wand[] = "*";
                attroff(COLOR_PAIR(9));
                mvprintw(i, j, "%s", wand);
            }
            else if (map[i][j] == 'A' || map[i][j] == 'a')
            {
                char arrow[] = "⪧";
                mvprintw(i, j, "%s", arrow);
            }
            else if (map[i][j] == 'C')
            {
                char crown[] = "👑";
                mvprintw(i, j - 1, "%s", crown);
            }
            else if (map[i][j] == 's')
            {
                char sword[] = "🔪";
                mvprintw(i, j - 1, "%s", sword);
            }
            else
            {
                mvaddch(i, j, map[i][j]);
            }
        }
    }

    if (!final_room)
    {
        for (int r = 0; r < 6; r++)
        {
            Room *room = &floors[current_floor].rooms[r];

            int x = (r < 3) ? room->right_door_x : room->left_door_x;
            int y = (r < 3) ? room->right_door_y : room->left_door_y;

            if (floors[current_floor].temp_map[y][x] == '?' && discovered_map[room->y + 2][room->x + 2])
            {
                mvaddch(y, x, floors[current_floor].map[y][x]);
            }

            if (room->is_room_security)
            {
                int door_x = (r < 3) ? room->right_door_x : room->left_door_x;
                int door_y = (r < 3) ? room->right_door_y : room->left_door_y;

                if (discovered_map[door_y][door_x])
                {
                    init_pair(8, COLOR_RED, COLOR_BLACK);
                    init_pair(9, COLOR_GREEN, COLOR_BLACK);
                    if (room->is_room_locked)
                    {
                        attron(COLOR_PAIR(8));
                        mvaddch(door_y, door_x, '@');
                        attroff(COLOR_PAIR(8));
                    }
                    else
                    {
                        attron(COLOR_PAIR(9));
                        mvaddch(door_y, door_x, '@');
                        attroff(COLOR_PAIR(9));
                    }
                }
            }

            if (room->is_room_nightmare)
            {
                attron(COLOR_PAIR(70));
                for (int y = room->y; y < room->y + room->height; y++)
                {
                    for (int x = room->x; x < room->x + room->width; x++)
                    {
                        if (discovered_map[y][x])
                        {
                            mvaddch(y, x, map[y][x]);
                        }
                    }
                }
                attroff(COLOR_PAIR(70));
            }

            if (room->is_room_enchanted && discovered_map[room->y][room->x])
            {
                init_pair(112, COLOR_CYAN, COLOR_BLACK);
                attron(COLOR_PAIR(112));
                for (int y = room->y; y < room->y + room->height; y++)
                {
                    for (int x = room->x; x < room->x + room->width; x++)
                    {
                        mvaddch(y, x, map[y][x]);
                    }
                }
                attroff(COLOR_PAIR(112));
            }

            if (room->is_room_treasure && discovered_map[room->y][room->x])
            {
                attron(COLOR_PAIR(9));
                for (int y = room->y; y < room->y + room->height; y++)
                {
                    for (int x = room->x; x < room->x + room->width; x++)
                    {
                        mvaddch(y, x, map[y][x]);
                    }
                }
                attroff(COLOR_PAIR(9));
            }
        }
    }
    for (int i = 1; i < HEIGHT - 1; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            if (map[i][j] == '@' && discovered_map[i][j])
            {
                init_pair(6, current_hero_color == 1 ? COLOR_WHITE : current_hero_color == 2 ? COLOR_RED
                                                                 : current_hero_color == 3   ? COLOR_GREEN
                                                                 : current_hero_color == 4   ? COLOR_BLUE
                                                                 : current_hero_color == 5   ? COLOR_MAGENTA
                                                                                             : COLOR_CYAN,
                          COLOR_BLACK);
                attron(COLOR_PAIR(6));
                mvaddch(i, j, map[i][j]);
                attroff(COLOR_PAIR(6));
            }
            else if (map[i][j] == '%' && discovered_map[i][j])
            {
                char emoji[5] = "🌮";
                mvprintw(i, j - 1, "%s", emoji);
            }
            else if (map[i][j] == 'K' && discovered_map[i][j])
            {
                char key[5] = "∆";
                attron(COLOR_PAIR(167));
                mvprintw(i, j, "%s", key);
                attroff(COLOR_PAIR(167));
            }
            else if (map[i][j] == 'b' && discovered_map[i][j])
            {
                init_pair(81, COLOR_BLACK, COLOR_YELLOW);
                attron(COLOR_PAIR(81));
                mvaddch(i, j, '$');
                attroff(COLOR_PAIR(81));
            }
            else if (map[i][j] == '~' && discovered_map[i][j])
            {
                char spell[] = "💨";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if (map[i][j] == 'H' && discovered_map[i][j])
            {
                char spell[] = "💊";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if (map[i][j] == '!' && discovered_map[i][j])
            {
                char spell[] = "💀";
                mvprintw(i, j - 1, "%s", spell);
            }
            else if ((map[i][j] == 'd' || map[i][j] == '"') && discovered_map[i][j])
            {
                char dagger[] = "◊";
                mvprintw(i, j, "%s", dagger);
            }
            else if ((map[i][j] == '*' || map[i][j] == 'W') && discovered_map[i][j])
            {
                attron(COLOR_PAIR(9));
                char wand[] = "*";
                attroff(COLOR_PAIR(9));
                mvprintw(i, j, "%s", wand);
            }
            else if ((map[i][j] == 'A' || map[i][j] == 'a') && discovered_map[i][j])
            {
                char arrow[] = "⪧";
                mvprintw(i, j, "%s", arrow);
            }
            else if (map[i][j] == 'C' && discovered_map[i][j])
            {
                char crown[] = "👑";
                mvprintw(i, j - 1, "%s", crown);
            }
            else if (map[i][j] == 's' && discovered_map[i][j])
            {
                char sword[] = "🔪";
                mvprintw(i, j - 1, "%s", sword);
            }
            else if (map[i][j] == 'm' && discovered_map[i][j])
            {
                char weapon[] = "⚒︎";
                mvprintw(i, j - 1, "%s", weapon);
            }
        }
    }
    for (int r = 0; r < 6; r++)
    {
        Room *room = &floors[current_floor].rooms[r];
        if (room->is_room_security)
        {
            int door_x = (r < 3) ? room->right_door_x : room->left_door_x;
            int door_y = (r < 3) ? room->right_door_y : room->left_door_y;

            if (discovered_map[door_y][door_x])
            {
                init_pair(8, COLOR_RED, COLOR_BLACK);
                init_pair(9, COLOR_GREEN, COLOR_BLACK);
                if (room->is_room_locked)
                {
                    attron(COLOR_PAIR(8));
                    mvaddch(door_y, door_x, '@');
                    attroff(COLOR_PAIR(8));
                }
                else
                {
                    attron(COLOR_PAIR(9));
                    mvaddch(door_y, door_x, '@');
                    attroff(COLOR_PAIR(9));
                }
            }
        }
    }
    refresh();
}

void save_game(char username[], int score)
{
    FILE *scores = fopen("scores.txt", "r");
    FILE *temp = fopen("temp_scores.txt", "w");

    char current_username[100];
    int current_score;
    bool found = false;
    char line[256];

    while (scores && fgets(line, sizeof(line), scores))
    {
        sscanf(line, "%s %d", current_username, &current_score);
        if (strcmp(current_username, username) == 0)
        {
            fprintf(temp, "%s %d\n", username, score);
            found = true;
        }
        else
        {
            fprintf(temp, "%s", line);
        }
    }

    if (!found)
    {
        fprintf(temp, "%s %d\n", username, score);
    }

    if (scores)
    {
        fclose(scores);
        fclose(temp);
        remove("scores.txt");
        rename("temp_scores.txt", "scores.txt");
    }

    char save_path[256];
    sprintf(save_path, "%s_save.dat", username);
    FILE *save_file = fopen(save_path, "wb");

    SaveMetadata metadata;
    strcpy(metadata.username, username);
    metadata.save_time = time(NULL);
    metadata.score = score;
    metadata.current_hero_color = current_hero_color;
    metadata.current_floor = current_floor;

    metadata.current_room = 0;
    for (int i = 0; i < floors[current_floor].num_rooms; i++)
    {
        if (hero_x >= floors[current_floor].rooms[i].x &&
            hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
            hero_y >= floors[current_floor].rooms[i].y &&
            hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
        {
            metadata.current_room = i + 1;
            break;
        }
    }

    fwrite(&metadata, sizeof(SaveMetadata), 1, save_file);
    fwrite(&HEIGHT, sizeof(int), 1, save_file);
    fwrite(&WIDTH, sizeof(int), 1, save_file);
    fwrite(&hero_x, sizeof(int), 1, save_file);
    fwrite(&hero_y, sizeof(int), 1, save_file);

    for (int f = 0; f < 4; f++)
    {
        fwrite(&floors[f].num_rooms, sizeof(int), 1, save_file);
        fwrite(floors[f].rooms, sizeof(Room), floors[f].num_rooms, save_file);

        for (int y = 0; y < HEIGHT; y++)
        {
            fwrite(floors[f].map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fwrite(floors[f].temp_map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fwrite(floors[f].discovered_map[y], sizeof(char), WIDTH, save_file);
        }

        fwrite(&floors[f].floor_discovered, sizeof(bool), 1, save_file);
    }

    for (int f = 0; f < 4; f++)
    {
        fwrite(&enemy_counts[f], sizeof(int), 1, save_file);

        for (int i = 0; i < enemy_counts[f]; i++)
        {
            if (enemies[f][i] != NULL && enemies[f][i]->active)
            {
                bool exists = true;
                fwrite(&exists, sizeof(bool), 1, save_file);

                fwrite(&enemies[f][i]->symbol, sizeof(char), 1, save_file);
                fwrite(&enemies[f][i]->x, sizeof(int), 1, save_file);
                fwrite(&enemies[f][i]->y, sizeof(int), 1, save_file);
                fwrite(&enemies[f][i]->health, sizeof(int), 1, save_file);
                fwrite(&enemies[f][i]->damage, sizeof(int), 1, save_file);
                fwrite(&enemies[f][i]->active, sizeof(bool), 1, save_file);
                fwrite(&enemies[f][i]->moves_made, sizeof(int), 1, save_file);
                fwrite(&enemies[f][i]->can_move, sizeof(bool), 1, save_file);
            }
            else
            {
                bool exists = false;
                fwrite(&exists, sizeof(bool), 1, save_file);
            }
        }
    }

    fwrite(&bag, sizeof(Bag), 1, save_file);

    fclose(save_file);
    char metadata_path[256];
    sprintf(metadata_path, "%s_info.txt", username);
    FILE *metadata_file = fopen(metadata_path, "w");
    if (metadata_file != NULL)
    {
        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&metadata.save_time));
        fprintf(metadata_file, "Last Save: %s\n", time_str);
        fprintf(metadata_file, "Score: %d\n", metadata.score);
        fprintf(metadata_file, "Current Room: %d/6\n", metadata.current_room);
        fprintf(metadata_file, "Current Floor: %d/4\n", metadata.current_floor + 1);
        fprintf(metadata_file, "Hero Color: %d\n", metadata.current_hero_color);
        fclose(metadata_file);
    }
}

int load_game(char username[], int *score)
{
    char save_path[256];
    sprintf(save_path, "%s_save.dat", username);

    FILE *save_file = fopen(save_path, "rb");
    if (save_file == NULL)
    {
        return 0;
    }

    SaveMetadata metadata;
    fread(&metadata, sizeof(SaveMetadata), 1, save_file);
    *score = metadata.score;
    current_hero_color = metadata.current_hero_color;
    current_floor = metadata.current_floor;

    fread(&HEIGHT, sizeof(int), 1, save_file);
    fread(&WIDTH, sizeof(int), 1, save_file);
    fread(&hero_x, sizeof(int), 1, save_file);
    fread(&hero_y, sizeof(int), 1, save_file);

    for (int f = 0; f < 4; f++)
    {
        if (floors[f].map == NULL)
        {
            floors[f].map = (char **)malloc(HEIGHT * sizeof(char *));
            floors[f].temp_map = (char **)malloc(HEIGHT * sizeof(char *));
            floors[f].discovered_map = (char **)malloc(HEIGHT * sizeof(char *));
            for (int i = 0; i < HEIGHT; i++)
            {
                floors[f].map[i] = (char *)malloc(WIDTH * sizeof(char));
                floors[f].temp_map[i] = (char *)malloc(WIDTH * sizeof(char));
                floors[f].discovered_map[i] = (char *)malloc(WIDTH * sizeof(char));
            }
        }
        for (int f = 0; f < 4; f++)
        {
            for (int i = 0; i < MAX_ENEMIES_PER_FLOOR; i++)
            {
                if (enemies[f][i] != NULL)
                {
                    free(enemies[f][i]);
                    enemies[f][i] = NULL;
                }
            }
        }

        fread(&floors[f].num_rooms, sizeof(int), 1, save_file);
        fread(floors[f].rooms, sizeof(Room), floors[f].num_rooms, save_file);

        for (int y = 0; y < HEIGHT; y++)
        {
            fread(floors[f].map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fread(floors[f].temp_map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fread(floors[f].discovered_map[y], sizeof(char), WIDTH, save_file);
        }

        fread(&floors[f].floor_discovered, sizeof(bool), 1, save_file);
    }

    cleanup_game_memory();
    initialize_map();

    for (int y = 0; y < HEIGHT; y++)
    {
        memcpy(map[y], floors[current_floor].map[y], WIDTH * sizeof(char));
        memcpy(temp_map[y], floors[current_floor].temp_map[y], WIDTH * sizeof(char));
        memcpy(discovered_map[y], floors[current_floor].discovered_map[y], WIDTH * sizeof(char));
    }

    for (int f = 0; f < 4; f++)
    {
        fread(&enemy_counts[f], sizeof(int), 1, save_file);

        for (int i = 0; i < enemy_counts[f]; i++)
        {
            bool exists;
            fread(&exists, sizeof(bool), 1, save_file);

            if (exists)
            {
                enemies[f][i] = (Enemy *)malloc(sizeof(Enemy));
                if (enemies[f][i] == NULL)
                {
                    mvprintw(0, 0, "Failed to allocate memory for enemy");
                    refresh();
                    return 0;
                }

                fread(&enemies[f][i]->symbol, sizeof(char), 1, save_file);
                fread(&enemies[f][i]->x, sizeof(int), 1, save_file);
                fread(&enemies[f][i]->y, sizeof(int), 1, save_file);
                fread(&enemies[f][i]->health, sizeof(int), 1, save_file);
                fread(&enemies[f][i]->damage, sizeof(int), 1, save_file);
                fread(&enemies[f][i]->active, sizeof(bool), 1, save_file);
                fread(&enemies[f][i]->moves_made, sizeof(int), 1, save_file);
                fread(&enemies[f][i]->can_move, sizeof(bool), 1, save_file);
            }
            else
            {
                enemies[f][i] = NULL;
            }
        }
    }

    fread(&bag, sizeof(Bag), 1, save_file);
    fclose(save_file);
    return 1;
}

void check_save_exists(char username[])
{
    char save_path[256];
    sprintf(save_path, "%s_save.dat", username);

    FILE *save_file = fopen(save_path, "rb");
    if (save_file != NULL)
    {
        SaveMetadata metadata;
        fread(&metadata, sizeof(SaveMetadata), 1, save_file);
        fclose(save_file);

        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&metadata.save_time));

        mvprintw(LINES / 2 - 1, COLS / 2 - 25, "Found saved game from: %s", time_str);
        mvprintw(LINES / 2, COLS / 2 - 25, "Score: %d  |  Room: %d/6", metadata.score, metadata.current_room);
        mvprintw(LINES / 2 + 1, COLS / 2 - 25, "Press 'L' to load or any other key to return");
        refresh();

        int ch = getch();
        if (toupper(ch) == 'L')
        {
            int hero_x, hero_y, score;
            if (load_game(username, &score))
            {
                continue_game(username);
                return;
            }
        }
        else
        {
            pre_game_menu(username);
        }
    }

    mvprintw(2, COLS / 2 - 14, "you dont have a saved game!");
    return;
}