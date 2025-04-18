//#test_maps + floor_machanism 

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define MIN_ROOM_SIZE 4
#define MAX_ROOM_SIZE 6

typedef struct
{
    int x, y, width, height;
    int left_door_x, left_door_y;
    int right_door_x, right_door_y;
} Room;

char **map;
char **temp_map;
int WIDTH, HEIGHT;
int current_floor;

char **map;
int WIDTH, HEIGHT;
Room rooms[6];
int num_rooms = 0;
typedef struct
{
    char **map;
    char **temp_map;
    Room rooms[6];
    int num_rooms;
} Floor;
Floor floors[4];

Floor *create_floor()
{
    Floor *floor = (Floor *)malloc(sizeof(Floor));

    floor->map = (char **)malloc(HEIGHT * sizeof(char *));
    floor->temp_map = (char **)malloc(HEIGHT * sizeof(char *));
    for (int i = 0; i < HEIGHT; i++)
    {
        floor->map[i] = (char *)malloc(WIDTH * sizeof(char));
        floor->temp_map[i] = (char *)malloc(WIDTH * sizeof(char));
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

    return floor;
}

void init_floor_map(Floor *floor)
{
    if (floor == NULL || floor->map == NULL)
    {
        return;
    }

    for (int y = 0; y < HEIGHT + 2; y++)
        for (int x = 0; x < WIDTH; x++)
            floor->map[y][x] = ' ';
}

void draw_health_bar(int health)
{
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    int new_health = health / 10;
    for (int i = 0; i < new_health; i++)
    {
        attron(COLOR_PAIR(1));
        mvprintw(2, i + 1, "҈");
    }
    attroff(COLOR_PAIR(1));
    for (int i = new_health; i < 10; i++)
    {
        attron(COLOR_PAIR(2));
        mvprintw(2, i + 1, "҈");
    }
    attroff(COLOR_PAIR(2));
}

void init_map()
{
    for (int y = 0; y < HEIGHT; y++)
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

    int num_pillars = rand() % 3 + 1;
    for (int i = 0; i < num_pillars; i++)
    {
        int pillar_x = room.x + 1 + rand() % (room.width - 2);
        int pillar_y = room.y + 1 + rand() % (room.height - 2);

        if (floor->map[pillar_y][pillar_x] == '.')
            floor->map[pillar_y][pillar_x] = 'O';
    }

    int num_emojis = rand() % 2 + 1;
    for (int i = 0; i < num_emojis; i++)
    {
        int pillar_x = room.x+ 1  + rand() % (room.width - 4);
        int pillar_y = room.y + 1 + rand() % (room.height - 4);

        if (floor->map[pillar_y][pillar_x] == '.' &&
            floor->map[pillar_y][pillar_x - 1] != '%')
        {
            floor->map[pillar_y][pillar_x] = '%';
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

    if (room_index == 5)
    {
        int stair_x = room.x;
        int stair_y = room.y + 1 + rand() % (room.height - 2);
        floor->map[stair_y][stair_x] = '<';
    }

    add_door(&room, room_index, floor);
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
}

void initialize_map()
{
    map = malloc(HEIGHT * sizeof(char *));
    temp_map = malloc(HEIGHT * sizeof(char *));
    for (int i = 0; i < HEIGHT; i++)
    {
        map[i] = malloc(WIDTH * sizeof(char));
        temp_map[i] = malloc(WIDTH * sizeof(char));
    }
}

void generate_map()
{
    srand(time(NULL));

    int section_width = WIDTH / 3;
    int section_height = HEIGHT / 2;

    int section_positions[6][2] = {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {1, 1}, {0, 1}};


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

        // Add stairs
        if (f < 3)
        {
            // Add down stairs '>' in a random position in the last room
            Room last_room = current_floor->rooms[5];
            int stair_x = last_room.x + 1 + rand() % (last_room.width - 2);
            int stair_y = last_room.y + 1 + rand() % (last_room.height - 2);
            // current_floor->map[stair_y][stair_x] = '<';
        }

        if (f > 0)
        {
            Room first_room = current_floor->rooms[0];
            int stair_x = first_room.x + 1 + rand() % (first_room.width - 2);
            int stair_y = first_room.y + 1 + rand() % (first_room.height - 2);
            // current_floor->map[stair_y][stair_x] = '>';
        }

        floors[f] = *current_floor;
        free(current_floor);
    }
}

void switch_floor()
{
    cleanup_game_memory();
    initialize_map();
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {

            map[y][x] = floors[current_floor].map[y][x];
            temp_map[y][x] = floors[current_floor].temp_map[y][x];
        }
    }
}

///// this is for saving floors
void save_floors(char username[])
{
    char save_path[256];
    sprintf(save_path, "%s_floors.dat", username);

    FILE *save_file = fopen(save_path, "wb");
    if (save_file == NULL)
        return;

    fwrite(&current_floor, sizeof(int), 1, save_file);

    for (int f = 0; f < 4; f++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            fwrite(floors[f].map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fwrite(floors[f].temp_map[y], sizeof(char), WIDTH, save_file);
        }

        fwrite(&floors[f].num_rooms, sizeof(int), 1, save_file);
        fwrite(floors[f].rooms, sizeof(Room), floors[f].num_rooms, save_file);
    }

    fclose(save_file);
}

int load_floors(char username[])
{
    char save_path[256];
    sprintf(save_path, "%s_floors.dat", username);

    FILE *save_file = fopen(save_path, "rb");
    if (save_file == NULL)
        return 0;

    fread(&current_floor, sizeof(int), 1, save_file);

    for (int f = 0; f < 4; f++)
    {
        if (floors[f].map == NULL)
        {
            Floor *new_floor;
            init_floor_map(new_floor);
            floors[f] = *new_floor;
            free(new_floor);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fread(floors[f].map[y], sizeof(char), WIDTH, save_file);
        }

        for (int y = 0; y < HEIGHT; y++)
        {
            fread(floors[f].temp_map[y], sizeof(char), WIDTH, save_file);
        }

        fread(&floors[f].num_rooms, sizeof(int), 1, save_file);
        fread(floors[f].rooms, sizeof(Room), floors[f].num_rooms, save_file);
    }

    fclose(save_file);
    return 1;
}

void draw_map()
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            mvaddch(y, x, map[y][x]);
        }
    }
}

int main()
{
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    getmaxyx(stdscr, HEIGHT, WIDTH);
    printw("Width: %d, Height: %d", WIDTH, HEIGHT);
    refresh();
    getch();

    map = (char **)malloc(HEIGHT * sizeof(char *));
    for (int i = 0; i < HEIGHT; i++)
    {
        map[i] = (char *)malloc(WIDTH * sizeof(char));
    }
    
    init_map();
    generate_map();

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            map[y][x] = floors[current_floor].map[y][x];
        }
    }

    int hero_x, hero_y, score = 0;

    draw_map();
    while (1) {
        char ch = getch();
        if (ch == '<')
        {
            current_floor++;
            switch_floor();
            draw_map();
        }
        if (toupper(ch) == '>' && current_floor > 0)
        {
            current_floor--;
            switch_floor();
            draw_map();
        }
        if(ch == 27) {break;}
    }
    endwin();
    return 0;
}
