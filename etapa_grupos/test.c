#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

void show_standard_colors();
void show_color_cube();
void show_grayscale();

int main() {
    initscr();
    if (!has_colors()) {
        endwin();
        printf("Seu terminal não suporta cores.\n");
        return 1;
    }
    start_color();

    // Verifique se o terminal suporta 256 cores
    if (COLORS < 256) {
        endwin();
        printf("Seu terminal suporta apenas %d cores. Este programa requer 256 cores.\n", COLORS);
        return 1;
    }

    // Exibe as 16 cores padrão
    show_standard_colors();

    // Exibe a grade de 6x6x6
    show_color_cube();

    // Exibe os tons de cinza
    show_grayscale();

    getch();
    endwin();
    return 0;
}

void show_standard_colors() {
    int i, fg, bg;
    
    // Inicia os pares de cores com fundo preto (cor 0)
    for (i = 0; i < 16; ++i) {
        init_pair(i + 1, i, 0);
    }

    printw("16 Cores Padrao:\n\n");

    // Imprime as cores básicas (0-7)
    for (i = 0; i < 8; ++i) {
        attron(COLOR_PAIR(i + 1));
        printw("  %3d  ", i);
        attroff(COLOR_PAIR(i + 1));
    }
    printw("\n\n");

    // Imprime as cores brilhantes (8-15)
    for (i = 8; i < 16; ++i) {
        attron(COLOR_PAIR(i + 1));
        printw("  %3d  ", i);
        attroff(COLOR_PAIR(i + 1));
    }
    printw("\n\n");

    printw("Pressione qualquer tecla para continuar...\n");
    refresh();
    getch();
    clear();
}

void show_color_cube() {
    int i, r, g, b, fg, pair;
    
    printw("Grade de Cores 6x6x6 (216 cores):\n\n");
    
    pair = 17; // Inicia os pares de cores a partir do 17
    
    for (g = 0; g < 6; ++g) {
        for (r = 0; r < 6; ++r) {
            for (b = 0; b < 6; ++b) {
                fg = 16 + (r * 36) + (g * 6) + b;
                init_pair(pair, fg, 0);
                attron(COLOR_PAIR(pair));
                printw("  ");
                attroff(COLOR_PAIR(pair));
                pair++;
            }
            printw(" "); // Espaço entre as linhas do cubo
        }
        printw("\n");
    }
    printw("\n");

    printw("Pressione qualquer tecla para continuar...\n");
    refresh();
    getch();
    clear();
}

void show_grayscale() {
    int i, fg, pair;
    
    printw("Tons de Cinza (24 cores):\n\n");
    
    pair = 233; // Inicia os pares de cores a partir do 233
    
    for (i = 0; i < 24; ++i) {
        fg = 232 + i;
        init_pair(pair, fg, 0);
        attron(COLOR_PAIR(pair));
        printw("  ");
        attroff(COLOR_PAIR(pair));
        pair++;
    }
    printw("\n\n");

    printw("Pressione qualquer tecla para sair.\n");
    refresh();
    getch();
}
