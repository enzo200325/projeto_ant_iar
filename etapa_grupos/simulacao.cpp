// to run: g++ -std=c++20 -lncurses simulacao.cpp && ./a.out

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
#include <fstream>
#include <sstream>

mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }
uniform_real_distribution<double> urd(0.0, 1.0); 

const int N = 40; 
const int qnt_itens = 400; 
const int qnt_formigas = 50; 
const int raio_visao = 1; // tem que ser <= N
const int num_iteracoes = 500000; 
const int num_iteracoes_print = 1000; 

const double alpha = 0; 
const double k1 = 0; 
const double k2 = 0; 

struct formiga {
    int i, j; 
    dado carregando; // se dado.active == 0 -> nao esta carregando
    formiga(int _i, int _j, dado _carregando) : i(_i), j(_j), carregando(_carregando) {} 

    formiga() {
        i = uniform(0, N - 1); 
        j = uniform(0, N - 1); 
        carregando = dado(); 
    } 
};  

struct dado {
    double x, double y; 
    bool active; 

    int actual_group; 

    dado() : x(0.0), y(0.0), active(false) { } 
    dado(double _x, double _y, bool _active) : x(_x), y(_y), active(_active) { } 
};  

vector<vector<dado>> grid; // se grid[i][j] == 1 -> tem item no 
vector<formiga> formigas; 

void iniciar_grid() {
    grid.assign(N, vector<dado>(N));
    // nao pode haver mais de um item no mesmo lugar
    vector<int> ord(N * N); 
    iota(ord.begin(), ord.end(), 0); 
    shuffle(ord.begin(), ord.end(), rng); 

    ifstream inputFile("dados.txt"); 
    string buf; 
    vector<tuple<double, double, int>> dados_input;
    while (getline(inputFile, buf)) {
        if (buf[0] == '#') continue; 

        stringstream to_buf(buf); 
        dado d; 

        double aa, bb; cin >> aa >> bb; 
        int cc; cin >> cc; 
        dados_input.emplace_back(aa, bb, cc); 
    } 
    shuffle(dados_input.begin(), dados_input.end(), rng); 

    for (int i = 0; i < qnt_itens; i++) {
        int num = ord[i]; 
        int x = num / N; 
        int y = num % N; 

        dado d; 
        d.x = get<0>(dados_input); 
        d.y = get<1>(dados_input); 
        dado.actual_group = get<2>(dados_input); 
        d.active = 1; 

        grid[x][y] = d; 
    } 
} 

void iniciar_formigas() {
    formigas.assign(qnt_formigas, formiga()); 
} 

pair<int, int> get_move(int i, int j) {
    i = (i + N) % N; 
    j = (j + N) % N; 
    return make_pair(i, j); 
} 

void deslocar_formiga(int idx) {
    vector<pair<int, int>> dir; 
    for (int ii = -1; ii <= 1; ii++) {
        for (int jj = -1; jj <= 1; jj++) {
            if (abs(ii) + abs(jj) == 1) {
                dir.emplace_back(ii, jj); 
            } 
        } 
    } 
    int d = uniform(0, (int)dir.size() - 1); 
    auto [ii, jj] = dir[d]; 
    formigas[idx].i += ii; 
    formigas[idx].j += jj; 
    tie(formigas[idx].i, formigas[idx].j) = get_move(formigas[idx].i, formigas[idx].j); 
} 

double get_dist(dado d0, dado d1) {
    double dx = d0.x - d1.x; 
    double dy = d0.y - d1.y; 
    return sqrt(dx * dx + dy * dy); 
} 

double get_similarity(int i, int j, dado d) {
    double s = 0; 
    double sum = 0; 
    for (int ii = -raio_visao; ii <= raio_visao; ii++) {
        for (int jj = -raio_visao; jj <= raio_visao; jj++) {
            auto [ni, nj] = get_move(f.i + ii, f.j + jj); 

            if (grid[ni][nj].active) {
                s++; 
                sum += (1 - get_dist(d, grid[ni][nj])) / alpha; 
            } 
        } 
    } 

    return sum / s / s; 
} 

double prop_pick(double similarity) {
    double val = (k1 / (k1 + similarity)); 
    return val * val; 
} 
double prob_drop(double similarity) {
    double val = (similarity / (k2 + similarity)); 
    return val * val; 
} 
double check_prob(double prob) {
    double res = urd(rng); 
    return res < prob; 
} 

void pegar_ou_largar(int idx, bool after = 0) {
    formiga& f = formigas[idx]; 

    if (!f.carregando.active && !after) { 
        // posso pegar
        if (check_prob(prob_pick(get_similarity(f.i, f.j, grid[f.i][f.j])))) {
            f.carregando = grid[f.i][f.j]; 
            grid[f.i][f.j].active = 0; 
        } 
    } 
    else {
        if (f.carregando.active && !grid[f.i][f.j].active) {
            if (check_prob(prob_drop(get_similarity(f.i, f.j, f.carregando)))) {
                grid[f.i][f.j] = f.carregando; 
                f.carregando.active = 0; 
            } 
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
                mvprintw(i, j * 2, "  ");
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
