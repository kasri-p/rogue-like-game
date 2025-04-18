#include "maps.h"
#include "music.h"
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>

#define PROJECTILE_HIT_ENEMY 1
#define PROJECTILE_MISSED 0

WINDOW *full_map_win = NULL;
bool show_full_map = false;
time_t last_hit_time;

bool check_final_room_victory()
{
    if (!final_room)
    {
        return false;
    }

    for (int i = 0; i < enemy_counts[4]; i++)
    {
        if (enemies[4][i] != NULL && enemies[4][i]->active)
        {
            return false;
        }
    }

    return true;
}

int get_movement_distance()
{
    if (time(NULL) < speed_spell_end_time)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

int get_damage_multiplier()
{
    if (time(NULL) < damage_spell_end_time)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

int get_health_restore_multiplier()
{
    if (time(NULL) < health_spell_end_time)
    {
        return 2;
    }
    else
    {
        return 1;
    }
}

void display_trophy_ascii(int start_y, int start_x)
{
    mvprintw(start_y, start_x, "       ___________      ");
    mvprintw(start_y + 1, start_x, "      '._==_==_=_.'     ");
    mvprintw(start_y + 2, start_x, "      .-\\:      /-.    ");
    mvprintw(start_y + 3, start_x, "     | (|:.     |) |    ");
    mvprintw(start_y + 4, start_x, "      '-|:.     |-'     ");
    mvprintw(start_y + 5, start_x, "        \\::.    /      ");
    mvprintw(start_y + 6, start_x, "         '::. .'        ");
    mvprintw(start_y + 7, start_x, "           ) (          ");
    mvprintw(start_y + 8, start_x, "         _.' '._        ");
    mvprintw(start_y + 9, start_x, "        '-------'       ");
}

void win_game(char username[])
{
    clear();
    FILE *scores = fopen("scores.txt", "r");
    FILE *temp = fopen("temp_scores.txt", "w");
    int score = (bag.gold_count) * (difficulty) + 500;
    char current_username[100];
    int current_score;
    bool found = false;
    char line[256];

    start_color();
    init_pair(1, 23, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);

    int center_y = HEIGHT / 2;
    int center_x = WIDTH / 2;

    attron(COLOR_PAIR(1));
    for (int i = center_x - 30; i < center_x + 30; i++)
    {
        mvprintw(center_y - 12, i, "═");
        mvprintw(center_y + 12, i, "═");
    }
    for (int i = center_y - 12; i < center_y + 13; i++)
    {
        mvprintw(i, center_x - 30, "║");
        mvprintw(i, center_x + 29, "║");
    }
    mvprintw(center_y - 12, center_x - 30, "╔");
    mvprintw(center_y - 12, center_x + 29, "╗");
    mvprintw(center_y + 12, center_x - 30, "╚");
    mvprintw(center_y + 12, center_x + 29, "╝");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    display_trophy_ascii(center_y - 10, center_x - 11);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(center_y + 1, center_x - 15, "GLORIOUS VICTORY");
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(center_y + 3, center_x - 20, "Brave Adventurer: %s", username);
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(center_y + 5, center_x - 15, "Final Score: %d", score);
    attroff(COLOR_PAIR(4) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(center_y + 8, center_x - 20, "Press any key to continue your journey...");
    attroff(COLOR_PAIR(3));

    refresh();

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

    getch();

    // Clean up save file
    char save_file[256];
    snprintf(save_file, sizeof(save_file), "%s_save.dat", username);
    remove(save_file);

    pre_game_menu(username);
}

void game_over(char username[])
{
    clear();
    FILE *scores = fopen("scores.txt", "r");
    FILE *temp = fopen("temp_scores.txt", "w");
    int score = (bag.gold_count) * (difficulty);
    char current_username[100];
    int current_score;
    bool found = false;
    char line[256];

    start_color();
    init_pair(1, 23, COLOR_BLACK);
    init_pair(2, 52, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);

    int center_y = HEIGHT / 2;
    int center_x = WIDTH / 2;

    // Draw border
    attron(COLOR_PAIR(1));
    for (int i = center_x - 30; i < center_x + 30; i++)
    {
        mvprintw(center_y - 12, i, "═");
        mvprintw(center_y + 12, i, "═");
    }
    for (int i = center_y - 12; i < center_y + 13; i++)
    {
        mvprintw(i, center_x - 30, "║");
        mvprintw(i, center_x + 29, "║");
    }
    mvprintw(center_y - 12, center_x - 30, "╔");
    mvprintw(center_y - 12, center_x + 29, "╗");
    mvprintw(center_y + 12, center_x - 30, "╚");
    mvprintw(center_y + 12, center_x + 29, "╝");
    attroff(COLOR_PAIR(1));

    // Skeleton Art
    attron(COLOR_PAIR(2));
    mvprintw(center_y - 10, center_x - 20, "     ,-'~~~~    ~~/  ' /_");
    mvprintw(center_y - 9, center_x - 20, "   ,'             ``~~~%%',");
    mvprintw(center_y - 8, center_x - 20, "  (                        Y");
    mvprintw(center_y - 7, center_x - 20, " {                      %% I");
    mvprintw(center_y - 6, center_x - 20, "{      -                    `.");
    mvprintw(center_y - 5, center_x - 20, "|       ',                   )");
    mvprintw(center_y - 4, center_x - 20, "|        |   ,..__      __. Y");
    mvprintw(center_y - 3, center_x - 20, "|    .,_./  Y ' / ^Y   J   )|");
    mvprintw(center_y - 2, center_x - 20, "\\           |' /   |   |   ||");
    mvprintw(center_y - 1, center_x - 20, " \\          L_/    . _ (_,.'(");
    mvprintw(center_y, center_x - 20, "  \\,   ,      ^^\"\"' / |      )");
    mvprintw(center_y + 1, center_x - 20, "    \\_  \\          /,L]     /");
    mvprintw(center_y + 2, center_x - 20, "      '-_`-,       ` `   ./`");
    mvprintw(center_y + 3, center_x - 20, "         `-(_            )");
    mvprintw(center_y + 4, center_x - 20, "             ^^.___,.--`");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(center_y + 6, center_x - 10, "GAME OVER");
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(3));
    mvprintw(center_y + 8, center_x - 20, "Foul Tarnished: %s", username);
    attroff(COLOR_PAIR(3));

    // Final Score
    attron(COLOR_PAIR(4) | A_BOLD);
    mvprintw(center_y + 10, center_x - 15, "Final Score: %d", score);
    attroff(COLOR_PAIR(4) | A_BOLD);

    // Continue prompt
    attron(COLOR_PAIR(3));
    mvprintw(center_y + 11, center_x - 20, "Press any key to continue your journey...");
    attroff(COLOR_PAIR(3));

    refresh();

    // Score file handling
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

    getch();

    char save_file[256];
    snprintf(save_file, sizeof(save_file), "%s_save.dat", username);
    remove(save_file);

    pre_game_menu(username);
}

void check_food_changing()
{
    for (int i = 0; i < bag.total_foods; i++)
    {
        if ((time(NULL) - bag.food_pickup[i]) >= 150)
        {
            switch (bag.food_type[i])
            {
            case 1:
                break;
            case 2:
                bag.food_type[i] = 1;
                bag.food_pickup[i] = time(NULL);
                break;
            case 3:
                bag.food_type[i] = 2;
                bag.food_pickup[i] = time(NULL);
                break;
            case 4:
                bag.food_type[i] = 2;
                bag.food_pickup[i] = time(NULL);
                break;
            }
        }
    }
}

void toggle_full_map()
{
    if (!show_full_map)
    {
        full_map_win = newwin(HEIGHT, WIDTH, 0, 0);
        box(full_map_win, 0, 0);

        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
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
                    char key[5] = "∆";
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
                else if ((map[i][j] == 'd' || map[i][j] == '"'))
                {
                    char dagger[] = "◊";
                    mvprintw(i, j, "%s", dagger);
                }
                else if ((map[i][j] == '*' || map[i][j] == 'W'))
                {
                    attron(COLOR_PAIR(9));
                    char wand[] = "*";
                    attroff(COLOR_PAIR(9));
                    mvprintw(i, j, "%s", wand);
                }
                else if ((map[i][j] == 'A' || map[i][j] == 'a'))
                {
                    char arrow[] = "⪧";
                    mvprintw(i, j, "%s", arrow);
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
                for (int r = 0; r < 6; r++)
                {
                    Room *room = &floors[current_floor].rooms[r];

                    int x = (r < 3) ? room->right_door_x : room->left_door_x;
                    int y = (r < 3) ? room->right_door_y : room->left_door_y;

                    if (floors[current_floor].temp_map[y][x] == '?')
                    {
                        mvaddch(y, x, floors[current_floor].map[y][x]);
                    }

                    if (room->is_room_security)
                    {
                        int door_x = (r < 3) ? room->right_door_x : room->left_door_x;
                        int door_y = (r < 3) ? room->right_door_y : room->left_door_y;

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

                    if (room->is_room_enchanted)
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

                    if (room->is_room_nightmare)
                    {
                        init_pair(53, COLOR_BLUE, COLOR_BLACK);
                        attron(COLOR_PAIR(53));
                        for (int y = room->y; y < room->y + room->height; y++)
                        {
                            for (int x = room->x; x < room->x + room->width; x++)
                            {
                                mvaddch(y, x, map[y][x]);
                            }
                        }
                        attroff(COLOR_PAIR(53));
                    }
                }
                for (int i = 1; i < HEIGHT - 1; i++)
                {
                    for (int j = 0; j < WIDTH; j++)
                    {
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
                            char emoji[5] = "🌮";
                            mvprintw(i, j - 1, "%s", emoji);
                        }
                        else if (map[i][j] == 'K')
                        {
                            char key[5] = "∆";
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
                        else if ((map[i][j] == 'd' || map[i][j] == '"'))
                        {
                            char dagger[] = "◊";
                            mvprintw(i, j, "%s", dagger);
                        }
                        else if ((map[i][j] == '*' || map[i][j] == 'W'))
                        {
                            attron(COLOR_PAIR(9));
                            char wand[] = "*";
                            attroff(COLOR_PAIR(9));
                            mvprintw(i, j, "%s", wand);
                        }
                        else if ((map[i][j] == 'A' || map[i][j] == 'a'))
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
                        else if (map[i][j] == 'm')
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
        }

        mvwprintw(full_map_win, 0, WIDTH / 2 - 5, " FULL MAP ");
        wrefresh(full_map_win);
        show_full_map = true;
        wrefresh(full_map_win);
        getch();
        delwin(full_map_win);
        full_map_win = NULL;
        show_full_map = false;

        draw_map();
        refresh();
    }
}

void *clear_msg()
{
    sleep(2);
    refresh();

    mvprintw(0, 2, "                                                                                                                                                                                       ");
    refresh();

    return NULL;
}

void handle_projectile_pickup(int x, int y)
{
    if (temp_map[y][x] == 'd' || temp_map[y][x] == '"')
    {
        if (temp_map[y][x] == 'd')
        {
            bag.dagger_count += 1;
        }
        else
        {
            bag.dagger_count += 10;
        }
        map[y][x] = '.';
        temp_map[y][x] = '.';
        mvprintw(0, 2, "                                                                ");
        mvprintw(0, 2, "Dagger picked up!");
    }
    else if (temp_map[y][x] == 'a' || temp_map[y][x] == 'A')
    {
        if (temp_map[y][x] == 'a')
        {
            bag.arrow_count += 1;
        }
        else
        {
            bag.arrow_count += 20;
        }
        map[y][x] = '.';
        temp_map[y][x] = '.';
        mvprintw(0, 2, "                                                                ");
        mvprintw(0, 2, "Arrow picked up!");
    }
    else if (temp_map[y][x] == '*' || temp_map[y][x] == 'W')
    {
        if (temp_map[y][x] == '*')
        {
            bag.wand_count += 1;
        }
        else
        {
            bag.wand_count += 8;
        }
        map[y][x] = '.';
        temp_map[y][x] = '.';
        mvprintw(0, 2, "                                                                ");
        mvprintw(0, 2, "Magic Wand picked up!");
    }
    else if (temp_map[y][x] == 's')
    {
        bag.sword = true;
        map[y][x] = '.';
        temp_map[y][x] = '.';
        mvprintw(0, 2, "                                                                ");
        mvprintw(0, 2, "Sword picked up!");
    }
}

void slow_down_enemies(int start_x, int start_y, int dx, int dy)
{
    int projectile_x = start_x + dx;
    int projectile_y = start_y + dy;
    while (projectile_x >= 0 && projectile_x < WIDTH && projectile_y >= 0 && projectile_y < HEIGHT && map[projectile_y][projectile_x] == '.')
    {
        projectile_x += dx;
        projectile_y += dy;
    }
    for (int i = 0; i < enemy_counts[current_floor]; i++)
    {
        Enemy *e = enemies[current_floor][i];
        if (e->x == projectile_x && e->y == projectile_y)
        {
            e->can_move = false;
            mvprintw(0, 2, "                                                                ");
            mvprintw(0, 2, "Enemy slowed down!");
            break;
        }
    }
}

int fire_projectile(int start_x, int start_y, int dx, int dy, int damage, char symbol, int max_distance)
{
    int projectile_x = start_x + dx;
    int projectile_y = start_y + dy;
    int distance = 0;
    bool hit_enemy = false;

    while (projectile_x >= 0 && projectile_x < WIDTH &&
           projectile_y >= 0 && projectile_y < HEIGHT &&
           distance < max_distance && map[projectile_y + dy][projectile_x + dx] != '|' &&
           map[projectile_y + dy][projectile_x + dx] != '-' && map[projectile_y + dy][projectile_x + dx] != 'O')
    {
        Enemy **current_enemies = final_room ? enemies[4] : enemies[current_floor];
        int current_enemy_count = final_room ? enemy_counts[4] : enemy_counts[current_floor];

        for (int i = 0; i < current_enemy_count; i++)
        {
            Enemy *e = current_enemies[i];
            if (e && e->x == projectile_x && e->y == projectile_y && e->active)
            {
                e->health -= damage;
                if (e->health <= 0)
                {
                    e->active = false;
                    map[e->y][e->x] = '.';
                    temp_map[e->y][e->x] = '.';
                    mvprintw(0, 60, "Enemy defeated!");
                }
                hit_enemy = true;
                break;
            }
        }

        if (hit_enemy)
            break;

        projectile_x += dx;
        projectile_y += dy;
        distance++;
    }

    if (!hit_enemy &&
        projectile_x >= 0 && projectile_x < WIDTH &&
        projectile_y >= 0 && projectile_y < HEIGHT &&
        map[projectile_y][projectile_x] == '.')
    {
        map[projectile_y][projectile_x] = tolower(symbol);
        temp_map[projectile_y][projectile_x] = tolower(symbol);
        return PROJECTILE_MISSED;
    }

    return hit_enemy ? PROJECTILE_HIT_ENEMY : PROJECTILE_MISSED;
}

void handle_sword_attack()
{
    bool hit = false;
    last_dy = 0;
    last_dx = 0;

    Enemy **current_enemies = final_room ? enemies[4] : enemies[current_floor];
    int current_enemy_count = final_room ? enemy_counts[4] : enemy_counts[current_floor];

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int new_x = hero_x + dx;
            int new_y = hero_y + dy;

            for (int i = 0; i < current_enemy_count; i++)
            {
                Enemy *e = current_enemies[i];
                if (e && e->active && e->x == new_x && e->y == new_y)
                {
                    int multiplier = get_damage_multiplier();
                    hit = true;
                    e->health -= 10 * multiplier;
                    if (e->health <= 0)
                    {
                        e->active = false;
                        map[e->y][e->x] = '.';
                        temp_map[e->y][e->x] = '.';
                        mvprintw(0, 2, "                                                        ");
                        mvprintw(0, 60, "Enemy defeated!");
                    }
                    break;
                }
            }
        }
    }
    mvprintw(0, 2, "                                                        ");
    mvprintw(0, 20, "Sword attack%s!", hit ? "" : " missed");
}

void handle_mace_attack()
{
    bool hit = false;
    last_dy = 0;
    last_dx = 0;

    Enemy **current_enemies = final_room ? enemies[4] : enemies[current_floor];
    int current_enemy_count = final_room ? enemy_counts[4] : enemy_counts[current_floor];

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int new_x = hero_x + dx;
            int new_y = hero_y + dy;

            for (int i = 0; i < current_enemy_count; i++)
            {
                Enemy *e = current_enemies[i];
                if (e && e->active && e->health > 0 && e->can_move)
                {
                    if (e->x == new_x && e->y == new_y)
                    {
                        hit = true;
                        int multiplier = get_damage_multiplier();
                        e->health -= 5 * multiplier;
                        if (e->health <= 0)
                        {
                            e->active = false;
                            map[e->y][e->x] = '.';
                            temp_map[e->y][e->x] = '.';
                            mvprintw(0, 60, "Enemy defeated!");
                        }
                        break;
                    }
                }
            }
        }
    }
    if (!hit)
    {
        mvprintw(0, 2, "                        ");
        mvprintw(0, 20, "Attack missed!");
    }
    else
    {
        mvprintw(0, 2, "                        ");
        mvprintw(0, 2, "Mace attack! 12 Damage.");
    }
}

void handle_dagger_attack()
{
    mvprintw(0, 2, "                        ");
    mvprintw(0, 2, "insert direction : (hjklyubn)");
    int dy = 0, dx = 0;
    char ch = getch();

    switch (ch)
    {
    case 'h':
        dx = -1;
        dy = 0;
        break;
    case 'j':
        dx = 0;
        dy = -1;
        break;
    case 'k':
        dx = 0;
        dy = 1;
        break;
    case 'l':
        dx = 1;
        dy = 0;
        break;
    case 'y':
        dx = -1;
        dy = -1;
        break;
    case 'u':
        dx = 1;
        dy = -1;
        break;
    case 'b':
        dx = -1;
        dy = 1;
        break;
    case 'n':
        dx = 1;
        dy = 1;
        break;
    default:
        return;
    }

    last_dx = dx;
    last_dy = dy;

    if (bag.dagger_count > 0)
    {
        bag.dagger_count--;
        int multiplier = get_damage_multiplier();
        int result = fire_projectile(hero_x, hero_y, dx, dy, 12 * multiplier, 'd', 5);

        if (result == PROJECTILE_MISSED)
        {
            mvprintw(0, 2, "Dagger missed!");
        }
        else
        {
            mvprintw(0, 2, "Dagger hit!");
        }
        mvprintw(0, 20, "Dagger thrown! Remaining daggers: %d", bag.dagger_count);
    }
    else
    {
        mvprintw(0, 2, "No daggers left!");
    }
}

void handle_magic_wand_attack()
{

    mvprintw(0, 2, "                        ");
    mvprintw(0, 2, "insert direction : (hjklyubn)");
    int dy = 0, dx = 0;
    char ch = getch();

    switch (ch)
    {
    case 'h':
        dx = -1;
        dy = 0;
        break;
    case 'j':
        dx = 0;
        dy = -1;
        break;
    case 'k':
        dx = 0;
        dy = 1;
        break;
    case 'l':
        dx = 1;
        dy = 0;
        break;
    case 'y':
        dx = -1;
        dy = -1;
        break;
    case 'u':
        dx = 1;
        dy = -1;
        break;
    case 'b':
        dx = -1;
        dy = 1;
        break;
    case 'n':
        dx = 1;
        dy = 1;
        break;
    default:
        return;
    }
    last_dx = dx;
    last_dy = dy;
    if (bag.wand_count > 0)
    {
        bag.wand_count--;
        int multiplier = get_damage_multiplier();
        fire_projectile(hero_x, hero_y, dx, dy, 15 * multiplier, '*', 10);
        slow_down_enemies(hero_x, hero_y, dx, dy);
        mvprintw(0, 2, "Magic Wand used! Remaining wands: %d", bag.wand_count);
    }
    else
    {
        mvprintw(0, 2, "No magic wands left!");
    }
}

void handle_arrow_attack()
{
    mvprintw(0, 2, "                        ");
    mvprintw(0, 2, "insert direction : (hjklyubn)");
    int dy = 0, dx = 0;
    char ch = getch();

    switch (ch)
    {
    case 'h':
        dx = -1;
        dy = 0;
        break;
    case 'j':
        dx = 0;
        dy = -1;
        break;
    case 'k':
        dx = 0;
        dy = 1;
        break;
    case 'l':
        dx = 1;
        dy = 0;
        break;
    case 'y':
        dx = -1;
        dy = -1;
        break;
    case 'u':
        dx = 1;
        dy = -1;
        break;
    case 'b':
        dx = -1;
        dy = 1;
        break;
    case 'n':
        dx = 1;
        dy = 1;
        break;
    default:
        return;
    }
    last_dx = dx;
    last_dy = dy;
    if (bag.arrow_count > 0)
    {
        bag.arrow_count--;
        int multiplier = get_damage_multiplier();
        int result = fire_projectile(hero_x, hero_y, dx, dy, 5 * multiplier, 'a', 5);

        if (result == PROJECTILE_MISSED)
        {
            mvprintw(0, 2, "                                                        ");
            mvprintw(0, 2, "Arrow missed!");
        }
        else
        {
            mvprintw(0, 2, "                                                        ");
            mvprintw(0, 2, "Arrow hit!");
        }
        // mvprintw(0, 2, "                                                        ");
        mvprintw(0, 20, "Arrow fired! Remaining arrows: %d", bag.arrow_count);
    }
    else
    {
        mvprintw(0, 2, "                                                        ");
        mvprintw(0, 2, "No arrows left!");
    }
}

void replay_last_attack()
{
    if (last_dx == 0 && last_dy == 0)
    {
        mvprintw(0, 2, "                                                        ");
        mvprintw(0, 2, "Your last attack was not a ranged attack!");
        return;
    }
    else
    {
        if (weapon_selected == 1)
        {
            if (bag.dagger_count > 0)
            {
                bag.dagger_count--;
                int multiplier = get_damage_multiplier();
                int result = fire_projectile(hero_x, hero_y, last_dx, last_dy, 12 * multiplier, 'd', 5);

                if (result == PROJECTILE_MISSED)
                {
                    mvprintw(0, 2, "Dagger missed!");
                }
                else
                {
                    mvprintw(0, 2, "Dagger hit!");
                }
                mvprintw(0, 20, "Dagger thrown! Remaining daggers: %d", bag.dagger_count);
            }
            else
            {
                mvprintw(0, 2, "No daggers left!");
            }
        }
        else if (weapon_selected == 2)
        {
            if (bag.wand_count > 0)
            {
                bag.wand_count--;
                int multiplier = get_damage_multiplier();
                fire_projectile(hero_x, hero_y, last_dx, last_dy, 15 * multiplier, '*', 10);
                slow_down_enemies(hero_x, hero_y, last_dx, last_dy);
                mvprintw(0, 2, "Magic Wand used! Remaining wands: %d", bag.wand_count);
            }
            else
            {
                mvprintw(0, 2, "No magic wands left!");
            }
        }
        else if (weapon_selected == 3)
        {
            if (bag.arrow_count > 0)
            {
                bag.arrow_count--;
                int multiplier = get_damage_multiplier();
                int result = fire_projectile(hero_x, hero_y, last_dx, last_dy, 5 * multiplier, 'a', 5);

                if (result == PROJECTILE_MISSED)
                {
                    mvprintw(0, 2, "                                                        ");
                    mvprintw(0, 2, "Arrow missed!");
                }
                else
                {
                    mvprintw(0, 2, "                                                        ");
                    mvprintw(0, 2, "Arrow hit!");
                }
                mvprintw(0, 20, "Arrow fired! Remaining arrows: %d", bag.arrow_count);
            }
            else
            {
                mvprintw(0, 2, "                                                        ");
                mvprintw(0, 2, "No arrows left!");
            }
        }
    }
}

void attack_with_weapon()
{
    switch (weapon_selected)
    {
    case 0:
        handle_mace_attack();
        break;
    case 1:
        handle_dagger_attack();
        break;
    case 2:
        handle_magic_wand_attack();
        break;
    case 3:
        handle_arrow_attack();
        break;
    case 4:
        handle_sword_attack();
        break;
    default:
        mvprintw(0, 2, "No weapon selected!");
        break;
    }
}

void eat_food()
{
    if (bag.total_foods > 0)
    {
        int food_type = bag.food_type[bag.total_foods - 1];
        bag.total_foods--;

        switch (food_type)
        {
        case 1:
            mvprintw(0, 2, "Spoiled food eaten! Hunger +20");
            hunger += 20;
            break;
        case 2:
            mvprintw(0, 2, "Normal food eaten! Hunger->0");
            hunger = 0;
            break;
        case 3:
            mvprintw(0, 2, "Magical food eaten! Hunger->0, damage boosted");
            hunger = 0;
            damage_spell_end_time += 10;
            break;
        case 4:
            mvprintw(0, 2, "Special food eaten! Hunger->0, speed boosted");
            hunger = 0;
            speed_spell_end_time += 10;
            break;
        default:
            mvprintw(0, 2, "Unknown food type! No effect.");
            break;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, clear_msg, NULL);
        pthread_detach(tid);
    }
    else
    {
        mvprintw(0, 2, "No food in bag!");
        pthread_t tid;
        pthread_create(&tid, NULL, clear_msg, NULL);
        pthread_detach(tid);
    }
}

void open_weapon_window()
{
    const char *weapons[] = {"Mace", "Dagger", "Magical Wand", "Normal Arrow", "Sword"};
    int num_weapons = 5;
    int selected = 0;

    WINDOW *weapon_win = newwin(8, 30, LINES / 2 - 4, COLS / 2 - 15);
    box(weapon_win, 0, 0);
    keypad(weapon_win, TRUE);

    start_color();
    init_pair(10, COLOR_GREEN, COLOR_BLACK);
    init_pair(11, COLOR_RED, COLOR_BLACK);

    while (1)
    {
        wclear(weapon_win);
        box(weapon_win, 0, 0);
        mvwprintw(weapon_win, 1, 2, "Select Weapon:");

        for (int i = 0; i < num_weapons; i++)
        {
            if (bag.dagger_count == 0)
            {
                bag.dagger = false;
            }
            else
            {
                bag.dagger = true;
            }

            if (bag.arrow_count == 0)
            {
                bag.normal_arrow = false;
            }
            else
            {
                bag.normal_arrow = true;
            }

            if (bag.wand_count == 0)
            {
                bag.magic_wand = false;
            }
            else
            {
                bag.wand_count = true;
            }

            bool has_weapon = (i == 0) ? bag.mace : (i == 1) ? bag.dagger
                                                : (i == 2)   ? bag.magic_wand
                                                : (i == 3)   ? bag.normal_arrow
                                                             : bag.sword;

            if (i == selected)
                wattron(weapon_win, A_REVERSE);

            wattron(weapon_win, COLOR_PAIR(has_weapon ? 10 : 11));

            if (i == 1)
                mvwprintw(weapon_win, i + 2, 2, "%s (%d)", weapons[i], bag.dagger_count);
            else if (i == 2)
                mvwprintw(weapon_win, i + 2, 2, "%s (%d)", weapons[i], bag.wand_count);
            else if (i == 3)
                mvwprintw(weapon_win, i + 2, 2, "%s (%d)", weapons[i], bag.arrow_count);
            else
                mvwprintw(weapon_win, i + 2, 2, "%s", weapons[i]);

            wattroff(weapon_win, COLOR_PAIR(has_weapon ? 10 : 11));

            if (i == selected)
                wattroff(weapon_win, A_REVERSE);
        }

        wrefresh(weapon_win);

        int ch = wgetch(weapon_win);
        if (ch == KEY_UP)
            selected = (selected - 1 + num_weapons) % num_weapons;
        else if (ch == KEY_DOWN)
            selected = (selected + 1) % num_weapons;
        else if (ch == 10)
        {
            bool has_selected_weapon = (selected == 0) ? bag.mace : (selected == 1) ? bag.dagger
                                                                : (selected == 2)   ? bag.magic_wand
                                                                : (selected == 3)   ? bag.normal_arrow
                                                                                    : bag.sword;

            if (has_selected_weapon)
                weapon_selected = selected;
            break;
        }
        else if (ch == 27 || ch == 'i')
            break;
    }
    delwin(weapon_win);
    refresh();
}

void open_spell_window()
{
    WINDOW *spell_win = newwin(7, 30, LINES / 2 - 4, COLS / 2 - 15);
    box(spell_win, 0, 0);
    keypad(spell_win, TRUE);

    const char *spells[] = {"Speed Spell", "Damage Spell", "Health Spell"};
    int selected = 0;
    int num_spells = 3;

    while (1)
    {
        wclear(spell_win);
        box(spell_win, 0, 0);
        mvwprintw(spell_win, 1, 2, "Select Spell:");

        for (int i = 0; i < num_spells; i++)
        {
            if (i == selected)
                wattron(spell_win, A_REVERSE);
            mvwprintw(spell_win, i + 2, 2, "%s (%d)", spells[i],
                      (i == 0) ? bag.speed_spells : (i == 1) ? bag.damage_spells
                                                             : bag.health_spells);
            if (i == selected)
                wattroff(spell_win, A_REVERSE);
        }

        wrefresh(spell_win);

        int ch = wgetch(spell_win);
        if (ch == KEY_UP)
            selected = (selected - 1 + num_spells) % num_spells;
        else if (ch == KEY_DOWN)
            selected = (selected + 1) % num_spells;
        else if (ch == 10)
        {
            if (selected == 0 && bag.speed_spells > 0)
            {
                bag.speed_spells--;
                speed_spell_end_time = time(NULL) + 20;
                mvprintw(0, 2, "Speed spell activated! Movement speed doubled for 20 seconds.");
                refresh();
            }
            else if (selected == 1 && bag.damage_spells > 0)
            {
                bag.damage_spells--;
                damage_spell_end_time = time(NULL) + 10;
                mvprintw(0, 2, "Damage spell activated! Movement speed doubled for 15 seconds.");
            }
            else if (selected == 2 && bag.health_spells > 0)
            {
                bag.health_spells--;
                health_spell_end_time = time(NULL) + 10;
                mvprintw(0, 2, "Health spell activated! Movement speed doubled for 10 seconds.");
            }
            break;
        }
        else if (ch == 27 || ch == 'w')
            break;
    }
    delwin(spell_win);

    pthread_t tid1;
    pthread_create(&tid1, NULL, clear_msg, NULL);
    pthread_detach(tid1);

    refresh();
}

void create_final_room()
{
    clear();
    current_floor = 4;
    final_room = true;

    for (int i = 0; i < MAX_ENEMIES_PER_FLOOR; i++)
    {
        if (enemies[4][i] != NULL)
        {
            free(enemies[4][i]);
            enemies[4][i] = NULL;
        }
    }
    enemy_counts[4] = 0;

    if (map == NULL)
    {
        initialize_map();
    }

    int room_width = 40;
    int room_height = 20;
    int start_x = (WIDTH - room_width) / 2;
    int start_y = (HEIGHT - room_height) / 2;
    int end_x = start_x + room_width;
    int end_y = start_y + room_height;

    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            map[y][x] = ' ';
            temp_map[y][x] = ' ';
        }
    }

    for (int y = start_y; y <= end_y; y++)
    {
        for (int x = start_x; x <= end_x; x++)
        {
            if (y == start_y || y == end_y)
            {
                map[y][x] = '-';
            }
            else if (x == start_x || x == end_x)
            {
                map[y][x] = '|';
            }
            else
            {
                map[y][x] = '.';
            }
            discovered_map[y][x] = 1;
            temp_map[y][x] = map[y][x];
        }
    }

    int center_x = start_x + (room_width / 2);
    int center_y = start_y + (room_height / 2);

    for (int i = 0; i < 4; i++)
    {
        Enemy *e = calloc(1, sizeof(Enemy));
        if (e == NULL)
            continue;

        e->active = true;
        e->health = 40;
        e->can_move = true;

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

        double angle = (2 * M_PI * i) / 8;
        int radius = 4;
        e->x = center_x + (int)(radius * cos(angle));
        e->y = center_y + (int)(radius * sin(angle));

        e->x = (e->x < start_x + 1) ? start_x + 1 : (e->x >= end_x - 1 ? end_x - 1 : e->x);
        e->y = (e->y < start_y + 1) ? start_y + 1 : (e->y >= end_y - 1 ? end_y - 1 : e->y);
        e->active = true;

        map[e->y][e->x] = e->symbol;
        enemies[4][enemy_counts[4]++] = e;
    }

    hero_x = start_x + 1;
    hero_y = start_y + 1;

    map[hero_y][hero_x] = '@';

    draw_map();
    refresh();
}

void *clear_security_mode()
{
    sleep(20);
    refresh();
    mvprintw(LINES - 1, COLS / 2 - 7, "                                  ");
    security_mode = false;
    refresh();

    return NULL;
}

void open_pass_window(int current_floor, int current_room)
{
    start_color();
    int win_height = 8;
    int win_width = 40;
    int start_y = (LINES - win_height) / 2;
    int start_x = (COLS - win_width) / 2;
    init_pair(71, COLOR_YELLOW, COLOR_BLACK);
    init_pair(72, COLOR_RED, COLOR_BLACK);
    init_pair(73, COLOR_GREEN, COLOR_BLACK);

    WINDOW *pass_win = newwin(win_height, win_width, start_y, start_x);
    box(pass_win, 0, 0);

    keypad(pass_win, TRUE);

    char password[5] = {0};
    int pos = 0;

    if (bag.has_ancient_key)
    {
        mvwprintw(pass_win, win_height - 2, 2, "Press 'K' to use Ancient Key");
    }

    mvwprintw(pass_win, 1, 2, "Enter Room Password:");
    wrefresh(pass_win);

    while (1)
    {
        mvwprintw(pass_win, 2, 2, "Password: ");
        for (int i = 0; i < 4; i++)
        {
            if (i < pos)
            {
                waddch(pass_win, password[i]);
            }
            else
            {
                waddch(pass_win, '_');
            }
            waddch(pass_win, ' ');
        }

        mvwprintw(pass_win, 3, 2, "                              ");

        wmove(pass_win, 2, 11 + (pos * 2));

        wrefresh(pass_win);

        int ch = wgetch(pass_win);

        if (ch == 'k' || ch == 'K')
        {
            if (bag.has_ancient_key)
            {
                if (rand() % 10 == 0)
                {
                    wattron(pass_win, COLOR_PAIR(72));
                    mvwprintw(pass_win, 3, 2, "Ancient Key broke!");
                    wattron(pass_win, COLOR_PAIR(72));
                    floors[current_floor].rooms[current_room].sec_level = 0;
                    floors[current_floor].rooms[current_room].is_room_locked = false;
                    bag.has_ancient_key -= 1;
                    bag.broke_key += 1;
                    if (bag.broke_key >= 2)
                    {
                        bag.broke_key -= 2;
                        bag.has_ancient_key += 1;
                        mvprintw(0, COLS / 2 + 5, "Ancient key made!");
                    }
                    wrefresh(pass_win);
                    napms(1000);
                    break;
                }
                else
                {
                    wattron(pass_win, COLOR_PAIR(73));
                    mvwprintw(pass_win, 3, 2, "Ancient Key worked!");
                    wattroff(pass_win, COLOR_PAIR(73));
                    floors[current_floor].rooms[current_room].sec_level = 0;
                    floors[current_floor].rooms[current_room].is_room_locked = false;
                    wrefresh(pass_win);
                    napms(1000);
                    break;
                }
            }
        }

        if (ch >= '0' && ch <= '9' && pos < 4)
        {
            password[pos++] = ch;
        }
        else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b' || ch == 8)
        {
            if (pos > 0)
            {
                pos--;
                password[pos] = 0;
                mvwprintw(pass_win, 2, 11 + (pos * 2), "_");
            }
        }
        else if (ch == 10 && pos == 4)
        {
            int entered_pass = atoi(password);

            if (entered_pass == floors[current_floor].rooms[current_room].password)
            {
                wattron(pass_win, COLOR_PAIR(73));
                mvwprintw(pass_win, 3, 2, "Password Correct!");
                wattroff(pass_win, COLOR_PAIR(73));
                floors[current_floor].rooms[current_room].sec_level = 0;
                floors[current_floor].rooms[current_room].is_room_locked = false;
                wrefresh(pass_win);
                napms(1000);
                break;
            }
            else
            {
                floors[current_floor].rooms[current_room].sec_level++;

                if (floors[current_floor].rooms[current_room].sec_level >= 3)
                {
                    mvwprintw(pass_win, 3, 2, "Security Mode!");
                    security_mode = true;
                    pthread_t tid;
                    pthread_create(&tid, NULL, clear_security_mode, NULL);
                    pthread_detach(tid);

                    napms(1000);
                    break;
                }
                int num = 70 + floors[current_floor].rooms[current_room].sec_level;
                wattron(pass_win, COLOR_PAIR(num));
                mvwprintw(pass_win, 3, 2, "Wrong Password!");
                wattroff(pass_win, COLOR_PAIR(num));
                wrefresh(pass_win);
                napms(1000);
                pos = 0;
                memset(password, 0, sizeof(password));
            }

            if (!floors[current_floor].rooms[current_room].is_room_locked)
            {
                Room *room = &floors[current_floor].rooms[current_room];

                int door_x = (current_room >= 3) ? room->left_door_x : room->right_door_x;
                int door_y = (current_room >= 3) ? room->left_door_y : room->right_door_y;

                floors[current_floor].temp_map[door_y][door_x] = '+';
                temp_map[door_y][door_x] = '+';
            }
        }
        else if (ch == 27)
        {
            break;
        }
    }

    delwin(pass_win);
    refresh();
}

void choose_difficulty()
{
    clear();
    boarder();
    refresh();

    const char *difficulty_options[] = {"Easy", "Medium", "Hard"};
    int option = 0;
    int num_options = 3;

    mvprintw(2, COLS / 2 - 12, "Choose Game Difficulty");
    mvprintw(4, COLS / 2 - 21, "Easy: Less damage, more time between hits");
    mvprintw(5, COLS / 2 - 16, "Medium: Moderate damage and food");
    mvprintw(6, COLS / 2 - 14, "Hard: More damage, less food");

    keypad(stdscr, TRUE);
    while (1)
    {
        for (int i = 0; i < num_options; i++)
        {
            if (i == option)
                attron(A_REVERSE);
            mvprintw(LINES / 2 + i * 2, COLS / 2 - strlen(difficulty_options[i]) / 2,
                     "%s", difficulty_options[i]);
            if (i == option)
                attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP)
        {
            option = (option == 0) ? num_options - 1 : option - 1;
        }
        else if (ch == KEY_DOWN)
        {
            option = (option == num_options - 1) ? 0 : option + 1;
        }
        else if (ch == 10)
        {
            difficulty = option + 1;
            clear();
            return;
        }
    }
}

void init_health_system()
{
    health = 100;
    last_hit_time = time(NULL);
    hunger = 0;
}

int get_damage_interval()
{
    switch (difficulty)
    {
    case 1:
        return 60;
    case 2:
        return 30;
    case 3:
        return 20;
    }
    return 1;
}

void hit_damage()
{
    time_t current_time = time(NULL);
    int damage_interval = get_damage_interval();

    if (difftime(current_time, last_hit_time) >= damage_interval)
    {
        hunger += 1;
        last_hit_time = current_time;
        hunger = hunger >= 100 ? 100 : hunger;
        draw_map();
        refresh();
    }
}

void init_health(char username[])
{
    if (hunger >= 100)
    {
        health -= 5;
    }
    else
    {
        if (rand() % (difficulty + 1))
        {
            int multiplier = get_health_restore_multiplier();
            health += multiplier * (double)((100 - hunger) / 100);
        }
        health = health >= 100 ? 100 : health;
    }

    if (health <= 0)
    {
        clear();
        game_over(username);
    }
}

void add_hero(int hero_y, int hero_x)
{
    if (current_hero_color == 1)
    {
        init_pair(6, COLOR_WHITE, COLOR_BLACK);
    }
    else if (current_hero_color == 2)
    {
        init_pair(6, COLOR_RED, COLOR_BLACK);
    }
    else if (current_hero_color == 3)
    {
        init_pair(6, COLOR_GREEN, COLOR_BLACK);
    }
    else if (current_hero_color == 4)
    {
        init_pair(6, COLOR_BLUE, COLOR_BLACK);
    }
    else if (current_hero_color == 5)
    {
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    }
    else if (current_hero_color == 6)
    {
        init_pair(6, COLOR_CYAN, COLOR_BLACK);
    }
    attron(COLOR_PAIR(6));
    mvaddch(hero_y, hero_x, '@');
    attroff(COLOR_PAIR(6));
}

void boarder_prim()
{
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(LINES - 1, i, " ");
        mvprintw(0, i, " ");
    }
}

int room_pass_generator()
{
    srand(time(NULL));

    return rand() % 9000 + 1000;
}

void *clear_password_after_delay(void *arg)
{
    sleep(30);
    refresh();
    for (int i = 0; i < WIDTH; i++)
        mvprintw(0, COLS / 2 + 20, "                       ");
    refresh();

    return NULL;
}

int reverse_number(int num)
{
    int reversed = 0;

    while (num != 0)
    {
        reversed = reversed * 10 + (num % 10);
        num /= 10;
    }

    return reversed;
}

void generate_pass(int current_floor, int current_room)
{
    mvprintw(LINES - 1, COLS / 2 - 10, "                       ");
    refresh();
    int password = room_pass_generator();
    int shown_password;
    if (floors[current_floor].rooms[current_room].is_room_old)
    {
        shown_password = reverse_number(password);
    }
    else
    {
        shown_password = password;
    }
    floors[current_floor].rooms[current_room].password = password;

    mvprintw(0, COLS / 2 + 20, "Room Password: %d", shown_password);
    refresh();

    pthread_t tid;
    pthread_create(&tid, NULL, clear_password_after_delay, NULL);
    pthread_detach(tid);
}

// Function for special movement system
void move_special(char c)
{
    if (c == 'f')
    {
        char ch = getch();

        int dx = 0, dy = 0;

        switch (ch)
        {
        case 'h':
            dx = -1;
            break;
        case 'l':
            dx = 1;
            break;
        case 'j':
            dy = -1;
            break;
        case 'k':
            dy = 1;
            break;
        case 'y':
            dx = -1;
            dy = 1;
            break;
        case 'u':
            dx = 1;
            dy = 1;
            break;
        case 'b':
            dx = -1;
            dy = -1;
            break;
        case 'n':
            dx = 1;
            dy = -1;
            break;
        default:
            return;
        }

        while (hero_x + dx >= 0 && hero_x + dx < WIDTH &&
               hero_y + dy >= 0 && hero_y + dy < HEIGHT &&
               map[hero_y + dy][hero_x + dx] != 'O' &&
               map[hero_y + dy][hero_x + dx] != '|' &&
               map[hero_y + dy][hero_x + dx] != '-' &&
               map[hero_y + dy][hero_x + dx] != ' ')
        {
            map[hero_y][hero_x] = temp_map[hero_y][hero_x];
            hero_x += dx;
            hero_y += dy;
            temp_map[hero_y][hero_x] = map[hero_y][hero_x];
            map[hero_y][hero_x] = '@';
            draw_map();
            refresh();

            napms(100);
        }
    }
    if (c == 'g')
    {
        char ch = getch();

        int dx = 0, dy = 0;

        switch (ch)
        {
        case 'h':
            dx = -1;
            break;
        case 'l':
            dx = 1;
            break;
        case 'j':
            dy = -1;
            break;
        case 'k':
            dy = 1;
            break;
        case 'y':
            dx = -1;
            dy = 1;
            break;
        case 'u':
            dx = 1;
            dy = 1;
            break;
        case 'b':
            dx = -1;
            dy = -1;
            break;
        case 'n':
            dx = 1;
            dy = -1;
            break;
        default:
            return;
        }

        while (hero_x + dx >= 0 && hero_x + dx < WIDTH &&
               hero_y + dy >= 0 && hero_y + dy < HEIGHT &&
               map[hero_y + dy][hero_x + dx] != 'O' &&
               map[hero_y + dy][hero_x + dx] != '|' &&
               map[hero_y + dy][hero_x + dx] != '-' &&
               map[hero_y + dy][hero_x + dx] != ' ' &&
               map[hero_y + dy][hero_x + dx] != '!' &&
               map[hero_y + dy][hero_x + dx] != '~' &&
               map[hero_y + dy][hero_x + dx] != 'a' &&
               map[hero_y + dy][hero_x + dx] != 'A' &&
               map[hero_y + dy][hero_x + dx] != '*' &&
               map[hero_y + dy][hero_x + dx] != 'W' &&
               map[hero_y + dy][hero_x + dx] != '"' &&
               map[hero_y + dy][hero_x + dx] != 'd' &&
               map[hero_y + dy][hero_x + dx] != 'H' &&
               map[hero_y + dy][hero_x + dx] != '%')
        {
            map[hero_y][hero_x] = temp_map[hero_y][hero_x];
            hero_x += dx;
            hero_y += dy;
            temp_map[hero_y][hero_x] = map[hero_y][hero_x];
            map[hero_y][hero_x] = '@';

            draw_map();
            refresh();

            napms(100);
        }
    }
}

void handle_room_exit(int old_room_index)
{
    if (old_room_index != -1 && floors[current_floor].rooms[old_room_index].is_room_nightmare)
    {
        Room room = floors[current_floor].rooms[old_room_index];
        for (int y = room.y; y < room.y + room.height; y++)
        {
            for (int x = room.x; x < room.x + room.width; x++)
            {
                discovered_map[y][x] = 0;
            }
        }
    }
}

void continue_game(char username[])
{
    final_room = false;
    int old_room = -1;
    refresh();
    boarder_prim();
    cleanup_game_memory();

    int score = 0;
    init_health_system();
    init_bag();
    if (load_game(username, &score))
    {
        draw_map();
        if (current_hero_color == 1)
        {
            init_pair(6, COLOR_WHITE, COLOR_BLACK);
        }
        else if (current_hero_color == 2)
        {
            init_pair(6, COLOR_RED, COLOR_BLACK);
        }
        else if (current_hero_color == 3)
        {
            init_pair(6, COLOR_GREEN, COLOR_BLACK);
        }
        else if (current_hero_color == 4)
        {
            init_pair(6, COLOR_BLUE, COLOR_BLACK);
        }
        else if (current_hero_color == 5)
        {
            init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        }
        else if (current_hero_color == 6)
        {
            init_pair(6, COLOR_CYAN, COLOR_BLACK);
        }
        attron(COLOR_PAIR(6));
        mvaddch(hero_y, hero_x, '@');
        attroff(COLOR_PAIR(6));
    }
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
            temp_map[i][j] = floors[current_floor].temp_map[i][j];
        }
    }

    while (1)
    {
        update_enemies();
        hit_damage();
        init_health(username);
        check_food_changing();

        if (check_final_room_victory())
        {
            win_game(username);
        }
        draw_map();
        char ch = getch();
        if (ch == 27)
        {
            pause_menu(username, hero_x, hero_y, score);
            break;
        }

        int new_x = hero_x;
        int new_y = hero_y;

        if (ch == 'j')
        {
            new_y -= get_movement_distance();
        }
        else if (ch == 'k')
        {
            new_y += get_movement_distance();
        }
        else if (ch == 'h')
        {
            new_x -= get_movement_distance();
        }
        else if (ch == 'l')
        {
            new_x += get_movement_distance();
        }
        else if (ch == 'y')
        {
            new_y -= get_movement_distance();
            new_x -= get_movement_distance();
        }
        else if (ch == 'u')
        {
            new_y -= get_movement_distance();
            new_x += get_movement_distance();
        }
        else if (ch == 'b')
        {
            new_y += get_movement_distance();
            new_x -= get_movement_distance();
        }
        else if (ch == 'n')
        {
            new_x += get_movement_distance();
            new_y += get_movement_distance();
        }
        else if (ch == 'e')
        {
            eat_food();
        }
        else if (ch == 'i')
        {
            open_weapon_window();
        }
        else if (ch == 'w')
        {
            open_spell_window();
        }
        else if (tolower(ch) == 27)
        {
            save_game(username, score);
            pause_menu(username, hero_x, hero_y, score);
        }
        else if (ch == 'm')
        {
            toggle_full_map();
        }
        else if (ch == 'a')
        {
            replay_last_attack();
        }
        else if (ch == ' ')
        {
            attack_with_weapon();
        }

        else if (ch == 'g' || ch == 'f')
        {
            move_special(ch);
            new_x = hero_x;
            new_y = hero_y;
        }

        else if (ch == '<' || ch == ',')
        {
            if (temp_map[hero_y][hero_x] == '<')
            {
                current_floor++;
                switch_floor();
                hero_y = floors[current_floor].rooms[0].down_stair_y;
                hero_x = floors[current_floor].rooms[0].down_stair_x;
                map[hero_y][hero_x] = '@';
                init_health_system();
                draw_map();
                mvaddch(hero_y, hero_x, '@');
                new_x = hero_x;
                new_y = hero_y;
            }
        }
        else if (ch == '>' || ch == '.')
        {
            if (temp_map[hero_y][hero_x] == '>')
            {
                current_floor--;
                hero_y = floors[current_floor].rooms[5].up_stair_y;
                hero_x = floors[current_floor].rooms[5].up_stair_x;
                switch_floor();
                map[hero_y][hero_x] = '@';
                init_health_system();
                draw_map();
                mvaddch(hero_y, hero_x, '@');
                new_x = hero_x;
                new_y = hero_y;
            }
        }

        if (ch == 10)
        {

            if (temp_map[hero_y][hero_x] == 'd' || map[new_y][new_x] == '"' ||
                temp_map[hero_y][hero_x] == 'a' || temp_map[hero_y][hero_x] == 'A' ||
                temp_map[hero_y][hero_x] == '*' || temp_map[hero_y][hero_x] == 'W')
            {
                handle_projectile_pickup(hero_x, hero_y);
            }

            if (temp_map[hero_y][hero_x] == '~' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Speed Spell +1");
                bag.speed_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == '!' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Damage Spell +1");
                bag.damage_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == 'H' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Health Spell +1");
                bag.health_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if ((temp_map[hero_y][hero_x] == 'H' || map[hero_y][hero_x] == '!' ||
                      map[hero_y][hero_x] == '~') &&
                     bag.total_spells == 5)
            {
                mvprintw(0, 2, "Bag of spells is full");
                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == '%' && bag.total_foods != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Food picked up!");
                bag.food_pickup[bag.total_foods] = time(NULL);
                bag.food_type[bag.total_foods] = rand() % 3 + 1;
                bag.total_foods++;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (bag.total_foods == 5 && temp_map[hero_y][hero_x] == '%')
            {
                mvprintw(0, 2, "                        ");
                mvprintw(0, 2, "Bag of foods is full");
                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == 's')
            {
                bag.sword = true;
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Sword picked up!");
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
        }

        if (map[new_y][new_x] == '@' && temp_map[new_y][new_x] != '+')
        {
            int current_room = -1;
            for (int i = 0; i < floors[current_floor].num_rooms; i++)
            {
                if (hero_x >= floors[current_floor].rooms[i].x &&
                    hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                    hero_y >= floors[current_floor].rooms[i].y &&
                    hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                {
                    current_room = i;
                    break;
                }
            }

            if (current_room != -1 && floors[current_floor].rooms[current_room].is_room_locked &&
                (temp_map[new_y][new_x] == '@' || temp_map[new_y][new_x] == '+'))
            {
                if (security_mode)
                {
                    mvprintw(0, COLS / 2 - 7, "Security Mode");
                }
                else
                {
                    open_pass_window(current_floor, current_room);
                }
            }
            else if (!floors[current_floor].rooms[current_room].is_room_locked)
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                add_hero(hero_y, hero_x);
            }
        }

        if (temp_map[new_y][new_x] == '&')
        {
            int current_room = -1;
            for (int i = 0; i < floors[current_floor].num_rooms; i++)
            {
                if (hero_x >= floors[current_floor].rooms[i].x &&
                    hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                    hero_y >= floors[current_floor].rooms[i].y &&
                    hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                {
                    current_room = i;
                    break;
                }
            }

            if (current_room != -1)
            {
                generate_pass(current_floor, current_room);
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                add_hero(hero_y, hero_x);
            }
        }

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT &&
                (map[new_y][new_x] == '.' || map[new_y][new_x] == '+' ||
                 map[new_y][new_x] == '#' || map[new_y][new_x] == '^' ||
                 map[new_y][new_x] == '<' || map[new_y][new_x] == '>' ||
                 map[new_y][new_x] == 'K' || map[new_y][new_x] == '%' ||
                 map[new_y][new_x] == '$' || temp_map[new_y][new_x] == '?' ||
                 map[new_y][new_x] == 'b' || map[new_y][new_x] == '~' ||
                 map[new_y][new_x] == '!' || map[new_y][new_x] == 'H') ||
            map[new_y][new_x] == '*' || map[new_y][new_x] == '@' ||
            map[new_y][new_x] == 'W' || map[new_y][new_x] == 'a' ||
            map[new_y][new_x] == 'A' || map[new_y][new_x] == 'd' ||
            map[new_y][new_x] == '"' || map[new_y][new_x] == 'C' ||
            map[new_y][new_x] == 's')
        {
            if (temp_map[new_y][new_x] == '$')
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Gold +5");
                bag.gold_count += 5;
                score = (bag.gold_count * difficulty) / 3;
                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (map[new_y][new_x] == 'b')
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Gold +30");
                bag.gold_count += 30;
                score = (bag.gold_count * difficulty) / 3;
                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }

            if (map[new_y][new_x] == 'K')
            {
                mvprintw(0, 2, "                    ");

                mvprintw(0, 2, "Ancient Key Added to Your Bag");
                bag.has_ancient_key += 1;

                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x - 1] = '.';
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }

            if (temp_map[new_y][new_x] == '^')
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                hero_x = new_x;
                hero_y = new_y;

                map[hero_y][hero_x] = '@';

                health -= 5;
                draw_map();
                refresh();
                napms(1000);
            }
            if (floors[current_floor].temp_map[new_y][new_x] == '?')
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                hero_x = new_x;
                hero_y = new_y;

                map[hero_y][hero_x] = '@';

                draw_map();
                refresh();
            }
            else if (temp_map[new_y][new_x] == 'C')
            {
                create_final_room();
                new_x = hero_x;
                new_y = hero_y;
            }
            else
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
            }
            if (!final_room)
            {
                int current_room = -1;
                for (int i = 0; i < floors[current_floor].num_rooms; i++)
                {
                    if (hero_x >= floors[current_floor].rooms[i].x &&
                        hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                        hero_y >= floors[current_floor].rooms[i].y &&
                        hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                    {
                        current_room = i;
                        break;
                    }
                }
                if (floors[current_floor].rooms[current_room].is_room_enchanted)
                {
                    health -= 1;
                }
                if (old_room != current_room && old_room != -1)
                {
                    if (floors[current_floor].rooms[old_room].is_room_nightmare)
                    {
                        Room *old_room_ptr = &floors[current_floor].rooms[old_room];
                        for (int y = old_room_ptr->y; y < old_room_ptr->y + old_room_ptr->height; y++)
                        {
                            for (int x = old_room_ptr->x; x < old_room_ptr->x + old_room_ptr->width; x++)
                            {
                                discovered_map[y][x] = 0;
                            }
                        }
                    }
                }
                old_room = current_room;
                if (current_room != -1 && !floors[current_floor].rooms[current_room].is_room_nightmare)
                {
                    Room room = floors[current_floor].rooms[current_room];
                    for (int y = room.y; y < room.y + room.height; y++)
                    {
                        for (int x = room.x; x < room.x + room.width; x++)
                        {
                            discovered_map[y][x] = 1;
                        }
                    }
                    if (current_room == 5)
                    {
                        floors[current_floor].floor_discovered = true;
                    }
                }
                if (current_room != -1 && floors[current_floor].rooms[current_room].is_room_nightmare)
                {
                    Room room = floors[current_floor].rooms[current_room];
                    for (int y = room.y; y < room.y + room.height; y++)
                    {
                        for (int x = room.x; x < room.x + room.width; x++)
                        {
                            discovered_map[y][x] = 0;
                        }
                    }

                    for (int dy = -2; dy <= 2; dy++)
                    {
                        for (int dx = -2; dx <= 2; dx++)
                        {
                            int check_x = hero_x + dx;
                            int check_y = hero_y + dy;

                            if (check_x >= room.x && check_x < room.x + room.width &&
                                check_y >= room.y && check_y < room.y + room.height)
                            {
                                discovered_map[check_y][check_x] = 1;
                            }
                        }
                    }
                }
                else
                {
                    int discover_radius = 1;
                    for (int dy = -discover_radius; dy <= discover_radius; dy++)
                    {
                        for (int dx = -discover_radius; dx <= discover_radius; dx++)
                        {
                            int check_x = hero_x + dx;
                            int check_y = hero_y + dy;

                            if (check_x >= 0 && check_x < WIDTH &&
                                check_y >= 0 && check_y < HEIGHT)
                            {
                                discovered_map[check_y][check_x] = 1;
                            }
                        }
                    }
                }

                for (int y = 0; y < HEIGHT; y++)
                {
                    for (int x = 0; x < WIDTH; x++)
                    {
                        floors[current_floor].discovered_map[y][x] = discovered_map[y][x];
                    }
                }
            }
        }

        draw_map();

        refresh();
    }
}

void maps(char username[])
{
    current_floor = 0;
    final_room = false;
    int old_room = -1;
    clear();
    refresh();
    cleanup_game_memory();
    getmaxyx(stdscr, HEIGHT, WIDTH);
    init_health_system();

    generate_map();
    make_spec_rooms();
    add_traps();
    cleanup_game_memory();
    initialize_map();

    add_golds();
    add_spells();
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            map[i][j] = floors[current_floor].map[i][j];
        }
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
    srand(time(NULL));

    for (int i = 0; i < 6; i++)
    {
        rooms[i] = floors[current_floor].rooms[i];
    }

    char ch;

    hero_x = rooms[0].x + 1;
    hero_y = rooms[0].y + 1;
    int score = 0;
    map[hero_y][hero_x] = '@';
    init_bag();
    initialize_enemies();
    while (1)
    {
        check_food_changing();
        update_enemies();
        hit_damage();
        init_health(username);
        if (check_final_room_victory())
        {
            win_game(username);
        }
        draw_map();
        ch = getch();
        if (ch == 27)
        {
            pause_menu(username, hero_x, hero_y, score);
            break;
        }

        int new_x = hero_x;
        int new_y = hero_y;

        if (ch == 'j')
        {
            new_y -= get_movement_distance();
        }
        else if (ch == 'k')
        {
            new_y += get_movement_distance();
        }
        else if (ch == 'h')
        {
            new_x -= get_movement_distance();
        }
        else if (ch == 'l')
        {
            new_x += get_movement_distance();
        }
        else if (ch == 'y')
        {
            new_y -= get_movement_distance();
            new_x -= get_movement_distance();
        }
        else if (ch == 'u')
        {
            new_y -= get_movement_distance();
            new_x += get_movement_distance();
        }
        else if (ch == 'b')
        {
            new_y += get_movement_distance();
            new_x -= get_movement_distance();
        }
        else if (ch == 'n')
        {
            new_x += get_movement_distance();
            new_y += get_movement_distance();
        }
        else if (ch == 'e')
        {
            eat_food();
        }
        else if (ch == 'a')
        {
            replay_last_attack();
        }
        else if (ch == 'i')
        {
            open_weapon_window();
        }
        else if (ch == 'w')
        {
            open_spell_window();
        }
        else if (tolower(ch) == 27)
        {
            save_game(username, score);
            pause_menu(username, hero_x, hero_y, score);
        }
        else if (ch == 'm')
        {
            toggle_full_map();
        }

        else if (ch == ' ')
        {
            attack_with_weapon();
        }

        else if (ch == 'g' || ch == 'f')
        {
            move_special(ch);
            new_x = hero_x;
            new_y = hero_y;
        }

        else if (ch == '<' || ch == ',')
        {
            if (temp_map[hero_y][hero_x] == '<')
            {
                current_floor++;
                switch_floor();
                hero_y = floors[current_floor].rooms[0].down_stair_y;
                hero_x = floors[current_floor].rooms[0].down_stair_x;
                map[hero_y][hero_x] = '@';
                init_health_system();
                draw_map();
                mvaddch(hero_y, hero_x, '@');
                new_x = hero_x;
                new_y = hero_y;
            }
        }
        else if (ch == '>' || ch == '.')
        {
            if (temp_map[hero_y][hero_x] == '>')
            {
                current_floor--;
                hero_y = floors[current_floor].rooms[5].up_stair_y;
                hero_x = floors[current_floor].rooms[5].up_stair_x;
                switch_floor();
                map[hero_y][hero_x] = '@';
                init_health_system();
                draw_map();
                mvaddch(hero_y, hero_x, '@');
                new_x = hero_x;
                new_y = hero_y;
            }
        }
        else if (ch == 'x')
        {
            create_final_room();
            current_floor = 3;
            draw_enemies();
            update_enemies();
        }

        if (ch == 10)
        {

            if (temp_map[hero_y][hero_x] == 'd' || map[new_y][new_x] == 'D' ||
                temp_map[hero_y][hero_x] == 'a' || temp_map[hero_y][hero_x] == 'A' ||
                temp_map[hero_y][hero_x] == '*' || temp_map[hero_y][hero_x] == 'W' || temp_map[hero_y][hero_x] == 's')
            {
                handle_projectile_pickup(hero_x, hero_y);
            }

            if (temp_map[hero_y][hero_x] == '~' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Speed Spell +1");
                bag.speed_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == '!' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Damage Spell +1");
                bag.damage_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == 'H' && bag.total_spells != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Health Spell +1");
                bag.health_spells++;
                bag.total_spells++;
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if ((temp_map[hero_y][hero_x] == 'H' || map[hero_y][hero_x] == '!' ||
                      map[hero_y][hero_x] == '~') &&
                     bag.total_spells == 5)
            {
                mvprintw(0, 2, "Bag of spells is full");
                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == '%' && bag.total_foods != 5)
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Food picked up!");
                bag.food_pickup[bag.total_foods] = time(NULL);
                bag.food_type[bag.total_foods] = rand() % 3 + 1;
                bag.total_foods++;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (bag.total_foods == 5 && temp_map[hero_y][hero_x] == '%')
            {
                mvprintw(0, 2, "                        ");
                mvprintw(0, 2, "Bag of foods is full");
                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (temp_map[hero_y][hero_x] == 's')
            {
                bag.sword = true;
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Sword picked up!");
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = '.';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
        }

        if (map[new_y][new_x] == '@' && temp_map[new_y][new_x] != '+')
        {
            int current_room = -1;
            for (int i = 0; i < floors[current_floor].num_rooms; i++)
            {
                if (hero_x >= floors[current_floor].rooms[i].x &&
                    hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                    hero_y >= floors[current_floor].rooms[i].y &&
                    hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                {
                    current_room = i;
                    break;
                }
            }

            if (current_room != -1 && floors[current_floor].rooms[current_room].is_room_locked &&
                (temp_map[new_y][new_x] == '@' || temp_map[new_y][new_x] == '+'))
            {
                if (security_mode)
                {
                    mvprintw(0, COLS / 2 - 7, "Security Mode");
                }
                else
                {
                    open_pass_window(current_floor, current_room);
                }
            }
            else if (!floors[current_floor].rooms[current_room].is_room_locked)
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                add_hero(hero_y, hero_x);
            }
        }

        if (temp_map[new_y][new_x] == '&')
        {
            int current_room = -1;
            for (int i = 0; i < floors[current_floor].num_rooms; i++)
            {
                if (hero_x >= floors[current_floor].rooms[i].x &&
                    hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                    hero_y >= floors[current_floor].rooms[i].y &&
                    hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                {
                    current_room = i;
                    break;
                }
            }

            if (current_room != -1)
            {
                generate_pass(current_floor, current_room);
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                add_hero(hero_y, hero_x);
            }
        }

        if (new_x >= 0 && new_x < WIDTH && new_y >= 0 && new_y < HEIGHT &&
                (map[new_y][new_x] == '.' || map[new_y][new_x] == '+' ||
                 map[new_y][new_x] == '#' || map[new_y][new_x] == '^' ||
                 map[new_y][new_x] == '<' || map[new_y][new_x] == '>' ||
                 map[new_y][new_x] == 'K' || map[new_y][new_x] == '%' ||
                 map[new_y][new_x] == '$' || temp_map[new_y][new_x] == '?' ||
                 map[new_y][new_x] == 'b' || map[new_y][new_x] == '~' ||
                 map[new_y][new_x] == '!' || map[new_y][new_x] == 'H') ||
            map[new_y][new_x] == '*' || map[new_y][new_x] == '@' ||
            map[new_y][new_x] == 'W' || map[new_y][new_x] == 'a' ||
            map[new_y][new_x] == 'A' || map[new_y][new_x] == 'd' ||
            map[new_y][new_x] == '"' || map[new_y][new_x] == 'C' ||
            map[new_y][new_x] == 's')
        {
            if (temp_map[new_y][new_x] == '$')
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Gold +5");
                bag.gold_count += 5;
                score = (bag.gold_count * difficulty) / 3;
                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }
            else if (map[new_y][new_x] == 'b')
            {
                mvprintw(0, 2, "                    ");
                mvprintw(0, 2, "Gold +30");
                bag.gold_count += 30;
                score = (bag.gold_count * difficulty) / 3;
                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }

            if (map[new_y][new_x] == 'K')
            {
                mvprintw(0, 2, "                    ");

                mvprintw(0, 2, "Ancient Key Added to Your Bag");
                bag.has_ancient_key += 1;

                map[hero_y][hero_x] = '.';

                hero_x = new_x;
                hero_y = new_y;

                temp_map[hero_y][hero_x - 1] = '.';
                temp_map[hero_y][hero_x] = '.';
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                map[hero_y][hero_x] = '@';

                pthread_t tid1;
                pthread_create(&tid1, NULL, clear_msg, NULL);
                pthread_detach(tid1);
            }

            if (temp_map[new_y][new_x] == '^')
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];

                hero_x = new_x;
                hero_y = new_y;

                map[hero_y][hero_x] = '@';

                health -= 5;
                draw_map();
                refresh();
                napms(1000);
            }
            if (floors[current_floor].temp_map[new_y][new_x] == '?')
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                map[new_y][new_x] = '?';
                temp_map[new_y][new_x] = '?';
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                temp_map[hero_y][hero_x] = floors[current_floor].temp_map[hero_y][hero_x];
                // map[hero_y][hero_x] = '@';

                draw_map();
                refresh();
            }
            else if (temp_map[new_y][new_x] == 'C')
            {
                create_final_room();
                new_y = hero_y;
                new_x = hero_x;
            }
            else
            {
                map[hero_y][hero_x] = temp_map[hero_y][hero_x];
                hero_x = new_x;
                hero_y = new_y;
                map[hero_y][hero_x] = '@';
                add_hero(hero_y, hero_x);
            }
            if (!final_room)
            {
                int current_room = -1;
                for (int i = 0; i < floors[current_floor].num_rooms; i++)
                {
                    if (hero_x >= floors[current_floor].rooms[i].x &&
                        hero_x < floors[current_floor].rooms[i].x + floors[current_floor].rooms[i].width &&
                        hero_y >= floors[current_floor].rooms[i].y &&
                        hero_y < floors[current_floor].rooms[i].y + floors[current_floor].rooms[i].height)
                    {
                        current_room = i;
                        break;
                    }
                }
                if (current_room == 0 || current_room == 1)
                {
                    if (map[hero_y][hero_x + 1] == '=')
                    {
                        Room next_room = floors[current_floor].rooms[current_room + 1];
                        for (int y = next_room.y; y < next_room.y + next_room.height; y++)
                        {
                            for (int x = next_room.x; x < next_room.x + next_room.width; x++)
                            {
                                discovered_map[y][x] = 1;
                            }
                        }
                    }
                }
                else if (current_room == 3 || current_room == 4)
                {
                    if (map[hero_y][hero_x - 1] == '=')
                    {
                        Room next_room = floors[current_floor].rooms[current_room + 1];
                        for (int y = next_room.y; y < next_room.y + next_room.height; y++)
                        {
                            for (int x = next_room.x; x < next_room.x + next_room.width; x++)
                            {
                                discovered_map[y][x] = 1;
                            }
                        }
                    }
                }
                if (floors[current_floor].rooms[current_room].is_room_enchanted)
                {
                    health -= 1;

                    if (current_track_index != 4)
                    {
                        temp_track_index = current_track_index;
                        play_music(4);
                    }
                }
                if (old_room != current_room && old_room != -1)
                {

                    if (floors[current_floor].rooms[old_room].is_room_enchanted)
                    {

                        play_music(temp_track_index);
                    }

                    if (floors[current_floor].rooms[old_room].is_room_nightmare)
                    {
                        Room *old_room_ptr = &floors[current_floor].rooms[old_room];
                        for (int y = old_room_ptr->y; y < old_room_ptr->y + old_room_ptr->height; y++)
                        {
                            for (int x = old_room_ptr->x; x < old_room_ptr->x + old_room_ptr->width; x++)
                            {
                                discovered_map[y][x] = 0;
                            }
                        }

                        play_music(temp_track_index);
                    }
                }
                old_room = current_room;
                if (current_room != -1 && !floors[current_floor].rooms[current_room].is_room_nightmare)
                {
                    Room room = floors[current_floor].rooms[current_room];
                    for (int y = room.y; y < room.y + room.height; y++)
                    {
                        for (int x = room.x; x < room.x + room.width; x++)
                        {
                            discovered_map[y][x] = 1;
                        }
                    }
                    if (current_room == 5)
                    {
                        floors[current_floor].floor_discovered = true;
                    }
                }
                if (current_room != -1 && floors[current_floor].rooms[current_room].is_room_nightmare)
                {
                    if (current_track_index != 3)
                    {
                        if (current_track_index != 4 || current_track_index != 3)
                        {
                            temp_track_index = current_track_index;
                        }
                        play_music(3);
                    }

                    Room room = floors[current_floor].rooms[current_room];
                    for (int y = room.y; y < room.y + room.height; y++)
                    {
                        for (int x = room.x; x < room.x + room.width; x++)
                        {
                            discovered_map[y][x] = 0;
                        }
                    }

                    for (int dy = -2; dy <= 2; dy++)
                    {
                        for (int dx = -2; dx <= 2; dx++)
                        {
                            int check_x = hero_x + dx;
                            int check_y = hero_y + dy;

                            if (check_x >= room.x && check_x < room.x + room.width &&
                                check_y >= room.y && check_y < room.y + room.height)
                            {
                                discovered_map[check_y][check_x] = 1;
                            }
                        }
                    }
                }
                else
                {
                    int discover_radius = 1;
                    for (int dy = -discover_radius; dy <= discover_radius; dy++)
                    {
                        for (int dx = -discover_radius; dx <= discover_radius; dx++)
                        {
                            int check_x = hero_x + dx;
                            int check_y = hero_y + dy;

                            if (check_x >= 0 && check_x < WIDTH &&
                                check_y >= 0 && check_y < HEIGHT)
                            {
                                discovered_map[check_y][check_x] = 1;
                            }
                        }
                    }
                }

                for (int y = 0; y < HEIGHT; y++)
                {
                    for (int x = 0; x < WIDTH; x++)
                    {
                        floors[current_floor].discovered_map[y][x] = discovered_map[y][x];
                    }
                }
            }
        }

        draw_map();

        refresh();
    }
}
