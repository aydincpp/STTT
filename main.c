#define _XOPEN_SOURCE_EXTENDED 1
#include <ctype.h>
#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define WINDOW_PADDING 4
#define MAX_WORD_CH 30
#define MAX_LINE_NUM 20
#define MAX_LINE_WORDS 20

typedef struct {
    int padding;
    int lines;
    int cols;
    int start_line;
    int start_col;
} g_Window;

typedef struct {
    char key;
    int x;
    int y;
} KeyPosition;

KeyPosition keyboard[] = {
    { 'Q', 2, 1 }, { 'W', 6, 1 }, { 'E', 10, 1 }, { 'R', 14, 1 }, { 'T', 18, 1 },
    { 'Y', 22, 1 }, { 'U', 26, 1 }, { 'I', 30, 1 }, { 'O', 34, 1 }, { 'P', 38, 1 },
    { 'A', 4, 3 }, { 'S', 8, 3 }, { 'D', 12, 3 }, { 'F', 16, 3 }, { 'G', 20, 3 },
    { 'H', 24, 3 }, { 'J', 28, 3 }, { 'K', 32, 3 }, { 'L', 36, 3 },
    { 'Z', 6, 5 }, { 'X', 10, 5 }, { 'C', 14, 5 }, { 'V', 18, 5 }, { 'B', 22, 5 },
    { 'N', 26, 5 }, { 'M', 30, 5 }
};

const char* g_file = "words.txt";
int g_ui_width = 40;

g_Window g_header_win;
g_Window g_main_win;
g_Window g_type_win;
g_Window g_footer_win;
g_Window g_keyboard_win;

int isSpace(const char);
int isPrint(const char);
int getAvailableCols(const g_Window* win);
int getContentXOffset(int cols, int content_width);
int scale_to_1000(int);
void draw_words(WINDOW* win, char words[][MAX_WORD_CH], int words_num, int current_word, int cols);

void draw_keyboard(WINDOW* win, int keyboard_size)
{
    curs_set(0);
    for (int i = 0; i < keyboard_size; i++) {
        mvwprintw(win, keyboard[i].y, keyboard[i].x, "[%c]", keyboard[i].key);
    }
    wrefresh(win);
}

void highlight_key(WINDOW* win, int ch, int keyboard_size)
{
    ch = toupper(ch);
    for (int i = 0; i < keyboard_size; i++) {
        if (keyboard[i].key == ch) {
            wattron(win, A_REVERSE);
            mvwprintw(win, keyboard[i].y, keyboard[i].x, "[%c]", ch);
            wattroff(win, A_REVERSE);
            wrefresh(win);
            break;
        }
    }
}

void draw_header(WINDOW* win)
{
    wchar_t* text = L"󰌌 Simple Terminal Typing Test ";
    mvwaddwstr(win, g_header_win.lines / 2, g_header_win.padding, text);
    wrefresh(win);
}

void draw_footer(WINDOW* win)
{
    wchar_t* text = L" github.com/aydincpp"; // nf-md-speedometer
    mvwaddwstr(win, g_footer_win.lines / 2, g_footer_win.padding, text);
    wrefresh(win);
}

int main()
{
    initscr();
    start_color();
    cbreak();
    noecho();
    nodelay(stdscr, 1);
    setlocale(LC_ALL, "");

    int keyboard_size = sizeof(keyboard) / sizeof(KeyPosition);

    if (can_change_color()) {
        int bg_r = scale_to_1000(30);
        int bg_g = scale_to_1000(30);
        int bg_b = scale_to_1000(30);
        init_color(COLOR_BLACK, bg_r, bg_g, bg_b);

        int text_r = scale_to_1000(200);
        int text_g = scale_to_1000(200);
        int text_b = scale_to_1000(200);
        init_color(COLOR_WHITE, text_r, text_g, text_b);

        int green_r = scale_to_1000(80);
        int green_g = scale_to_1000(250);
        int green_b = scale_to_1000(160);
        init_color(COLOR_GREEN, green_r, green_g, green_b);

        int red_r = scale_to_1000(255);
        int red_g = scale_to_1000(100);
        int red_b = scale_to_1000(100);
        init_color(COLOR_RED, red_r, red_g, red_b);

        int yellow_r = scale_to_1000(255);
        int yellow_g = scale_to_1000(204);
        int yellow_b = scale_to_1000(0);
        init_color(COLOR_YELLOW, yellow_r, yellow_g, yellow_b);
    }

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);

    bkgd(COLOR_PAIR(1));
    refresh();

    int content_width = (g_ui_width / 100.0) * COLS;
    int content_x_offset = getContentXOffset(COLS, content_width);

    g_header_win.padding = WINDOW_PADDING;
    g_header_win.lines = (((LINES * 10) / 100) % 2 == 0 ? ((LINES * 10) / 100) + 1 : ((LINES * 10) / 100));
    g_header_win.cols = (COLS * g_ui_width) / 100;
    g_header_win.start_line = 0;
    g_header_win.start_col = content_x_offset;

    g_footer_win.padding = WINDOW_PADDING;
    g_footer_win.lines = (((LINES * 10) / 100) % 2 == 0 ? ((LINES * 10) / 100) + 1 : ((LINES * 10) / 100));
    g_footer_win.cols = (COLS * g_ui_width) / 100;
    g_footer_win.start_line = (LINES - g_footer_win.lines);
    g_footer_win.start_col = content_x_offset;

    g_main_win.padding = WINDOW_PADDING;
    g_main_win.lines = LINES - (g_header_win.lines + g_footer_win.lines);
    g_main_win.cols = (COLS * g_ui_width) / 100;
    g_main_win.start_line = (g_header_win.start_line + g_header_win.lines);
    g_main_win.start_col = content_x_offset;

    g_type_win.padding = WINDOW_PADDING;
    g_type_win.lines = 5;
    g_type_win.cols = MAX_WORD_CH + g_type_win.padding;
    g_type_win.start_line = (g_main_win.lines - 12);
    g_type_win.start_col = content_x_offset + getContentXOffset(g_main_win.cols, g_type_win.cols);

    g_keyboard_win.padding = WINDOW_PADDING;
    g_keyboard_win.lines = 8;
    g_keyboard_win.cols = 44;
    g_keyboard_win.start_line = (g_main_win.lines - (g_keyboard_win.lines / 2) - 2);
    g_keyboard_win.start_col = content_x_offset + getContentXOffset(g_main_win.cols, g_keyboard_win.cols);

    WINDOW* header_window = newwin(g_header_win.lines, g_header_win.cols, g_header_win.start_line, g_header_win.start_col);
    wbkgd(header_window, COLOR_PAIR(1));
    box(header_window, 0, 0);
    wrefresh(header_window);

    WINDOW* footer_window = newwin(g_footer_win.lines, g_footer_win.cols, g_footer_win.start_line, g_footer_win.start_col);
    wbkgd(footer_window, COLOR_PAIR(1));
    box(footer_window, 0, 0);
    wrefresh(footer_window);

    WINDOW* main_window = newwin(g_main_win.lines, g_main_win.cols, g_main_win.start_line, g_main_win.start_col);
    wbkgd(main_window, COLOR_PAIR(1));
    box(main_window, 0, 0);
    wrefresh(main_window);

    WINDOW* type_window = newwin(g_type_win.lines, g_type_win.cols, g_type_win.start_line, g_type_win.start_col);
    wbkgd(type_window, COLOR_PAIR(1));
    box(type_window, 0, 0);
    wrefresh(type_window);

    WINDOW* keyboard_window = newwin(g_keyboard_win.lines, g_keyboard_win.cols, g_keyboard_win.start_line, g_keyboard_win.start_col);
    box(keyboard_window, 0, 0);
    draw_keyboard(keyboard_window, keyboard_size);
    wrefresh(keyboard_window);

    draw_header(header_window);
    draw_footer(footer_window);

    FILE* file = fopen(g_file, "r");
    if (file == NULL) {
        char error_msg[100];
        snprintf(error_msg, sizeof(error_msg), "Error opening file '%s'", g_file);
        perror(error_msg);
        return 1;
    }

    // read text file
    int target_buffer = 1;
    char* target = malloc(target_buffer * sizeof(char));
    int in_word = 0;
    int words_num = 0;
    int target_index = 0;
    int ch = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (!isSpace(ch) && isPrint(ch)) {
            if (!in_word) {
                in_word = 1;
                words_num++;
            }

            if (target_index >= target_buffer - 1) {
                target_buffer *= 2;
                target = realloc(target, target_buffer * sizeof(char));
                if (target == NULL) {
                    perror("Memory reallocation for 'target' failed\n");
                    free(target);
                    fclose(file);
                    return 1;
                }
            }
            target[target_index++] = (char)ch;
        } else {
            if (in_word) {
                target[target_index++] = ' ';
                in_word = 0;
            }
        }
    }
    if (target_index > 0 && isSpace(target[target_index - 1])) {
        target_index--;
    }
    target[target_index] = '\0';

    // file the words
    char words[words_num][MAX_WORD_CH];
    int word_index = 0;
    int word_ch_index = 0;

    for (int i = 0; i < (int)strlen(target); i++) {
        if (!isSpace(target[i])) {
            if (word_ch_index < MAX_WORD_CH - 1) {
                words[word_index][word_ch_index++] = target[i];
            }
        } else {
            words[word_index++][word_ch_index] = '\0';
            word_ch_index = 0;
        }
    }

    words[word_index][word_ch_index] = '\0';

restart:;
    // draw the words
    int cols = getAvailableCols(&g_main_win);
    curs_set(1);
    box(header_window, 0, 0);
    box(footer_window, 0, 0);
    draw_words(main_window, words, words_num, 0, cols);
    box(type_window, 0, 0);
    wrefresh(type_window);
    wrefresh(header_window);
    wrefresh(footer_window);

    char typed_word[MAX_WORD_CH] = { 0 };
    int word_pos = 0;
    int char_pos = 0;

    int start_cursor_x = 2;
    int start_cursor_y = g_type_win.lines / 2;

    // int cps = 0; // not used yet
    int cpm = 0;
    int wpm = 0;
    int is_typing = 0;
    double accuracy = 0.0;

    // int correct_flags[words_num][MAX_WORD_CH]; // not used yet
    int correct_chars = 0;
    int total_chars = 0;

    struct timeval t_start, t_current;
    long timer_sec = 0;

    wmove(type_window, start_cursor_y, start_cursor_x);
    wrefresh(type_window);
    keypad(type_window, 1);

    while (word_pos < words_num) {
        if (is_typing) {
            gettimeofday(&t_current, NULL);
            timer_sec = (t_current.tv_sec - t_start.tv_sec);

            double minutes = timer_sec / 60.0;

            if (minutes > 0) {
                wpm = (int)((word_pos / minutes));
                cpm = (int)(total_chars / minutes);
            }

            wmove(main_window, 0, g_main_win.padding);
            wprintw(main_window, "Timer: %3lu sec |\t WPM: %3d |\t CPM: %3d", timer_sec, wpm, cpm);
            wrefresh(main_window);
        }
        char ch = getch();

        if (ch == '\n')
            goto res;

        if (ch == 27) {
            break;
        }

        if (char_pos < (int)strlen(words[word_pos]) && isPrint(ch) && ch != ' ') {
            total_chars++;

            if (!is_typing) {
                gettimeofday(&t_start, NULL);
                is_typing = 1;
            }

            char expected_char = words[word_pos][char_pos];

            if (ch == expected_char) {
                wattron(type_window, COLOR_PAIR(2));
                correct_chars++;
            } else {
                wattron(type_window, COLOR_PAIR(3));
            }
            waddch(type_window, ch);
            wattroff(type_window, COLOR_PAIR(2));
            wattroff(type_window, COLOR_PAIR(3));
            wrefresh(type_window);
            typed_word[char_pos++] = ch;
        }

        if ((ch == 127 || ch == KEY_BACKSPACE || ch == '\b' || ch == 8) && char_pos > 0) {
            if (char_pos <= (int)strlen(words[word_pos])) {
                char_pos--;

                mvwaddch(type_window, start_cursor_y, start_cursor_x + char_pos, ' ');
                wmove(type_window, start_cursor_y, start_cursor_x + char_pos);
                wrefresh(type_window);

                typed_word[char_pos] = '\0';
            }
            continue;
        }

        if (ch == ' ' || word_pos == words_num - 1) {
            if (strcmp(typed_word, words[word_pos]) == 0) {
                for (int i = 0; i < char_pos; i++) {
                    mvwaddch(type_window, start_cursor_y, start_cursor_x + i, ' ');
                    wmove(type_window, start_cursor_y, start_cursor_x + i);
                    wrefresh(type_window);
                }

                wmove(type_window, start_cursor_y, start_cursor_x);
                wrefresh(type_window);

                char_pos = 0;
                word_pos++;
                draw_words(main_window, words, words_num, word_pos, cols);
                box(type_window, 0, 0);
                wrefresh(type_window);
                memset(typed_word, 0, sizeof(typed_word));
            }
        }

        highlight_key(keyboard_window, ch, keyboard_size);
        usleep(15500); // short delay
        draw_keyboard(keyboard_window, keyboard_size); // reset visual
    }

    // nodelay(stdscr, 0);

    gettimeofday(&t_current, NULL);
    timer_sec = t_current.tv_sec - t_start.tv_sec;
    double minutes = timer_sec / 60.0;

    if (minutes > 0) {
        wpm = (int)(word_pos / minutes);
        cpm = (int)(total_chars / minutes);
    }
    accuracy = total_chars > 0 ? (double)correct_chars / total_chars * 100.0 : 0.0;

    wclear(main_window);

#define BUTTON_COUNT 2
    char* buttons[BUTTON_COUNT] = { "Restart", "Quit" };
    int selected_button = 0;

    int menu_height = 10;
    int menu_width = 40;
    int menu_start_y = (LINES - menu_height) / 2;
    int menu_start_x = (COLS - menu_width) / 2;

    WINDOW* menu_win = newwin(menu_height, menu_width, menu_start_y, menu_start_x);
    box(menu_win, 0, 0);
    keypad(menu_win, true);

    char stats_title[] = "Typing Test Results";
    int title_x = getContentXOffset(menu_width, strlen(stats_title));

    char wpm_str[32];
    char cpm_str[32];
    char acc_str[32];
    char time_str[32];

    snprintf(wpm_str, sizeof(wpm_str), "WPM: %d", wpm);
    snprintf(cpm_str, sizeof(cpm_str), "CPM: %d", cpm);
    snprintf(acc_str, sizeof(acc_str), "Accuracy: %.1f%%", accuracy);
    snprintf(time_str, sizeof(time_str), "Time: %ld sec", timer_sec);

    // Main loop for the menu
    int choice;
    curs_set(0);
    while (1) {
        werase(menu_win);
        box(menu_win, 0, 0);

        mvwprintw(menu_win, 1, title_x, "%s", stats_title);

        mvwprintw(menu_win, 3, getContentXOffset(menu_width, strlen(wpm_str)), "%s", wpm_str);
        mvwprintw(menu_win, 4, getContentXOffset(menu_width, strlen(cpm_str)), "%s", cpm_str);
        mvwprintw(menu_win, 5, getContentXOffset(menu_width, strlen(acc_str)), "%s", acc_str);
        mvwprintw(menu_win, 6, getContentXOffset(menu_width, strlen(time_str)), "%s", time_str);

        for (int i = 0; i < BUTTON_COUNT; i++) {
            int btn_width = strlen(buttons[i]) + 4; // "[ Btn ]"
            int x = (menu_width / 2 - 10) + i * (btn_width + 4);
            if (i == selected_button) {
                wattron(menu_win, A_REVERSE);
            }
            mvwprintw(menu_win, 8, x, "[ %s ]", buttons[i]);
            wattroff(menu_win, A_REVERSE);
        }

        wrefresh(menu_win);

        choice = wgetch(menu_win);

        if (choice == KEY_LEFT) {
            selected_button = (selected_button - 1 + BUTTON_COUNT) % BUTTON_COUNT;
        } else if (choice == KEY_RIGHT) {
            selected_button = (selected_button + 1) % BUTTON_COUNT;
        } else if (choice == '\n' || ch == KEY_ENTER) {
            if (selected_button == 0) {
            res:
                wclear(stdscr);
                wclear(main_window);
                wclear(type_window);
                wrefresh(stdscr);
                wrefresh(main_window);
                wrefresh(type_window);
                curs_set(1);
                goto restart;
            } else if (selected_button == 1) {
                break;
            }
        }
    }

    delwin(menu_win);
    free(target);
    fclose(file);
    endwin();
    return 0;
}

int isSpace(const char ch)
{
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\v' || ch == '\f' || ch == '\r';
}

int isPrint(const char ch)
{
    return (ch >= 32 && ch <= 126);
}

int getAvailableCols(const g_Window* win)
{
    return (win->cols - (win->padding * 2));
}

int getContentXOffset(int cols, int content_width)
{
    return (cols - content_width) / 2;
}

int scale_to_1000(int value)
{
    return (value * 1000) / 255;
}

void draw_words(WINDOW* win, char words[][MAX_WORD_CH], int words_num, int current_word, int cols)
{
    wclear(win);
    box(win, 0, 0);

    wmove(win, g_main_win.padding, g_main_win.padding);
    wrefresh(win);

    int printed_chars = 0;
    int line_index = 0;
    // int word_index; // not used yet

    for (int i = 0; i < words_num; i++) {
        int free_spaces = cols - printed_chars;
        int word_len = strlen(words[i]); // word len + one space

        if (word_len > free_spaces) {
            line_index++;
            // word_index = 0;

            wmove(win, g_main_win.padding + line_index, g_main_win.padding);
            wrefresh(win);

            printed_chars = 0;
        }

        // lines[line_index][word_index] = words[i];

        for (int j = 0; j < (int)strlen(words[i]); j++) {
            if (i == current_word) {
                wattron(win, COLOR_PAIR(4));
            }

            waddch(win, words[i][j]);
            wattroff(win, COLOR_PAIR(4));

            wrefresh(win);

            printed_chars++;
        }

        if ((i + 1) < words_num) {
            waddch(win, ' ');
            wrefresh(win);

            printed_chars++;
        }

        // word_index++;
    }
}
