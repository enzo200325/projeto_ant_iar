#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <climits> 
#include <thread>
using namespace std; 

#include <ncurses.h> 

mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }

const int N = 64; 
const int qnt_itens = 400; 
const int qnt_formigas = 100; 
const int raio_visao = 3; 
const int num_iteracoes = 10000000; 
const int num_iteracoes_print = 1000; 

struct formiga {
    int i, j; 
    bool carregando; 
    formiga(int _i, int _j, bool _carregando) : i(_i), j(_j), carregando(_carregando) {} 

    formiga() {
        i = uniform(0, N - 1); 
        j = uniform(0, N - 1); 
        carregando = 0; 
    } 
};  

vector<vector<bool>> grid; // se grid[i][j] == 1 -> tem item no 
vector<formiga> formigas; 

void iniciar_grid() {
    grid.assign(N, vector<bool>(N));
    // nao pode haver mais de um item no mesmo lugar
    vector<int> ord(N * N); 
    iota(ord.begin(), ord.end(), 0); 
    shuffle(ord.begin(), ord.end(), rng); 
    for (int i = 0; i < qnt_itens; i++) {
        int num = ord[i]; 
        int x = num / N; 
        int y = num % N; 
        grid[x][y] = 1; 
    } 
} 

void iniciar_formigas() {
    formigas.assign(qnt_formigas, formiga()); 
} 

bool verificar_bound(int i, int j) {
    return i >= 0 && j >= 0 && i < N && j < N; 
}

void deslocar_formiga(int idx) {
    vector<pair<int, int>> dir; 
    for (int ii = -1; ii <= 1; ii++) {
        for (int jj = -1; jj <= 1; jj++) {
            if (abs(ii) + abs(jj) == 1) {
                if (verificar_bound(formigas[idx].i + ii, formigas[idx].j + jj)) {
                    dir.emplace_back(ii, jj); 
                } 
            } 
        } 
    } 
    int d = uniform(0, (int)dir.size() - 1); 
    auto [ii, jj] = dir[d]; 
    formigas[idx].i += ii; 
    formigas[idx].j += jj; 
} 

void pegar_ou_largar(int idx, bool after = 0) {
    int livres = 0; 
    int com_itens = 0; 

    formiga& f = formigas[idx]; 

    for (int ii = -raio_visao; ii <= raio_visao; ii++) {
        for (int jj = -raio_visao; jj <= raio_visao; jj++) {
            if (verificar_bound(f.i + ii, f.j + jj)) {
                livres++; 
                com_itens += grid[f.i + ii][f.j + jj]; 
            } 
        } 
    } 

    // com_itens / livres 

    if (!f.carregando && !after) { 
        // posso pegar
        if (grid[f.i][f.j] && uniform(0, livres) > com_itens) { 
            // vou pegar
            grid[f.i][f.j] = 0; 
            f.carregando = 1; 
        } 
    } 
    else {
        bool ok = 1; 
        for (int rep = 0; rep < 2; rep++) {
            if (f.carregando && !grid[f.i][f.j] && uniform(0, livres) <= com_itens); 
            else ok = 0; 
        } 
        if (ok) {
            grid[f.i][f.j] = 1; 
            f.carregando = 0; 
        } 
    } 
} 


void init_ncurses() {
    initscr();            // Inicia modo ncurses
    noecho();             // Não ecoar teclas
    curs_set(FALSE);      // Esconde cursor
    start_color();        // Habilita cores
    init_pair(1, COLOR_WHITE, COLOR_BLACK);  // padrão
    init_pair(2, COLOR_GREEN, COLOR_BLACK);  // item
    init_pair(3, COLOR_RED, COLOR_BLACK);    // formiga sem item
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); // formiga carregando
}

// Mostra grid na tela
void draw_grid() {
    clear();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            bool temItem = grid[i][j];
            int idx_formiga = -1;
            int idx_formiga_carregando = -1; 
            for (int idx = 0; idx < (int)formigas.size(); idx++) {
                if (formigas[idx].i == i && formigas[idx].j == j) {
                    if (formigas[idx].carregando) idx_formiga_carregando = idx; 
                    idx_formiga = idx;
                }
            }

            // formiga carregando
            // formiga nao carregando 
            // iten

            if (idx_formiga_carregando != -1) {
                attron(COLOR_PAIR(4)); 
                mvprintw(i, j * 2, "F ");
                attroff(COLOR_PAIR(4));
            } 
            else if (idx_formiga != -1) {
                attron(COLOR_PAIR(3));
                mvprintw(i, j * 2, "F ");
                attroff(COLOR_PAIR(3));
            } else if (temItem) {
                attron(COLOR_PAIR(2));
                mvprintw(i, j * 2, "o ");
                attroff(COLOR_PAIR(2));
            } else {
                attron(COLOR_PAIR(1));
                mvprintw(i, j * 2, ". ");
                attroff(COLOR_PAIR(1));
            }
        }
    }
    refresh();
}

int main() {
    init_ncurses(); 


    iniciar_grid(); 
    iniciar_formigas(); 

    auto grid_inicial = grid; 
    for (int it = 0; it < num_iteracoes; it++) {
        int idx = it % qnt_formigas; 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx); 

        if (it%num_iteracoes_print == 0) {
            draw_grid();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // pausa 300ms
        } 

        //cout << "what" << endl;

    } 


    for (int it = num_iteracoes;;it++) {
        if (it == (int)2e9) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 
        for (formiga f : formigas) {
            ninguem_carregando &= !f.carregando;
        } 

        if (ninguem_carregando) break; 
        else {
            deslocar_formiga(idx); 
            pegar_ou_largar(idx, 1); 
        } 

            if (it%num_iteracoes_print == 0) {
                draw_grid();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // pausa 300ms
            } 
    }

    auto grid_final = grid; 
    vector<formiga> vv; 

    grid = grid_inicial; 
    formigas = vv; 
    draw_grid(); 
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // pausa 300ms
                                                                  //
    grid = grid_final; 
    draw_grid(); 
    //std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // pausa 300ms
                                                                  //

    int cnt = 0; 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cnt += grid[i][j]; 
        } 
    } 
    getch();    // Espera tecla
    endwin();   // Sai do modo ncurses
    cout << "cnt: " << cnt << endl;

} 
