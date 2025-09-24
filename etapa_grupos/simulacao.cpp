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

const int N = 64; 
const int qnt_itens = 400; 
const int qnt_formigas = 100; 
const int raio_visao = 1; // tem que ser <= N
const int num_iteracoes = 1e7; 
//const int num_iteracoes = 1; 
const int num_iteracoes_print = 10000; 

const double alpha = 11.8029; 
const double k1 = 0.3;
const double k2 = 0.6;

const string dados_file = "dados15.txt"; 
const bool normalize = 0; 

struct dado {
    double x, y; 
    bool active; 

    int actual_group; 

    dado() : x(0.0), y(0.0), active(false) { } 
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
    for (int i = 0; i < N; i++) for (int j = 0; j < N; j++) posicoes_livres.emplace_back(i, j); 
    // nao pode haver mais de um item no mesmo lugar
    vector<int> ord(N * N); 
    iota(ord.begin(), ord.end(), 0); 
    shuffle(ord.begin(), ord.end(), rng); 

    ifstream inputFile(dados_file); 
    string buf; 
    vector<tuple<double, double, int>> dados_input;
    while (getline(inputFile, buf)) {
        if (buf[0] == '#') continue; 

        for (char& c : buf) if (c == ',') c = '.'; 

        stringstream to_buf(buf); 
        dado d; 

        double aa, bb; to_buf >> aa >> bb; 
        int cc; to_buf >> cc; cc--; 

        dados_input.emplace_back(aa, bb, cc); 
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

        dado d; 
        d.x = get<0>(dados_input[i]); 
        d.y = get<1>(dados_input[i]); 
        d.actual_group = get<2>(dados_input[i]); 
        d.active = 1; 

        grid[x][y] = d; 
    } 
} 

void iniciar_formigas() {
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

    //cout << "sum: " << sum << " | s: " << s << endl;

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
        // posso pegar
        if (check_prob(prob_pick(get_similarity(f.i, f.j, grid[f.i][f.j])))) {
            f.carregando = grid[f.i][f.j]; 
            grid[f.i][f.j].active = 0; 
        } 
    } 
    else {
        if (f.carregando.active && !grid[f.i][f.j].active) {
            //cout << "what" << endl;
            //cout << "similarity: " << get_similarity(f.i, f.j, f.carregando) << endl;
            //cout << "prob_drop: " << prob_drop(get_similarity(f.i, f.j, f.carregando)) << endl;
            if (check_prob(prob_drop(get_similarity(f.i, f.j, f.carregando)))) {
                //cout << "damn" << endl;
                grid[f.i][f.j] = f.carregando; 
                f.carregando.active = 0; 
            } 
        } 
    } 
} 

vector<array<int, 3>> colors; 

void init_ncurses() {
    initscr();            // Inicia modo ncurses
    noecho();             // Não ecoar teclas
    curs_set(FALSE);      // Esconde cursor
    start_color();        // Habilita cores
                          
    init_pair(0, COLOR_WHITE, COLOR_BLACK); 
    // Para item ou formiga carregando item (maximo de itens 50); 
    int it = 1; 
    for (int i = 1; i < 16; i++) {
        init_pair(it++, i, COLOR_BLACK); 
    } 
    //init_pair(1, COLOR_GREEN, COLOR_BLACK);  
    //init_pair(2, COLOR_RED, COLOR_BLACK);  
    //init_pair(3, COLOR_YELLOW, COLOR_BLACK); 
    //init_pair(4, COLOR_BLUE, COLOR_BLACK); 
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

    } 


    for (int it = num_iteracoes;;it++) {
        if (it == (int)num_iteracoes + 1e6) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 
        for (formiga f : formigas) {
            //cout << "hm: " << f.carregando.active << endl;
            ninguem_carregando &= !f.carregando.active;
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

    int cnt = 0; 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cnt += grid[i][j].active; 
        } 
    } 
    getch();    // Espera tecla
    endwin();   // Sai do modo ncurses
    cout << "cnt: " << cnt << endl;

} 
