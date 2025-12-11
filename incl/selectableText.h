#pragma once

#include <stdbool.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define KEY_ENTER '\n'
#define KEY_UP (char)1001
#define KEY_DOWN (char)1002
#define KEY_RIGHT (char)1003
#define KEY_LEFT (char)1004

typedef struct Action {
    size_t start_pos;
    size_t end_pos;
} Action;

typedef struct Actions {
    Action* items;
    size_t count;
    size_t capacity;
} Actions;

typedef struct SelectableText {
    char* fmt_text;
    char* text;
    size_t count;
    Actions actions;
} SelectableText;

// helper function for reading raw CLI input
void read_action(char* out);

// helper function for adding an Action to an Actions instance
void actions_append(Action x, Actions* acts);

// helper function for deallocating an Actions instance
void actions_free(Actions* acts);

// function for loading a format string into a SelectableText instance
// this will define every member of the instance
bool selectableText_loadFmtString(char* fmt, SelectableText* txt);

// executes the prompt if the SelectableText instance is loaded
// returns the index of the action selected by the user
size_t selectableText_promptUser(SelectableText* txt);

// deallocates the SelectableText instance
void selectableText_free(SelectableText* txt);

void read_action(char* out) {
    // make a termios variable for the new terminal config
    // and one to save the old terminal config
    struct termios old_t, new_t;

    // get current terminal config and save the old one
    tcgetattr(STDIN_FILENO, &old_t);
    new_t = old_t;

    // setup new terminal config
    new_t.c_lflag &= ~(ICANON | ECHO); // raw mode
    tcsetattr(STDIN_FILENO, TCSANOW, &new_t); // apply new terminal config

    *out = getchar();

    // if key is escape sequence
    if (*out == 27) {
        char seq1 = getchar();
        char seq2 = getchar();

        if (seq1 == '[') {
            if (seq2 == 'A') *out = KEY_UP;
            else if (seq2 == 'B') *out = KEY_DOWN;
            else if (seq2 == 'C') *out = KEY_RIGHT;
            else if (seq2 == 'D') *out = KEY_LEFT;
        }
    } else if (*out == '\r' || *out == '\n') {
        *out = KEY_ENTER;
    }

    // restore original terminal config
    tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
}

void actions_append(Action x, Actions* acts) {
    if (acts->count >= acts->capacity) {
        size_t new_capacity = acts->capacity == 0 ? 1 : acts->capacity * 2;
        void *tmp = realloc(acts->items, new_capacity * sizeof(*acts->items));
        if (!tmp) {
            fprintf(stderr, "actions_append: memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        acts->items = tmp;
        acts->capacity = new_capacity; 
    }
    acts->items[acts->count++] = x;
}

void actions_free(Actions* acts) {
    free(acts->items);
    acts->items = NULL;
    acts->count = 0;
    acts->capacity = 0;
}

bool selectableText_loadFmtString(char *fmt, SelectableText *txt) {
    if (!fmt || !txt) return false;

    txt->actions.items = NULL;
    txt->actions.count = 0;
    txt->actions.capacity = 0;

    size_t n = strlen(fmt);

    size_t start_raw = (size_t)-1;
    size_t backticks_seen = 0;
    size_t plain_index = 0;

    // temp buffer for plain text (max length n plus 1 for null terminator)
    char *plain = malloc(n + 1);
    if (!plain) return false;

    for (size_t i = 0; i < n; i++) {
        char c = fmt[i];

        if (c == '`') {
            backticks_seen++;

            if (start_raw == (size_t)-1) {
                // opening delimiter
                start_raw = i;
            } else {
                // closing delimiter
                size_t end_raw = i;

                // convert raw delimiter indices to plain indices
                size_t start_plain = start_raw - (backticks_seen - 2);
                size_t end_plain = end_raw - (backticks_seen - 1) - 1;

                Action act = { start_plain, end_plain };
                actions_append(act, &txt->actions);

                start_raw = (size_t)-1;
            }

            continue; // don't copy backticks to the plain string
        }

        // copy non backticks to plain string
        plain[plain_index++] = c;
    }

    // check for unmatched backticks
    if (start_raw != (size_t)-1) {
        free(plain);
        actions_free(&txt->actions);
        return false;
    }

    // manually add a null terminator to the plain text
    plain[plain_index] = '\0';

    txt->fmt_text = strdup(fmt);
    
    // make sure strdup didn't fail
    if (!txt->fmt_text) {
        free(plain);
        actions_free(&txt->actions);
        return false;
    }
    
    txt->text = plain;
    txt->count = backticks_seen / 2;

    return true;
}

size_t selectableText_promptUser(SelectableText* txt) {
    if (!txt || !txt->fmt_text || !txt->text || !txt->count) {
        return (size_t)-1;
    }

    // ANSI escape codes
    char* start_reverse = "\033[7m"; // begin reversing bg and fg colors
    char* end_reverse = "\033[0m"; // end reversing bg and fg colors
    char* clear_screen = "\033[2J\033[H";  // clear screen and move cursor to top
   
    size_t action_index = 0;
    char c;
    
    do {
        // clear terminal
        printf("%s", clear_screen);
    
        // get current action
        Action act = txt->actions.items[action_index];
    
        // render the text with highlighting
        for (size_t i = 0; i < strlen(txt->text); i++) {
            if (i == act.start_pos) {
                printf("%s", start_reverse);
            }

            putchar(txt->text[i]);

            if (i == act.end_pos) {
                printf("%s", end_reverse);
            }
        }
    
        fflush(stdout);
    
        // read next character
        read_action(&c);
    
        // handle input
        if (c == KEY_UP || c == KEY_LEFT) {
            if (action_index > 0) action_index--;
            else action_index = txt->actions.count - 1;
        } else if (c == KEY_DOWN || c == KEY_RIGHT) {
            if (action_index < txt->actions.count - 1) action_index++;
            else action_index = 0;
        }
    } while (c != KEY_ENTER);

    printf("%s", end_reverse); // end reversing in case it's still active
    
    return action_index;
}

void selectableText_free(SelectableText *txt) {
    actions_free(&txt->actions);
    free(txt->fmt_text);
    free(txt->text);
    txt->fmt_text = NULL;
    txt->text = NULL;
    txt->count = 0;
}
