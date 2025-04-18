#include "game.h"
#include <sqlite3.h>

void signup_login();

void initialize_database()
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("users.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char *sql = "CREATE TABLE IF NOT EXISTS Users ("
                      "Username TEXT PRIMARY KEY, "
                      "Password TEXT NOT NULL, "
                      "Email TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
}

void enemies_info()
{
    // Deamon : weakest enemy and doesnt move, doesnt have special ability
    /*
    Fire Breathing Monster :
    symbol : F
    special ability: may attack with 15 damage (Fire Breath)
    damage :
    health :
    */

    /*
    Frost Giant :
    symbol : G
    special ability: may attack with 15 damage (Frozen)

    */

    /*

    Snake:
    symbol: S
    special abilty: poison (5) damage
    damage :
    health:
   */

    /*
    Ghost:
    symbol : g
    special abilty : can move through all walls and will not be shown until it enters a room
    health
    damage:
    */

    /*
    Undead:
    symbol: U
    special abilty:
    */
}

void clear_error()
{
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(LINES - 2, i, " ");
    }
    for (int i = 0; i < COLS; i++)
    {
        mvprintw(LINES - 3, i, " ");
    }
}

void draw_color_menu(int current_selection)
{
    clear();

    attron(A_BOLD);
    box(stdscr, 0, 0);
    attroff(A_BOLD);

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);


    attron(A_BOLD | COLOR_PAIR(4));
    mvprintw(2, (COLS - 18) / 2, "HERO COLOR SELECT");
    mvhline(3, 2, ACS_BULLET, COLS - 4);
    attroff(A_BOLD | COLOR_PAIR(4));

    const char *options[] = {
        "White (Default)",
        "Red",
        "Green",
        "Blue",
        "Magenta",
        "Cyan"};

    int start_y = LINES / 3;
    int start_x = COLS / 2;

    for (int i = 0; i < 6; i++)
    {
        int y_pos = start_y + i * 2;
        int x_pos = start_x - strlen(options[i]) / 2;

        if (i == current_selection)
        {
            attron(A_BOLD | COLOR_PAIR(i + 1));
            mvprintw(y_pos, x_pos - 4, "► [ %s ]", options[i]);
            attroff(A_BOLD | COLOR_PAIR(i + 1));
        }
        else
        {
            attron(COLOR_PAIR(i + 1) | A_DIM);
            mvprintw(y_pos, x_pos, "%s", options[i]);
            attroff(COLOR_PAIR(i + 1) | A_DIM);
        }
    }

    attron(A_BOLD | COLOR_PAIR(4));
    mvhline(LINES - 3, 2, ACS_BULLET, COLS - 4);
    attroff(A_BOLD | COLOR_PAIR(4));

    mvprintw(LINES - 2, (COLS - 42) / 2,
             "ENTER: Select   ESC: Return to Pause Menu");

    refresh();
}

void change_appearance(char username[], int *hero_x, int *hero_y, int *score)
{
    int current_selection = 0;

    for (int i = 0; i < 6; i++)
    {
        if ((i == 0 && current_hero_color == 1) ||
            (i > 0 && current_hero_color == i + 2))
        {
            current_selection = i + 1;
            break;
        }
    }

    draw_color_menu(current_selection);

    while (1)
    {
        int choice = getch();

        switch (choice)
        {
        case KEY_UP:
            if (current_selection == 0)
            {
                current_selection = 5;
                draw_color_menu(current_selection);
            }
            else
            {
                current_selection--;
                draw_color_menu(current_selection);
            }
            break;

        case KEY_DOWN:
            if (current_selection == 5)
            {
                current_selection = 0;
                draw_color_menu(current_selection);
            }
            else
            {
                current_selection++;
                draw_color_menu(current_selection);
            }
            break;

        case '\n':
            current_hero_color = current_selection + 1;
            save_game(username, *score);
            continue_game(username);
            return;
        case 27:
            pre_game_menu(username);
            return;
        }
    }
}

void profile_page(char current_username[])
{
    clear();
    boarder();

    keypad(stdscr, TRUE);
    curs_set(0);

    char email[100];
    char score[20] = "0";

    FILE *users = fopen("usr.txt", "r");
    if (users != NULL)
    {
        char username[100], password[100], temp_email[100];
        while (fscanf(users, "%s %s %s", username, password, temp_email) != EOF)
        {
            if (strcmp(username, current_username) == 0)
            {
                strcpy(email, temp_email);
                break;
            }
        }
        fclose(users);
    }

    FILE *scores = fopen("scores.txt", "r");
    if (scores != NULL)
    {
        char username[100], temp_score[20];
        while (fscanf(scores, "%s %s", username, temp_score) != EOF)
        {
            if (strcmp(username, current_username) == 0)
            {
                strcpy(score, temp_score);
                break;
            }
        }
        fclose(scores);
    }

    char save_path[256];
    sprintf(save_path, "%s_save.dat", current_username);

    FILE *save_file = fopen(save_path, "rb");
    if (save_file != NULL)
    {
        SaveMetadata metadata;
        fread(&metadata, sizeof(SaveMetadata), 1, save_file);
        fclose(save_file);

        char time_str[100];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&metadata.save_time));

        mvprintw(LINES / 3 + 8, COLS / 3 + 2, "Last login: %s", time_str);
    }


    mvprintw(LINES / 4, COLS / 2 - 7, "PLAYER PROFILE");

    mvprintw(LINES / 3, COLS / 3, "Username: %s", current_username);
    mvprintw(LINES / 3 + 2, COLS / 3, "Email: %s", email);
    mvprintw(LINES / 3 + 4, COLS / 3, "Score: %s", score);
    mvprintw(LINES / 3 + 6, COLS / 3 + 2, "Games Played: %d", games_played);
    mvprintw(LINES / 3 + 7, COLS / 3 + 2, "Highest Score: %s", score);
    mvprintw(LINES - 2, COLS / 2 - 11, "Press ESC to return to menu");

    refresh();

    while (1)
    {
        int ch = getch();
        if (ch == 27)
        {
            clear();
            refresh();
            pre_game_menu(current_username);
            return;
        }
    }
}

void leader_board(int from_game, char username[], int hero_x, int hero_y, int score)
{
    clear();
    boarder();
    refresh();

    const int VISIBLE_ENTRIES = 5;
    const int START_Y = LINES / 2 - 3;
    const int START_X = COLS / 2 - 20;
    const int SCROLL_BAR_X = COLS / 2 + 15;

    mvprintw(2, COLS / 2 - 10, "╔═══════════════════╗");
    mvprintw(3, COLS / 2 - 10, "║    Leaderboard    ║");
    mvprintw(4, COLS / 2 - 10, "╚═══════════════════╝");

    typedef struct
    {
        char username[100];
        int score;
    } LeaderboardEntry;

    LeaderboardEntry leaderboard[100];
    int num_entries = 0;
    int current_position = 0;
    int selected_index = 0;

    FILE *scores = fopen("scores.txt", "r");
    if (scores != NULL)
    {
        char username[100];
        int score, games_played;
        while (fscanf(scores, "%s %d %d", username, &score, &games_played) != EOF)
        {
            strcpy(leaderboard[num_entries].username, username);
            leaderboard[num_entries].score = score;
            num_entries++;
        }
        fclose(scores);
    }

    for (int i = 0; i < num_entries - 1; i++)
    {
        for (int j = i + 1; j < num_entries; j++)
        {
            if (leaderboard[i].score < leaderboard[j].score)
            {
                LeaderboardEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }

    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK); // For Elden Lord
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // For GOAT
    init_pair(3, COLOR_BLUE, COLOR_BLACK);   // For Tarnished

    int scroll_bar_height = VISIBLE_ENTRIES;
    int max_scroll = (num_entries > VISIBLE_ENTRIES) ? num_entries - VISIBLE_ENTRIES : 0;

    while (1)
    {
        for (int i = START_Y; i < START_Y + VISIBLE_ENTRIES + 2; i++)
        {
            move(i, START_X - 2);
            clrtoeol();
        }

        attron(A_BOLD);
        mvprintw(START_Y, START_X, "┌──────┬──────────────────┬─────────┐");
        mvprintw(START_Y + 1, START_X, "│ Rank │    Username      │  Score  │");
        mvprintw(START_Y + 2, START_X, "├──────┼──────────────────┼─────────┤");
        attroff(A_BOLD);

        for (int i = 0; i < VISIBLE_ENTRIES && (i + current_position) < num_entries; i++)
        {
            int rank = i + current_position + 1;
            char medal[20] = "  ";
            char title[20] = "";

            if (rank == 1)
            {
                strcpy(medal, "🥇");
                attron(COLOR_PAIR(1));
                strcpy(title, "Elden Lord");
            }
            else if (rank == 2)
            {
                strcpy(medal, "🥈");
                attron(COLOR_PAIR(2));
                strcpy(title, "GOAT");
            }
            else if (rank == 3)
            {
                strcpy(medal, "🥉");
                attron(COLOR_PAIR(3));
                strcpy(title, "Tarnished");
            }

            bool is_current_user = (strcmp(leaderboard[i + current_position].username, username) == 0);

            if (i + current_position == selected_index)
                attron(A_REVERSE);
            if (is_current_user)
                attron(A_BOLD);

            if (is_current_user)
            {
                mvprintw(START_Y + 3 + i, START_X - 2, "→");
            }
            else
            {
                mvprintw(START_Y + 3 + i, START_X - 2, " ");
            }

            mvprintw(START_Y + 3 + i, START_X, "│ %-4d │ %-14s %s│ %-7d │ %s",
                     rank,
                     leaderboard[i + current_position].username, medal,
                     leaderboard[i + current_position].score,
                     title);

            if (rank <= 3)
                attroff(COLOR_PAIR(rank));
            if (is_current_user)
                attroff(A_BOLD);
            if (i + current_position == selected_index)
                attroff(A_REVERSE);
        }

        mvprintw(START_Y + VISIBLE_ENTRIES + 3, START_X, "└──────┴──────────────────┴─────────┘");

        mvprintw(LINES - 3, COLS / 2 - 19, "↑/↓: Navigate  │  ESC: Return to menu");

        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            if (selected_index > 0)
            {
                selected_index--;
                if (selected_index < current_position)
                    current_position = selected_index;
            }
            break;
        case KEY_DOWN:
            if (selected_index < num_entries - 1)
            {
                selected_index++;
                if (selected_index >= current_position + VISIBLE_ENTRIES)
                    current_position = selected_index - VISIBLE_ENTRIES + 1;
            }
            break;
        case KEY_PPAGE:
            current_position = (current_position > VISIBLE_ENTRIES) ? current_position - VISIBLE_ENTRIES : 0;
            selected_index = current_position;
            break;
        case KEY_NPAGE:
            current_position = (current_position + VISIBLE_ENTRIES < max_scroll) ? current_position + VISIBLE_ENTRIES : max_scroll;
            selected_index = current_position;
            break;
        case 27:
            clear();
            refresh();
            if (from_game == 1)
            {
                pause_menu(username, hero_x, hero_y, score);
                return;
            }
            pre_game_menu(NULL);
            return;
        }
    }
}

void pause_menu(char username[], int hero_x, int hero_y, int score)
{
    clear();
    refresh();
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_GREEN, COLOR_BLACK);

    const char *pause_options[] = {
        "Continue Game",
        "Save Progress",
        "Music Settings",
        "Customize Hero",
        "View Leaderboard",
        "Choose Difficulty",
        "Exit to Main Menu"};
    int num_options = 7;
    int selected = 0;

    keypad(stdscr, TRUE);
    curs_set(0);

    const char *title_art[] = {
        "╔═══════════════════════════════╗",
        "║          PAUSE  MENU          ║",
        "╚═══════════════════════════════╝"};

    while (1)
    {

        attron(COLOR_PAIR(1) | A_BOLD);
        for (int i = 0; i < 3; i++)
        {
            mvprintw(1 + i, COLS / 2 - 16, "%s", title_art[i]);
        }
        attroff(COLOR_PAIR(1) | A_BOLD);

        attron(COLOR_PAIR(2));
        mvprintw(LINES / 3 - 5, COLS / 2 - 20, "Player: %s", username);
        mvprintw(LINES / 3 - 5, COLS / 2 + 5, "Score: %d", score);
        attroff(COLOR_PAIR(2));

        for (int i = 0; i < num_options; i++)
        {
            if (i == selected)
            {
                attron(COLOR_PAIR(3) | A_BOLD);
                if (i == 0 || i == 6)
                {
                    if (i == 0)
                    {
                        mvprintw(LINES / 3 + 2, COLS / 2 - strlen(pause_options[1]) / 2 - 2, " ");
                        mvprintw(LINES / 3 + 6 * 2, COLS / 2 - strlen(pause_options[6]) / 2 - 2, " ");
                    }
                    if (i == 6)
                    {
                        mvprintw(LINES / 3 + 10, COLS / 2 - strlen(pause_options[5]) / 2 - 2, " ");
                        mvprintw(LINES / 3, COLS / 2 - strlen(pause_options[0]) / 2 - 2, " ");
                    }
                }
                else
                {
                    mvprintw(LINES / 3 + (i - 1) * 2, COLS / 2 - strlen(pause_options[i - 1]) / 2 - 2, " ");
                    mvprintw(LINES / 3 + (i + 1) * 2, COLS / 2 - strlen(pause_options[i + 1]) / 2 - 2, " ");
                }

                mvprintw(LINES / 3 + i * 2, COLS / 2 - strlen(pause_options[i]) / 2 - 2, "► ");
                mvprintw(LINES / 3 + i * 2, COLS / 2 - strlen(pause_options[i]) / 2, "%s", pause_options[i]);
                attroff(COLOR_PAIR(3) | A_BOLD);
            }
            else
            {
                attron(COLOR_PAIR(2));
                mvprintw(LINES / 3 + i * 2, COLS / 2 - strlen(pause_options[i]) / 2, "%s", pause_options[i]);
                attroff(COLOR_PAIR(2));
            }
        }

        attron(COLOR_PAIR(4));
        mvprintw(LINES - 3, COLS / 2 - 30, "↑↓: Navigate   |   Enter: Select   |   ESC: Return to Game");
        attroff(COLOR_PAIR(4));

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

        case 10:
            switch (selected)
            {
            case 0:
                clear();
                continue_game(username);

            case 1:
                save_game(username, score);
                attron(COLOR_PAIR(5));
                mvprintw(LINES - 5, COLS / 2 - 13, "✔ Game saved successfully!");
                attroff(COLOR_PAIR(3));
                refresh();
                napms(1500);
                break;

            case 2:
                music_setting(3, username, score);
                break;

            case 3:
                change_appearance(username, &hero_x, &hero_y, &score);
                break;
            case 4:
                leader_board(1, username, hero_x, hero_y, score);
                break;

            case 5:
                choose_difficulty();
                pre_game_menu(username);
                break;

            case 6:
                clear();
                cleanup_game_memory();
                pre_game_menu(username);
                return;
            }
            break;

        case 27:
            clear();
            continue_game(username);
            return;
        }
    }
}

typedef struct
{
    char usrname[100];
    char pass[100];
    char email[100];
    char security[100];
} user;

void new_game(char username[])
{
    clear();
    boarder();
    maps(username);
    refresh();
}

void pre_game_menu(char usrname[])
{

    if (temp_track_index != 0)
    {
        if (current_track_index == 5)
        {
            play_music(temp_track_index);
        }
    }

    enum ColorPairs
    {
        TITLE_PAIR = 1,
        SELECTED_PAIR,
        NORMAL_PAIR,
        ACCENT_PAIR,
        HEADER_PAIR
    };

    clear();
    start_color();
    init_pair(TITLE_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(SELECTED_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(NORMAL_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(ACCENT_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(HEADER_PAIR, COLOR_WHITE, COLOR_BLACK);

    keypad(stdscr, TRUE);
    curs_set(0);

    const struct
    {
        const char *title;
        const char *description;
        void (*action)(void);
    } menu_items[] = {
        {"Continue", "Load your saved game", NULL},
        {"New Game", "Start a new game", NULL},
        {"Choose Difficulty", "Set game difficulty", NULL},
        {"Leader Board", "View high scores", NULL},
        {"Profile", "View your profile", NULL},
        {"Enemies Info", "Game information", NULL},
        {"Quit", "Exit game", NULL}};

    const int num_options = sizeof(menu_items) / sizeof(menu_items[0]);
    int selected_option = 0;
    int max_title_length = 0;
    int max_desc_length = 0;

    for (int i = 0; i < num_options; i++)
    {
        int title_len = strlen(menu_items[i].title);
        int desc_len = strlen(menu_items[i].description);
        max_title_length = (title_len > max_title_length) ? title_len : max_title_length;
        max_desc_length = (desc_len > max_desc_length) ? desc_len : max_desc_length;
    }

    const char *cursor = "•";

    int menu_width = max_title_length + 8;
    int desc_width = max_desc_length;
    int total_width = (menu_width > desc_width) ? menu_width : desc_width;

    while (1)
    {
        clear();
        boarder();

        const char header_text[] = "━━━━━━━━━━ MAIN MENU ━━━━━━━━━━";
        int header_x = (COLS - strlen(header_text)) / 2;

        attron(COLOR_PAIR(HEADER_PAIR) | A_BOLD);
        mvprintw(1, COLS / 2 - 16, "%s", header_text);
        attroff(COLOR_PAIR(HEADER_PAIR) | A_BOLD);

        char welcome_msg[100];
        snprintf(welcome_msg, sizeof(welcome_msg), "Welcome, %s", usrname);
        attron(COLOR_PAIR(ACCENT_PAIR) | A_BOLD);
        mvprintw(5, (COLS - strlen(welcome_msg)) / 2, "%s", welcome_msg);
        attroff(COLOR_PAIR(ACCENT_PAIR) | A_BOLD);

        int total_menu_height = num_options * 3;
        int menu_start_y = (LINES - total_menu_height) / 2;

        for (int i = 0; i < num_options; i++)
        {
            int y_pos = menu_start_y + i * 3;
            bool is_selected = (i == selected_option);

            int menu_start_x = (COLS - total_width) / 2;
            int title_x = menu_start_x + (total_width - strlen(menu_items[i].title)) / 2;
            int desc_x = menu_start_x + (total_width - strlen(menu_items[i].description)) / 2;

            if (is_selected)
            {
                attron(COLOR_PAIR(SELECTED_PAIR) | A_BOLD | A_REVERSE);
                mvprintw(y_pos, title_x, "%s", menu_items[i].title);
                attroff(A_REVERSE);
                attron(A_DIM);
                mvprintw(y_pos + 1, desc_x, "%s", menu_items[i].description);
                attroff(A_DIM);
                attroff(COLOR_PAIR(SELECTED_PAIR) | A_BOLD);
            }
            else
            {
                attron(COLOR_PAIR(NORMAL_PAIR));
                mvprintw(y_pos, title_x, "%s", menu_items[i].title);
                attroff(COLOR_PAIR(NORMAL_PAIR));
            }
        }

        const char *help_text = "Navigate: ↑/↓   Select: Enter   Quit: Q";
        attron(COLOR_PAIR(ACCENT_PAIR));
        mvprintw(LINES - 3, (COLS - strlen(help_text)) / 2, "%s", help_text);
        attroff(COLOR_PAIR(ACCENT_PAIR));

        refresh();

        int ch = getch();

        switch (ch)
        {
        case KEY_UP:
            selected_option = (selected_option == 0) ? num_options - 1 : selected_option - 1;
            break;
        case KEY_DOWN:
            selected_option = (selected_option == num_options - 1) ? 0 : selected_option + 1;
            break;
        case 'q':
        case 'Q':
            signup_login();
            break;
        case 10: 
            cleanup_game_memory();
            clear();

            switch (selected_option)
            {
            case 0:
                check_save_exists(usrname);
                break;
            case 1:
                new_game(usrname);
                break;
            case 2:
                choose_difficulty();
                pre_game_menu(usrname);
                break;
            case 3:
                leader_board(0, usrname, 0, 0, 0);
                break;
            case 4:
                profile_page(usrname);
                break;
            case 5:
                enemies_info();
                pre_game_menu(usrname);
                break;
            case 6:
                signup_login();
                break;
            }
            break;
        }
    }
}

void print_email_error();

void forgot_pass_page()
{
    boarder();
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("users.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char *forgot_options[] = {"username:", "email:", "show password"};
    char username[100] = {0};
    char email[100] = {0};
    int option = 0;
    bool found = false;

    while (1)
    {
        clear();
        boarder();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(2, COLS / 2 - 8, "Forgot Password?");
        attroff(COLOR_PAIR(1) | A_BOLD);

        for (int i = 0; i < 3; i++)
        {
            if (i == option)
                attron(A_REVERSE);
            mvprintw(LINES / 2 + i * 3, COLS / 2 - strlen(forgot_options[i]) / 2, "%s", forgot_options[i]);
            if (i == option)
                attroff(A_REVERSE);

            if (i == 0 && username[0])
                mvprintw(LINES / 2, COLS / 2 + strlen(forgot_options[0]) + 1, "%s", username);
            if (i == 1 && email[0])
                mvprintw(LINES / 2 + 3, COLS / 2 + strlen(forgot_options[1]) + 1, "%s", email);
        }
        refresh();

        int ch = getch();
        if (ch == 27)
        {
            sqlite3_close(db);
            login_page();
            return;
        }

        if (ch == KEY_UP)
        {
            option = (option == 0) ? 2 : option - 1;
        }
        else if (ch == KEY_DOWN)
        {
            option = (option == 2) ? 0 : option + 1;
        }
        else if (ch == 10)
        {
            if (option == 0 || option == 1)
            {
                echo();
                curs_set(TRUE);
                char *input = (option == 0) ? username : email;
                move(LINES / 2 + option * 3, COLS / 2 + strlen(forgot_options[option]) + 1);
                getstr(input);
                noecho();
                curs_set(FALSE);
            }
            else if (option == 2)
            {
                char *sql = sqlite3_mprintf(
                    "SELECT Password FROM Users WHERE Username = '%q' AND Email = '%q';",
                    username, email);

                char *password = NULL;
                sqlite3_stmt *stmt;
                rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

                if (rc == SQLITE_OK)
                {
                    if (sqlite3_step(stmt) == SQLITE_ROW)
                    {
                        found = true;
                        const char *pwd = (const char *)sqlite3_column_text(stmt, 0);

                        init_pair(2, COLOR_BLUE, COLOR_BLACK);
                        attron(COLOR_PAIR(2) | A_BOLD);
                        mvprintw(LINES / 2 + 10, COLS / 2 - strlen("Your password is: ") / 2,
                                 "Your password is: %s", pwd);
                        attroff(COLOR_PAIR(2) | A_BOLD);
                    }
                }

                sqlite3_finalize(stmt);
                sqlite3_free(sql);

                if (!found)
                {
                    init_pair(1, COLOR_WHITE, COLOR_RED);
                    attron(COLOR_PAIR(1));
                    mvprintw(LINES - 3, COLS / 2 - 13, "Invalid username or email");
                    attroff(COLOR_PAIR(1));
                }

                refresh();

                mvprintw(LINES - 2, COLS / 2 - 15, "Press esc twice to go back...");
                refresh();
                getch();
            }
        }
    }
}

void signup_login();

void boarder()
{
    clear();
    for (int i = 1; i < COLS - 1; i++)
    {
        mvprintw(1, i, "█");
        mvprintw(LINES - 2, i, "█");
    }
    for (int i = 2; i < LINES - 2; i++)
    {
        mvprintw(i, 1, "█");
        mvprintw(i, COLS - 2, "█");
    }

    mvprintw(1, 1, "╔");
    mvprintw(1, COLS - 2, "╗");
    mvprintw(LINES - 2, 1, "╚");
    mvprintw(LINES - 2, COLS - 2, "╝");
}

void login_page()
{
    clear();
    boarder();
    refresh();
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    const char *title_art[] = {
        "╔═══════════════════════════════╗",
        "║             Login             ║",
        "╚═══════════════════════════════╝"};

    attron(COLOR_PAIR(2) | A_BOLD);
    for (int i = 0; i < 3; i++)
    {
        mvprintw(i + 1, COLS / 2 - 16, "%s", title_art[i]);
    }
    attroff(COLOR_PAIR(2) | A_BOLD);

    curs_set(FALSE);
    keypad(stdscr, TRUE);

    char usrname[100] = {0};
    char password[100] = {0};
    const char *login_options[] = {"Username:", "Password:", "Forgot your password?", "Continue"};
    int option = 0;

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            if (i == option)
            {
                attron(A_REVERSE);
            }
            mvprintw(LINES / 2 + i * 2 - 6, COLS / 2 - strlen(login_options[i]) / 2, "%s", login_options[i]);
            if (i == option)
            {
                attroff(A_REVERSE);
            }
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP)
        {
            option = (option == 0) ? 3 : option - 1;
        }
        else if (ch == KEY_DOWN)
        {
            option = (option == 3) ? 0 : option + 1;
        }
        if (ch == 27)
        {
            signup_login();
        }
        if (ch == 10)
        {
            curs_set(FALSE);
            noecho();

            if (option == 0)
            {
                echo();
                curs_set(TRUE);
                move(LINES / 2 - 6, COLS / 2 + strlen(login_options[0]) / 2 + 1);
                getstr(usrname);
                noecho();
            }
            else if (option == 1)
            {
                echo();
                curs_set(TRUE);
                move(LINES / 2 + 2 - 6, COLS / 2 + strlen(login_options[1]) / 2 + 1);
                getstr(password);
                noecho();
            }
            else if (option == 2)
            {
                forgot_pass_page();
            }
            if (option == 3)
            {
                if (strlen(usrname) == 0 || strlen(password) == 0)
                {
                    mvprintw(LINES - 3, COLS / 2 - 15, "Please enter both username and password");
                    refresh();
                    continue;
                }

                sqlite3 *db;
                int rc = sqlite3_open("users.db", &db);

                if (rc != SQLITE_OK)
                {
                    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                char sql[256];
                snprintf(sql, sizeof(sql), "SELECT * FROM Users WHERE Username = '%s' AND Password = '%s';", usrname, password);

                sqlite3_stmt *stmt;
                rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

                if (rc != SQLITE_OK)
                {
                    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    pre_game_menu(usrname);
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    return;
                }
                else
                {
                    mvprintw(LINES - 3, COLS / 2 - 15, "Invalid username or password");
                    refresh();
                    memset(usrname, 0, sizeof(usrname));
                    memset(password, 0, sizeof(password));
                }

                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
        }
    }
}

void save_user(user usr)
{
    sqlite3 *db;
    char *err_msg = 0;
    int rc = sqlite3_open("users.db", &db);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO Users (Username, Password, Email) VALUES ('%s', '%s', '%s');",
             usr.usrname, usr.pass, usr.email);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
}

char *rand_pass()
{
    const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char lower[] = "abcdefghijklmnopqrstuvwxyz";
    const char digits[] = "0123456789";
    const char all[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    int length = 10;
    char *random = (char *)malloc((length + 1) * sizeof(char));

    srand(time(NULL));
    random[0] = upper[rand() % strlen(upper)];
    random[1] = lower[rand() % strlen(lower)];
    random[2] = digits[rand() % strlen(digits)];

    for (int i = 3; i < length; i++)
    {
        random[i] = all[rand() % strlen(all)];
    }

    for (int i = 0; i < length; i++)
    {
        int random_index = rand() % length;
        char temp = random[i];
        random[i] = random[random_index];
        random[random_index] = temp;
    }

    random[length] = '\0';
    return random;
}

void print_passlen_error()
{
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
    char error_msg[] = {"ERROR, password must have the minimum length of 7 characters and the maximum length of 20 characters"};
    {
        refresh();
        mvprintw(LINES - 2, COLS / 2 - strlen(error_msg) / 2, error_msg);
        attroff(COLOR_PAIR(1));
    }
}

void print_characters_error()
{
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
    char error_msg[] = {"ERROR, password must have at least 1 upper character, 1 lower character and 1 digit"};
    mvprintw(LINES - 2, COLS / 2 - strlen(error_msg) / 2, error_msg);
    refresh();
    attroff(COLOR_PAIR(2));
}

int check_lenght(const char *password)
{
    if (strlen(password) >= 7 && strlen(password) <= 20)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int check_digit(char password[])
{
    int pass_len = strlen(password);
    int check = 0;
    for (int i = 0; i < pass_len; i++)
    {
        if (isdigit(password[i]))
        {
            check = 1;
            break;
        }
    }
    return check;
}

int check_lower(char password[])
{
    int pass_len = strlen(password);
    int check = 0;
    for (int i = 0; i < pass_len; i++)
    {
        if (islower(password[i]))
        {
            check = 1;
            break;
        }
    }
    return check;
}

int check_upper(const char *password)
{
    int pass_len = strlen(password);
    int check = 0;
    for (int i = 0; i < pass_len; i++)
    {
        if (isupper(password[i]))
        {
            check = 1;
            break;
        }
    }
    return check;
}

void print_email_error()
{
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
    char error_msg[] = {"ERROR, email must be in the form of (xxx@yy.z)"};
    mvprintw(LINES - 2, COLS / 2 - strlen(error_msg) / 2, error_msg);
    refresh();
    attroff(COLOR_PAIR(1));
}

void print_save_error()
{
    init_pair(1, COLOR_WHITE, COLOR_RED);
    attron(COLOR_PAIR(1));
    char error_msg[] = {"please complete sign_up"};
    mvprintw(LINES - 2, COLS / 2 - strlen(error_msg) / 2, error_msg);
    refresh();
    attroff(COLOR_PAIR(1));
}

// Function to valiadate email to be in the form of (xxx@yy.z)
int validate_email(const char *email)
{
    int email_len = strlen(email);
    int at_pos = -1;
    int dot_pos = -1;

    for (int i = 0; i < email_len; i++)
    {
        if (email[i] == '@')
        {
            if (at_pos != -1)
            {
                return 0;
            }
            at_pos = i;
        }
    }

    if (at_pos == -1 || at_pos == 0 || at_pos == email_len - 1)
    {
        return 0;
    }

    for (int i = at_pos + 1; i < email_len; i++)
    {
        if (email[i] == '.')
        {
            dot_pos = i;
        }
    }

    if (dot_pos == -1 || dot_pos == email_len - 1)
    {
        return 0;
    }

    if (dot_pos - at_pos <= 1)
    {
        return 0;
    }

    for (int i = dot_pos + 1; i < email_len; i++)
    {
        if (!isalpha(email[i]))
        {
            return 0;
        }
    }

    if (email_len - dot_pos <= 2)
    {
        return 0;
    }

    return 1;
}

void sign_up_page()
{
    boarder();
    init_pair(2, COLOR_CYAN, COLOR_BLACK);

    const char *title_art[] = {
        "╔═══════════════════════════════╗",
        "║            Sign Up            ║",
        "╚═══════════════════════════════╝"};

    attron(COLOR_PAIR(2) | A_BOLD);
    for (int i = 0; i < 3; i++)
    {
        mvprintw(i + 1, COLS / 2 - 16, "%s", title_art[i]);
    }
    attroff(COLOR_PAIR(2) | A_BOLD);
    refresh();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    const char *sign_up_options[] = {"Username:", "e-mail:", "Password:", "Random Password", "Save"};

    char usrname[100];
    char pass[100];
    char email[100];
    user new_usr;
    user user;
    FILE *users = fopen("usr.text", "r");

    int pass_check = 0;
    int email_check = 0;
    int usrname_check = 0;

    int option = 0;
    while (1)
    {
        for (int i = 0; i < 5; i++)
        {
            if (i == option)
            {
                attron(A_REVERSE);
            }
            mvprintw(LINES / 2 - 6 + i * 2, COLS / 2 - strlen(sign_up_options[i]) / 2, "%s", sign_up_options[i]);
            if (i == option)
            {
                attroff(A_REVERSE);
            }
        }
        refresh();

        int ch = getch();
        if (ch == KEY_UP)
        {
            option = (option == 0) ? 4 : option - 1;
        }
        else if (ch == KEY_DOWN)
        {
            option = (option == 4) ? 0 : option + 1;
        }
        if (ch == 27)
        {
            fclose(users);
            signup_login();
            return;
        }
        if (ch == 10)
        {
            curs_set(FALSE);
            noecho();
            if (option == 0)
            {
                echo();
                curs_set(TRUE);
                move(LINES / 2 - 6, COLS / 2 + strlen(sign_up_options[0]) / 2 + 1);
                getstr(new_usr.usrname);

                sqlite3 *db;
                int rc = sqlite3_open("users.db", &db);

                if (rc != SQLITE_OK)
                {
                    fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                char sql[256];
                snprintf(sql, sizeof(sql), "SELECT * FROM Users WHERE Username = '%s';", new_usr.usrname);

                sqlite3_stmt *stmt;
                rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

                if (rc != SQLITE_OK)
                {
                    fprintf(stderr, "Failed to fetch data: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                if (sqlite3_step(stmt) == SQLITE_ROW)
                {
                    clear_error();
                    mvprintw(LINES - 3, COLS / 2 - 11, "Username already taken!");
                    usrname_check = 0;
                }
                else
                {
                    usrname_check = 1;
                }

                sqlite3_finalize(stmt);
                sqlite3_close(db);

                if (usrname_check)
                {
                    clear_error();
                }
                curs_set(FALSE);
                noecho();
            }
            if (option == 1)
            {
                clear_error();
                echo();
                curs_set(TRUE);
                move(LINES / 2 + 2 - 6, COLS / 2 + strlen(sign_up_options[1]) / 2 + 1);
                getstr(new_usr.email);
                email_check = 1;
                if (!validate_email(new_usr.email))
                {
                    clear_error();
                    print_email_error();
                    email_check = 0;
                }
                curs_set(FALSE);
                noecho();
            }
            if (option == 2)
            {
                echo();
                move(LINES / 2 + 4 - 6, COLS / 2 + strlen(sign_up_options[2]) / 2 + 1);
                getstr(pass);
                strcpy(new_usr.pass, pass);
                pass_check = 1;
                if (!check_lenght(new_usr.pass))
                {
                    clear_error();
                    print_passlen_error();
                    pass_check = 0;
                }
                else if (!check_upper(new_usr.pass) || !check_lower(new_usr.pass) || !check_digit(new_usr.pass))
                {
                    clear_error();
                    print_characters_error();
                    pass_check = 0;
                }
                curs_set(FALSE);
            }
            if (option == 3)
            {
                clear_error();
                char *random_pass = rand_pass();
                mvprintw(LINES / 2 + 4 - 6, COLS / 2 + strlen(sign_up_options[2]) / 2 + 1, "%s", random_pass);
                strcpy(new_usr.pass, random_pass);
                pass_check = 1;
            }
            if (option == 4)
            {
                if (pass_check && email_check && usrname_check)
                {
                    save_user(new_usr);
                    fclose(users);
                    signup_login();
                    return;
                }
                else
                {
                    clear_error();
                    print_save_error();
                }
            }
        }
    }
}

void signup_login()
{
    clear();
    boarder();

    init_pair(1, COLOR_CYAN, COLOR_BLACK);  
    init_pair(2, COLOR_GREEN, COLOR_BLACK); 
    init_pair(3, COLOR_BLUE, COLOR_BLACK); 

    const char *title_art[] = {
        "╔══════════════════════════════════════╗",
        "║         Game Access Terminal         ║",
        "╚══════════════════════════════════════╝"};

    attron(COLOR_PAIR(3) | A_BOLD);
    for (int i = 0; i < 3; i++)
    {
        mvprintw(i + 2, COLS / 2 - 20, "%s", title_art[i]);
    }
    attroff(COLOR_PAIR(3) | A_BOLD);

    const char *first_menu[] = {
        "🔐 Sign Up",
        "🔒 Login",
        "👤 Guest Access",
        "🎵 Music Settings"};

    keypad(stdscr, TRUE);
    curs_set(0);

    int choice = 0;
    int quit = 0;

    while (!quit)
    {
        for (int i = 0; i < 4; i++)
        {
            mvprintw(LINES / 2 + i * 2, COLS / 2 - 10, "                ");
        }

        for (int i = 0; i < 4; i++)
        {
            if (i == choice)
            {
                attron(A_REVERSE);
                mvprintw(LINES / 2 + i * 2, COLS / 2 - 10, first_menu[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(LINES / 2 + i * 2, COLS / 2 - 10, first_menu[i]);
            }
        }

        attron(COLOR_PAIR(1) | A_DIM);
        mvprintw(LINES - 4, COLS / 2 - 15, "↑ Navigate    ⏎ Select    ESC Quit");
        attroff(COLOR_PAIR(1) | A_DIM);

        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            choice = (choice == 0) ? 3 : choice - 1;
            break;
        case KEY_DOWN:
            choice = (choice == 3) ? 0 : choice + 1;
            break;
        case 10: 
            switch (choice)
            {
            case 0:
                sign_up_page();
                quit = 1;
                break;
            case 1:
                login_page();
                quit = 1;
                break;
            case 2:
            {
                char username[] = "guest";
                pre_game_menu(username);
                quit = 1;
                break;
            }
            case 3:
                music_setting(1, NULL, 0);
                quit = 1;
                break;
            }
            break;
        case 27: 
            clear();
            endwin();
            exit(0);
        }
    }
}
