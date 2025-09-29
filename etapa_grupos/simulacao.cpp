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

int N = 64; 
int qnt_itens = 400; 
int qnt_formigas = 100; 
int raio_visao = 1; // tem que ser <= N
int num_iteracoes = 2e6; 
int num_iteracoes_print = 10000; 

double alpha = 0.30; 
double k1 = 0.5;
double k2 = 0.025; 

bool normalize = 1;

const string dados_file = "dados2.txt"; 

const double inf = 1e99; 

struct dado {
    double x, y; 
    bool active; 

    int actual_group; 

    dado() : x(2), y(0.0), active(false) { } 
    dado(double _x, double _y, bool _active) : x(_x), y(_y), active(_active) { } 
};  

vector<vector<dado>> grid; 
vector<vector<int>> tem_formiga; 
vector<pair<int, int>> posicoes_livres; 

struct formiga {
    int i, j, id; 
    dado carregando; // se dado.active == 0 -> nao esta carregando
    formiga(int _i, int _j, int _id, dado _carregando) : i(_i), j(_j), id(_id), carregando(_carregando) {} 

    formiga(int _id = -1) {
        int idx = uniform(0, (int)posicoes_livres.size() - 1); 
        tie(i, j) = posicoes_livres[idx]; 

        // deleta posicao idx de posicoes_livres
        swap(posicoes_livres[idx], posicoes_livres.back()); 
        posicoes_livres.pop_back(); 

        carregando = dado(); 
        id = _id; 
        tem_formiga[i][j] = id; 
    } 
};  

vector<formiga> formigas; 

void iniciar_grid() {
    grid.assign(N, vector<dado>(N));
    tem_formiga.assign(N, vector<int>(N, -1)); 
    posicoes_livres.clear(); 
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) posicoes_livres.emplace_back(i, j); 
    // nao pode haver mais de um item no mesmo lugar
    vector<int> ord(N * N); 
    iota(ord.begin(), ord.end(), 0); 
    shuffle(ord.begin(), ord.end(), rng); 

    ifstream inputFile(dados_file); 
    string buf; 
    vector<tuple<double, double, int>> dados_input;
    int it = 0; 
    while (getline(inputFile, buf)) {
        if (buf[0] == '#') continue; 
        if ((int)buf.size() <= 1) continue; // linha vazia 

        for (char& c : buf) if (c == ',') c = '.'; 

        stringstream to_buf(buf); 
        dado d; 

        double aa, bb; to_buf >> aa >> bb; 
        int cc; to_buf >> cc; cc--; 
        dados_input.emplace_back(aa, bb, cc); 
        it++; 
    } 
    shuffle(dados_input.begin(), dados_input.end(), rng); 

    if (normalize) {
        double max_aa = -1e99, max_bb = -1e99; 
        for (auto [aa, bb, cc] : dados_input) {
            max_aa = max(max_aa, abs(aa)); 
            max_bb = max(max_bb, abs(bb)); 
        } 
        for (auto& [aa, bb, cc] : dados_input) {
            aa /= max_aa; 
            bb /= max_bb; 
        } 
    } 

    for (int i = 0; i < dados_input.size(); i++) {
        int num = ord[i]; 
        int x = num / N; 
        int y = num % N; 

        auto [aa, bb, cc] = dados_input[i]; 

        dado d; 
        d.x = get<0>(dados_input[i]); 
        d.y = get<1>(dados_input[i]); 
        d.actual_group = get<2>(dados_input[i]); 
        d.active = 1; 

        grid[x][y] = d; 
    } 
} 

void iniciar_formigas() {
    formigas.clear(); 
    for (int i = 0; i < qnt_formigas; i++) formigas.emplace_back(formiga(i)); 
} 

pair<int, int> get_move(int i, int j) {
    i = (i + N) % N; 
    j = (j + N) % N; 
    return make_pair(i, j); 
} 

void deslocar_formiga(int idx) {
    formiga& f = formigas[idx]; 
    vector<pair<int, int>> dir = {make_pair(0, 0)}; // pode ficar onde esta
    for (int ii = -1; ii <= 1; ii++) {
        for (int jj = -1; jj <= 1; jj++) {
            if (abs(ii) + abs(jj) == 1) { 
                auto [ni, nj] = get_move(f.i + ii, f.j + jj); 
                if (tem_formiga[ni][nj] == -1) dir.emplace_back(ii, jj); 
            } 
        } 
    } 

    int d = uniform(0, (int)dir.size() - 1); 
    auto [ii, jj] = dir[d]; 

    tem_formiga[f.i][f.j] = -1; 
    tie(f.i, f.j) = get_move(f.i + ii, f.j + jj); 
    tem_formiga[f.i][f.j] = f.id; 
} 

double get_dist(dado d0, dado d1) {
    double dx = d0.x - d1.x; 
    double dy = d0.y - d1.y; 

    return sqrt(dx * dx + dy * dy); 
} 

double get_similarity(int i, int j, dado d) {
    double s = 1; 
    double sum = 0; 
    for (int ii = -raio_visao; ii <= raio_visao; ii++) {
        for (int jj = -raio_visao; jj <= raio_visao; jj++) {
            if (ii == 0 && jj == 0) continue; // nao me olho

            auto [ni, nj] = get_move(i + ii, j + jj); 

            if (grid[ni][nj].active) {
                s++; 
                sum += 1 - (get_dist(d, grid[ni][nj]) / alpha); 
            } 
        } 
    } 

    return max(0.0, sum / (s * s)); 
} 

double prob_pick(double similarity) {
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
        if (grid[f.i][f.j].active && check_prob(prob_pick(get_similarity(f.i, f.j, grid[f.i][f.j])))) {
            if (abs(grid[f.i][f.j].x) <= 1e-100) {
                //assert(false); 
                //cout << "what: " << grid[f.i][f.j].x << endl;
            } 
            f.carregando = grid[f.i][f.j]; 
            grid[f.i][f.j].active = 0; 
        } 
    } 
    else {
        if (f.carregando.active && !grid[f.i][f.j].active) {
            if (check_prob(prob_drop(get_similarity(f.i, f.j, f.carregando)))) {
                grid[f.i][f.j] = f.carregando; 
                f.carregando.active = 0; 

                if (after) tem_formiga[f.i][f.j] = -1; 
            } 
        } 
    } 
} 

void init_ncurses() {
    initscr();            // Inicia modo ncurses
    noecho();             // Não ecoar teclas
    curs_set(FALSE);      // Esconde cursor
    start_color();        // Habilita cores
                          
    init_pair(0, COLOR_WHITE, COLOR_BLACK); 
    // Para item ou formiga carregando item  
    int it = 1; 
    for (int i = 1; i < 16; i++) {
        init_pair(it++, i, COLOR_BLACK); 
    } 
}

// Mostra grid na tela
void draw_grid() {
    // Se posicao no grid tem formiga e item ao mesmo tempo, preferencia é dada à formiga 
    clear();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (tem_formiga[i][j] > -1) {
                formiga f = formigas[tem_formiga[i][j]]; 
                int color = f.carregando.active ? f.carregando.actual_group : 0; 
                attron(COLOR_PAIR(color)); 
                mvprintw(i, j * 2, "F ");
                attroff(COLOR_PAIR(color));
            } else if (grid[i][j].active) {
                int color = grid[i][j].actual_group; 
                attron(COLOR_PAIR(color));
                mvprintw(i, j * 2, "o ");
                attroff(COLOR_PAIR(color));
            } 
        }
    }
    refresh();
}

void run_on_constants_and_show() {
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

    } 

    for (int it = num_iteracoes;;it++) {
        if (it == (int)num_iteracoes + 1e6) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 

        for (formiga f : formigas) {
            if (f.carregando.active) {
                ninguem_carregando = 0; 
                deslocar_formiga(f.id); 
                pegar_ou_largar(f.id, 1); 
            } 
        } 

        if (it%num_iteracoes_print == 0) {
            draw_grid();
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // pausa 300ms
        } 
        if (ninguem_carregando) break; 
    }

    auto grid_final = grid; 
    vector<formiga> vv; 

    vector<formiga> neutral; 

    grid = grid_inicial; 
    tem_formiga.assign(N, vector<int>(N, -1)); 
    draw_grid(); 
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // pausa 300ms
                                                                  //
    grid = grid_final; 
    tem_formiga.assign(N, vector<int>(N, -1)); 
    draw_grid(); 
    //std::this_thread::sleep_for(std::chrono::milliseconds(5000)); // pausa 300ms
                                                                  //

    getch();    // Espera tecla
    endwin();   // Sai do modo ncurses
} 

void run_on_constants() {
    iniciar_grid(); 
    iniciar_formigas(); 

    auto grid_inicial = grid; 
    for (int it = 0; it < num_iteracoes; it++) {
        int idx = it % qnt_formigas; 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx); 
    } 


    for (int it = num_iteracoes;;it++) {
        if (it == (int)num_iteracoes * 2) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 

        int cnt = 0; 
        for (formiga f : formigas) {
            if (f.carregando.active) {
                ninguem_carregando = 0; 
                cnt++; 
            } 
        } 


        if (ninguem_carregando) {
            break; 
        } 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx, 1); 
    }

    auto grid_final = grid; 
} 

vector<vector<dado>> run(double _alpha, double _k1, double _k2, bool _normalize) {
    double back_alpha = alpha; 
    double back_k1 = k1; 
    double back_k2 = k2; 
    alpha = _alpha; 
    k1 = _k1; 
    k2 = _k2; 

    iniciar_grid(); 
    iniciar_formigas(); 

    auto grid_inicial = grid; 
    for (int it = 0; it < num_iteracoes; it++) {
        int idx = it % qnt_formigas; 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx); 
    } 


    for (int it = num_iteracoes;;it++) {
        if (it == num_iteracoes * 2) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 

        for (formiga f : formigas) {
            if (f.carregando.active) {
                ninguem_carregando = 0; 
            } 
        } 
        if (ninguem_carregando) break; 

        deslocar_formiga(idx); 
        pegar_ou_largar(idx, 1); 
    }

    auto grid_final = grid; 
    alpha = back_alpha; 
    k1 = back_k1; 
    k2 = back_k2; 
    return grid_final; 
} 

double check_max_dist(vector<vector<dado>> grid) {
    map<int, vector<pair<int, int>>> mapa; 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j].active) {
                mapa[grid[i][j].actual_group].emplace_back(i, j); 
            } 
        } 
    } 

    double mx_dist = -inf;
    for (auto [c, V] : mapa) {
        for (auto [x1, y1] : V) {
            for (auto [x2, y2] : V) {
                double dx = x1 - x2; 
                double dy = y1 - y2; 
                mx_dist = max(mx_dist, sqrt(dx * dx + dy * dy)); 
            } 
        } 
    } 

    return mx_dist; 
} 

int main() {
    //run_on_constants(); 
//vector<vector<dado>> run(double _alpha, double _k1, double _k2, bool _normalize) {
    double best_alpha, best_k1, best_k2; 
    double best_dist = inf; 
    for (double al = 0.001; al <= 0.1; al += 0.001) {
        for (double kk1 = 0.01; kk1 <= 0.1; kk1 += 0.01) {
            for (double kk2 = 0.01; kk2 <= 0.1; kk2 += 0.01) {
                auto grid_final = run(al, kk1, kk2, 1); 
                double cur_dist = check_max_dist(grid_final); 
                if (cur_dist < best_dist) {
                    best_dist = cur_dist; 
                    best_alpha = al; 
                    best_k1 = kk1; 
                    best_k2 = kk2; 
                } 
            } 
        } 
        cout << "best_dist: " << best_dist << endl;
        cout << "alpha: " << best_alpha << endl;
        cout << "k1: " << best_k1 << endl;
        cout << "k2: " << best_k2 << endl;
        cout << "---------------------------------" << endl;
    } 

    //cout << "best_dist: " << best_dist << endl;
    //cout << "alpha: " << best_alpha << endl;
    //cout << "k1: " << best_k1 << endl;
    //cout << "k2: " << best_k2 << endl;
} 

