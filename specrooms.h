#include <ncurses.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#define MIN_ROOM_SIZE 10
#define MAX_ROOM_SIZE 10
#define MAX_PROJECTILES 20
#define FINAL_ROOM_ENEMIES 10
#define MAX_ENEMIES_PER_FLOOR 10

char **discovered_map;
int current_floor = 0;
int difficulty = 1;
int health = 100;
int hunger = 0;
void change_appearance(char username[], int *hero_x, int *hero_y, int *score);
void draw_color_menu(int current_selection);
void leader_board(int from_game, char username[], int hero_x, int hero_y, int score);
bool security_mode = false;
int spell_type = 0;
int weapon_selected = 0;
int frost = 0;
int hero_y, hero_x;
int enemy_counts[5];
int current_hero_color = 1;
char **map;
char **temp_map;
int WIDTH, HEIGHT;
int num_rooms = 0;
bool final_room = false;
int games_played = 0;


struct Enemy;
typedef void (*EnemyAbility)(struct Enemy *);

typedef struct
{
    int has_ancient_key;
    int gold_count;
    int speed_spells;
    int health_spells;
    int damage_spells;
    int broke_key;

    int total_spells;
    int total_foods;

    bool mace;
    bool magic_wand;
    bool dagger;
    bool normal_arrow;
    bool sword;

    int dagger_count;
    int wand_count;
    int arrow_count;

    bool health_spell;
    bool speed_spell;
    bool damage_spell;

    time_t food_pickup[5];
    int food_type[5];
} Bag;
Bag bag;

typedef struct
{
    char username[100];
    time_t save_time;
    int score;
    int current_room;
    int current_floor;
    int current_hero_color;
} SaveMetadata;

typedef struct
{
    int x, y, width, height;
    int left_door_x, left_door_y;
    int right_door_x, right_door_y;
    int up_stair_x, up_stair_y;
    int down_stair_x, down_stair_y;
    bool is_room_nightmare;
    bool is_room_locked;
    bool is_room_old;
    bool is_room_security;
    bool is_room_door_discovered;
    bool is_room_enchanted;
    bool is_room_treasure;
    int sec_level;
    int password;
} Room;
Room rooms[6];

typedef struct
{
    char name[100];
    int count;
    int x;
    int y;
    int room;
} spells;

typedef struct
{
    char **map;
    char **temp_map;
    char **discovered_map;
    Room rooms[6];
    int num_rooms;
    bool floor_discovered;
} Floor;
Floor floors[4];

typedef struct Enemy
{
    char symbol;
    int x;
    int y;
    int health;
    int damage;
    bool active;
    EnemyAbility special_ability;
    int moves_made;
    bool can_move;
} Enemy;

Enemy *enemies[5][MAX_ENEMIES_PER_FLOOR];


void undead_ability(Enemy *e)
{
    if (e->health <= 0 && rand() % 100 < 30)
    {
        e->health = 30;
        e->active = true;
        mvprintw(0, 2, "Undead rises again!");
        refresh();
    }
}

void initialize_enemies()
{
    for (int f = 0; f < 5; f++)
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

    for (int floor = 0; floor < 4; floor++)
    {
        int enemies_placed = 0;
        enemy_counts[floor] = 0;

        for (int room = 0; room < floors[floor].num_rooms; room++)
        {
            Enemy *e = (Enemy *)malloc(sizeof(Enemy));
            if (e == NULL)
            {
                mvprintw(0, 0, "Failed to allocate memory for enemy");
                refresh();
                return;
            }

            e->moves_made = 0;
            e->can_move = true;
            e->active = true;
            e->x = 0;
            e->y = 0;

            int type = rand() % 5;
            switch (type)
            {
            case 0:
                e->symbol = 'D';
                e->damage = 2;
                e->health = 5;
                break;
            case 1:
                e->symbol = 'F';
                e->damage = 4;
                e->health = 10;
                break;
            case 2:
                e->symbol = 'G';
                e->damage = 5;
                e->health = 15;
                break;
            case 3:
                e->symbol = 'S';
                e->damage = 6;
                e->health = 20;
                break;
            case 4:
                e->symbol = 'U';
                e->damage = 8;
                e->health = 30;
                e->special_ability = undead_ability;
                break;
            }

            Room *r = &floors[floor].rooms[room];

            int max_attempts = 100;
            int attempts = 0;
            bool position_found = false;

            while (attempts < max_attempts)
            {
                int test_x = r->x + (rand() % (r->width - 2)) + 1;
                int test_y = r->y + (rand() % (r->height - 2)) + 1;

                if (floors[floor].map[test_y][test_x] == '.')
                {
                    e->x = test_x;
                    e->y = test_y;
                    floors[floor].map[test_y][test_x] = e->symbol;
                    position_found = true;
                    break;
                }
                attempts++;
            }

            if (!position_found)
            {
                e->active = false;
                free(e);
                mvprintw(0, 0, "Warning: Could not place enemy in room %d on floor %d", room, floor);
                refresh();
                continue;
            }

            enemies[floor][enemies_placed] = e;
            enemies_placed++;
            enemy_counts[floor]++;
        }

        int additional_enemies = (floor + 1) * 2; 
        for (int i = 0; i < additional_enemies && enemy_counts[floor] < MAX_ENEMIES_PER_FLOOR; i++)
        {
            Enemy *e = (Enemy *)malloc(sizeof(Enemy));
            if (e == NULL)
                continue;

            e->moves_made = 0;
            e->can_move = true;
            e->active = true;

            int type = rand() % 5;
            switch (type)
            {
            case 0:
                e->symbol = 'D';
                e->damage = 2;
                e->health = 5;
                break;
            case 1:
                e->symbol = 'F';
                e->damage = 4;
                e->health = 10;
                break;
            case 2:
                e->symbol = 'G';
                e->damage = 5;
                e->health = 15;
                break;
            case 3:
                e->symbol = 'S';
                e->damage = 6;
                e->health = 20;
                break;
            case 4:
                e->symbol = 'U';
                e->damage = 8;
                e->health = 30;
                e->special_ability = undead_ability;
                break;
            }

            int random_room = rand() % floors[floor].num_rooms;
            Room *r = &floors[floor].rooms[random_room];

            int max_attempts = 100;
            int attempts = 0;
            bool position_found = false;

            while (attempts < max_attempts)
            {
                int test_x = r->x + (rand() % (r->width - 2)) + 1;
                int test_y = r->y + (rand() % (r->height - 2)) + 1;

                if (floors[floor].map[test_y][test_x] == '.')
                {
                    e->x = test_x;
                    e->y = test_y;
                    floors[floor].map[test_y][test_x] = e->symbol;
                    floors[floor].temp_map[test_y][test_x] = e->symbol;
                    position_found = true;
                    break;
                }
                attempts++;
            }

            if (!position_found)
            {
                e->active = false;
                free(e);
                continue;
            }

            enemies[floor][enemies_placed] = e;
            enemies_placed++;
            enemy_counts[floor]++;
        }
    }
}

void is_in_same_room(int x1, int y1, int x2, int y2, bool *same_room)
{
    *same_room = false;
    for (int i = 0; i < floors[current_floor].num_rooms; i++)
    {
        Room *r = &floors[current_floor].rooms[i];
        bool point1_in_room = (x1 >= r->x && x1 < r->x + r->width &&
                               y1 >= r->y && y1 < r->y + r->height);
        bool point2_in_room = (x2 >= r->x && x2 < r->x + r->width &&
                               y2 >= r->y && y2 < r->y + r->height);

        if (point1_in_room && point2_in_room)
        {
            *same_room = true;
            return;
        }
    }
}

void update_enemies()
{
    if (final_room)
    {
        for (int i = 0; i < enemy_counts[4]; i++)
        {
            Enemy *e = enemies[4][i];

            if (e == NULL || !e->active || e->health <= 0)
            {
                continue;
            }

            int prev_x = e->x;
            int prev_y = e->y;
            bool moved = false;

            if (e->can_move)
            {
                int dx = 0;
                int dy = 0;

                if (hero_x > e->x)
                    dx = 1;
                else if (hero_x < e->x)
                    dx = -1;

                if (hero_y > e->y)
                    dy = 1;
                else if (hero_y < e->y)
                    dy = -1;

                if (dx != 0 && dy != 0)
                {
                    if (map[e->y + dy][e->x + dx] == '.')
                    {
                        e->x += dx;
                        e->y += dy;
                        moved = true;
                    }
                    else
                    {
                        if (map[e->y][e->x + dx] == '.')
                        {
                            e->x += dx;
                            moved = true;
                        }
                        else if (map[e->y + dy][e->x] == '.')
                        {
                            e->y += dy;
                            moved = true;
                        }
                    }
                }
                else
                {
                    if (map[e->y + dy][e->x + dx] == '.')
                    {
                        e->x += dx;
                        e->y += dy;
                        moved = true;
                    }
                }

                if (moved)
                {
                    map[prev_y][prev_x] = '.';
                    temp_map[prev_y][prev_x] = '.';
                    map[e->y][e->x] = e->symbol;
                    temp_map[e->y][e->x] = e->symbol;
                }
            }

            // Attack player if adjacent
            if (abs(e->x - hero_x) <= 1 && abs(e->y - hero_y) <= 1 && e->active)
            {
                int final_damage = e->damage;
                health -= final_damage;
                mvprintw(0, 2, "Boss enemy attack! -%d HP", final_damage);
                refresh();

                if (rand() % 100 < 50 && e->special_ability != NULL)
                {
                    e->special_ability(e);
                }
            }
        }
        return; 
    }

    for (int i = 0; i < enemy_counts[current_floor]; i++)
    {
        Enemy *e = enemies[current_floor][i];

        if (e == NULL)
        {
            continue;
        }

        if (e->health <= 0 || !e->active)
        {
            map[e->y][e->x] = '.';
            temp_map[e->y][e->x] = '.';
            e->active = false;
            continue;
        }

        bool same_room = false;
        is_in_same_room(e->x, e->y, hero_x, hero_y, &same_room);

        int prev_x = e->x;
        int prev_y = e->y;
        bool moved = false;

        bool should_pursue = current_floor == 4 ||
                             (e->symbol == 'S' && same_room) ||
                             (same_room && e->moves_made < 5);

        if (should_pursue && e->can_move)
        {
            int dx = 0;
            int dy = 0;

            if (hero_x > e->x)
                dx = 1;
            else if (hero_x < e->x)
                dx = -1;

            if (hero_y > e->y)
                dy = 1;
            else if (hero_y < e->y)
                dy = -1;

            if (dx != 0 && dy != 0)
            {
                if (map[e->y + dy][e->x + dx] == '.')
                {
                    e->x += dx;
                    e->y += dy;
                    moved = true;
                }
                else if (map[e->y][e->x + dx] == '.')
                {
                    e->x += dx;
                    moved = true;
                }
                else if (map[e->y + dy][e->x] == '.')
                {
                    e->y += dy;
                    moved = true;
                }
            }
            else
            {
                if (map[e->y + dy][e->x + dx] == '.')
                {
                    e->x += dx;
                    e->y += dy;
                    moved = true;
                }
            }

            if (moved)
            {
                floors[current_floor].map[prev_y][prev_x] = '.';
                floors[current_floor].temp_map[prev_y][prev_x] = '.';
                map[prev_y][prev_x] = '.';
                temp_map[prev_y][prev_x] = '.';
                map[e->y][e->x] = e->symbol;
                temp_map[e->y][e->x] = e->symbol;

                if (e->symbol != 'S')
                {
                    e->moves_made++;
                }
            }
        }

        if (abs(e->x - hero_x) <= 1 && abs(e->y - hero_y) <= 1 && e->active)
        {
            health -= e->damage;
            mvprintw(0, 2, "Enemy attack! -%d HP", e->damage);
            refresh();

            if (rand() % 100 < 30 && e->special_ability != NULL)
            {
                e->special_ability(e);
            }
        }

        if (e->symbol == 'U')
        {
            if (rand() % 100 < 30 && e->special_ability != NULL)
            {
                e->special_ability(e);
            }
        }
    }
}

void draw_enemies()
{
    for (int i = 0; i < enemy_counts[current_floor]; i++)
    {
        Enemy *e = enemies[current_floor][i];
        if (e == NULL || !e->active || e->health <= 0)
        {
            continue;
        }

        if ((!e->active || e->health <= 0) && discovered_map[e->y][e->x])
        {
            mvaddch(e->y, e->x, '.');
        }

        if (e->y < 0 || e->y >= HEIGHT || e->x < 0 || e->x >= WIDTH)
        {
            continue;
        }

        if (discovered_map[e->y][e->x])
        {
            mvaddch(e->y, e->x, e->symbol);
        }
    }
}

void make_room_security(int f, int i)
{
    Floor *floor = &floors[f];
    Room *room = &floors[f].rooms[i];

    int door_x = (i >= 3) ? room->left_door_x : room->right_door_x;
    int door_y = (i >= 3) ? room->left_door_y : room->right_door_y;

    room->is_room_locked = true;
    room->is_room_security = true;

    floor->map[door_y][door_x] = '@';
    floor->temp_map[door_y][door_x] = '@';

    int pass_gen_x = room->x + 1 + rand() % (room->width - 4);
    int pass_gen_y = room->y + 1 + rand() % (room->height - 4);

    floor->map[pass_gen_y][pass_gen_x] = '&';
    floor->temp_map[pass_gen_y][pass_gen_x] = '&';

    int is_room_old = rand() % 2;

    room->is_room_old = is_room_old;
    room->sec_level = 0;
}

void make_room_nightmare(int f, int r)
{
    Floor *floor = &floors[f];
    Room *room = &floor->rooms[r];

    room->is_room_nightmare = true;

    for (int i = 0; i < enemy_counts[f]; i++)
    {
        Enemy *e = enemies[f][i];
        if (!e->active)
        {
            e->active = true;
            e->symbol = 'g';
            e->damage = 5;
            e->health = 15;

            int max_attempts = 100;
            while (max_attempts > 0)
            {
                int test_x = room->x + (rand() % (room->width - 2)) + 1;
                int test_y = room->y + (rand() % (room->height - 2)) + 1;

                if (floor->map[test_y][test_x] == '.')
                {
                    e->x = test_x;
                    e->y = test_y;
                    floor->map[test_y][test_x] = e->symbol;
                    e->active = true;
                    break;
                }
                max_attempts--;
            }
        }
    }
}

void make_room_door_hidden(int f, int r)
{
    Floor *floor = &floors[f];
    Room *room = &floors[f].rooms[r];

    int door_x = (r >= 3) ? room->left_door_x : room->right_door_x;
    int door_y = (r >= 3) ? room->left_door_y : room->right_door_y;

    floor->temp_map[door_y][door_x] = '?';
    floor->map[door_y][door_x] = '|';
}

void make_spec_rooms()
{
    int num_of_security_rooms = 0;
    while (num_of_security_rooms <= 7)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        int num_of_security_rooms_in_floor = 0;
        for (int r = 0; r < 6; r++)
        {
            if (floors[random_floor].rooms[r].is_room_security && r != 5)
            {
                num_of_security_rooms++;
            }
        }
        if (num_of_security_rooms <= 1 &&
            !floors[random_floor].rooms[random_room].is_room_security && random_room != 5)
        {
            make_room_security(random_floor, random_room);
            num_of_security_rooms++;
        }
    }

    int nightmare_rooms_placed = 0;
    while (nightmare_rooms_placed < 3)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        Room *selected_room = &floors[random_floor].rooms[random_room];

        if (!selected_room->is_room_nightmare &&
            !selected_room->is_room_security &&
            !selected_room->is_room_enchanted &&
            random_room != 5)
        {

            make_room_nightmare(random_floor, random_room);
            nightmare_rooms_placed++;
        }
    }

    int hidden_room = 0;
    while (hidden_room < 3)
    {
        int random_floor = rand() % 4;
        int random_room = rand() % 6;
        Room *selected_room = &floors[random_floor].rooms[random_room];

        if (!selected_room->is_room_nightmare &&
            !selected_room->is_room_security &&
            !selected_room->is_room_enchanted &&
            random_room != 5)
        {

            make_room_door_hidden(random_floor, random_room);
            hidden_room++;

            floors[random_floor].rooms[random_room + 1].is_room_enchanted = true;
        }
    }
}
