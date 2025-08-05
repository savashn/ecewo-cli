#include "cli.h"

// Clear the console screen
void clear_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
}

// Draw menu for selecting plugin
void draw_menu(int current)
{
    clear_screen();
    printf("Select plugins: (space to toggle, arrow keys to move, enter to confirm)\n\n");
    for (int i = 0; i < plugin_count; i++)
    {
        printf(" %c [%c] %s\n",
               i == current ? '>' : ' ',
               plugins[i].selected ? 'x' : ' ',
               plugins[i].name);
    }
}

// Read the keyboard
int read_key(void)
{
#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 224)
    {
        int dir = _getch();
        return 1000 + dir; // Add offset
    }
    return ch;
#else
    char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
        return -1;
    if (c == '\x1b')
    { // ESC
        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
            return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return '\x1b';

        if (seq[0] == '[')
        {
            if (seq[1] == 'A')
                return 1000 + 72; // UP
            if (seq[1] == 'B')
                return 1000 + 80; // DOWN
            if (seq[1] == 'C')
                return 1000 + 77; // RIGHT
            if (seq[1] == 'D')
                return 1000 + 75; // LEFT
        }

        return '\x1b';
    }
    return c;
#endif
}

// Select plugin in the creating project step
void interactive_select(void)
{
    int current = 0;
#ifndef _WIN32
    enable_raw_mode();
#endif

    while (1)
    {
        draw_menu(current);
        int c = read_key();

        if (c == ' ')
        {
            plugins[current].selected = !plugins[current].selected;
        }
        else if (c == '\r' || c == '\n')
        {
            break;
        }
        else if (c == 'w' && current > 0)
        {
            current--;
        }
        else if (c == 's' && current < plugin_count - 1)
        {
            current++;
        }
        else if (c == 1000 + 72 && current > 0) // UP
        {
            current--;
        }
        else if (c == 1000 + 80 && current < plugin_count - 1) // DOWN
        {
            current++;
        }
    }

#ifndef _WIN32
    disable_raw_mode();
#endif
    printf("\n");
}
