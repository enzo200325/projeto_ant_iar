// to run: 
// g++ -std=c++20 simulacao.cpp && ./a.out
// ffmpeg -framerate 20 -i frames/frame_%05d.ppm -vf scale=640:640 -c:v libx264 -pix_fmt yuv420p out.mp4
// open out.mp4 

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

#include <fstream>
#include <sstream>

// Configurações passadas via Makefile com -D flags
#ifndef DADOS_FILE
#define DADOS_FILE "dados_4_grupos.txt"
#endif

#ifndef EXP_DECAY
#define EXP_DECAY 0
#endif

#ifndef RAIO_VISAO_START
#define RAIO_VISAO_START 5
#endif

#ifndef RAIO_VISAO_END
#define RAIO_VISAO_END 2
#endif

#ifndef ALPHA_START
#define ALPHA_START 1.5
#endif

#ifndef ALPHA_END
#define ALPHA_END 1.5
#endif

#ifndef K1_START
#define K1_START 0.05
#endif

#ifndef K1_END
#define K1_END 0.05
#endif

#ifndef K2_START
#define K2_START 0.200
#endif

#ifndef K2_END
#define K2_END 0.200
#endif

#ifndef NORMALIZE
#define NORMALIZE 1
#endif

mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }
uniform_real_distribution<double> urd(0.0, 1.0); 

int N = 64; 
int qnt_itens = 400; 
int qnt_formigas = 150; 
int num_iteracoes = 1e7; 
int num_iteracoes_print = 100000;

const bool EXP = EXP_DECAY;
int raio_visao_start = RAIO_VISAO_START;
int raio_visao_end = RAIO_VISAO_END;
double alpha_start = ALPHA_START;
double alpha_end = ALPHA_END;
double k1_start = K1_START;
double k1_end = K1_END;
double k2_start = K2_START;
double k2_end = K2_END;

int raio_visao;
double alpha, k1, k2; 

bool normalize = NORMALIZE;
const string dados_file = DADOS_FILE; 

const double inf = 1e99; 

double schedule(double start, double end, int t, int T) {
    double frac = (double)t / T;
    return start + frac * (end - start);
}
double exp_decay(double start, double end, int t, int T) {
    double frac = (double)t / T;
    return start * pow(end/start, frac);
}

int schedule_int(int start, int end, int t, int T) {
    double frac = (double)t / T;
    double val = start + frac * (end - start);
    return (int)round(val);
}

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
    //double s = 0; 
    double sum = 0; 
    for (int ii = -raio_visao; ii <= raio_visao; ii++) {
        for (int jj = -raio_visao; jj <= raio_visao; jj++) {
            if (ii == 0 && jj == 0) continue; // nao me olho

            auto [ni, nj] = get_move(i + ii, j + jj); 

            if (grid[ni][nj].active) {
                //s++; 
                sum += max(0.0, 1 - (get_dist(d, grid[ni][nj]) / alpha)); 
            } 
        } 
    } 
    //if (s == 0) return 0; 
    
    double S = raio_visao * raio_visao; 
    return max(0.0, sum / (S * S)); 
} 

double prob_pick(double similarity) {
    double val = (k1 / (k1 + similarity)); 
    return val * val; 
} 
double prob_drop(double similarity) {
    //double val = (similarity / (k2 + similarity)); 
    //return val * val * val; 

    // machado: 
    if (similarity < k2) return 2*similarity; 
    else return 1; 
} 
double check_prob(double prob) {
    double res = urd(rng); 
    return res < prob; 
} 

void pegar_ou_largar(int idx, bool after = 0) {
    formiga& f = formigas[idx]; 

    if (!f.carregando.active && !after) { 
        if (grid[f.i][f.j].active && check_prob(prob_pick(get_similarity(f.i, f.j, grid[f.i][f.j])))) {
            f.carregando = grid[f.i][f.j]; 
            grid[f.i][f.j].active = 0; 
        } 
    } 
    else {
        if (f.carregando.active && !grid[f.i][f.j].active) {
            if (after && false) {
                //tem_formiga[f.i][f.j] = -1; 
                grid[f.i][f.j] = f.carregando; 
                f.carregando.active = 0; 
            } 
            else if (check_prob(prob_drop(get_similarity(f.i, f.j, f.carregando)))) {
                grid[f.i][f.j] = f.carregando; 
                f.carregando.active = 0; 
            } 
        } 
    } 
} 

void calc_parameters(int t) {
    if (EXP) {
        k1 = exp_decay(k1_start, k1_end, t, num_iteracoes);  // from 0.6 down to 0.2
        k2 = exp_decay(k2_start, k2_end, t, num_iteracoes); // from 0.05 down to 0.005
        alpha = exp_decay(alpha_start, alpha_end, t, num_iteracoes); // optional
    } 
    else {
        k1 = schedule(k1_start, k1_end, t, num_iteracoes);  // from 0.6 down to 0.2
        k2 = schedule(k2_start, k2_end, t, num_iteracoes); // from 0.05 down to 0.005
        alpha = schedule(alpha_start, alpha_end, t, num_iteracoes); // optional
    }
    raio_visao = schedule_int(raio_visao_start, raio_visao_end, t, num_iteracoes); 
} 

#include <filesystem> // C++17/20
namespace fs = std::filesystem;

struct RGB { int r,g,b; };

static const vector<RGB> PALETTE = {
    {255,0,0},    {0,255,0},    {0,0,255},    {255,255,0},
    {255,0,255},  {0,255,255},  {128,0,0},    {0,128,0},
    {0,0,128},    {128,128,0},  {128,0,128},  {0,128,128},
    {192,128,64}, {64,192,128}, {200,100,0},  {100,200,50}
};

int SCALE = 10; // pixels por célula

void save_ppm_frame(int frame_idx) {
    fs::create_directories("frames");
    char namebuf[256];
    snprintf(namebuf, sizeof(namebuf), "frames/frame_%05d.ppm", frame_idx);
    ofstream out(namebuf, ios::binary);
    if (!out) return;

    int W = N * SCALE;
    int H = N * SCALE;

    out << "P3\n" << W << " " << H << "\n255\n";

    for (int i = 0; i < N; ++i) {
        for (int sy = 0; sy < SCALE; ++sy) {  // repete linhas
            for (int j = 0; j < N; ++j) {
                int rr=0, gg=0, bb=0;
                if (tem_formiga[i][j] > -1) {
                    rr = 255; gg = 255; bb = 255;
                } else if (grid[i][j].active) {
                    int g = grid[i][j].actual_group;
                    int idx = (g >= 0) ? (g % (int)PALETTE.size()) : 0;
                    rr = PALETTE[idx].r;
                    gg = PALETTE[idx].g;
                    bb = PALETTE[idx].b;
                }
                for (int sx = 0; sx < SCALE; ++sx) {
                    out << rr << " " << gg << " " << bb << " ";
                }
            }
            out << "\n";
        }
    }
}


void run_on_constants_and_show() {
    iniciar_grid(); 
    iniciar_formigas(); 

    auto grid_inicial = grid; 

    int last_frame_id = 0; 
    int it = 0; 
    for (it; it < num_iteracoes; it++) {
        calc_parameters(it); 

        int idx = it % qnt_formigas; 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx); 

        if (it%num_iteracoes_print == 0) {
            int frame_id = it / num_iteracoes_print; 
            last_frame_id = frame_id; 
            save_ppm_frame(frame_id); 
            //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } 

    } 


    for (;;it++) {
        calc_parameters(num_iteracoes); 

        if (it == 3*(int)num_iteracoes) {
            break; 
        } 

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

        if (it%num_iteracoes_print == 0) {
            int frame_id = it / num_iteracoes_print; 
            last_frame_id = frame_id; 
            save_ppm_frame(frame_id); 
            //std::this_thread::sleep_for(std::chrono::milliseconds(5));
        } 
    }

    tem_formiga.assign(N, vector<int>(N, -1)); 
    save_ppm_frame(last_frame_id + 1); 

    map<int, int> mapa; 
    int tot = 0; 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j].active && tem_formiga[i][j] == -1) {
                mapa[grid[i][j].actual_group]++; 
                tot++; 
            } 
        } 
    } 
    cout << "tot: " << tot << endl;
    for (auto [col, F] : mapa) {
        cout << "col: " << col << " | F: " << F << endl;
    } 
    for (formiga f : formigas) {
        if (f.carregando.active) mapa[f.carregando.actual_group]++; 
    } 

    for (auto [col, F] : mapa) {
        //assert(F == 100); 
    } 
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
    //double best_alpha, best_k1, best_k2; 
    //double best_dist = inf; 
    //for (double al = 0.001; al <= 0.1; al += 0.001) {
    //    //for (double kk1 = 0.01; kk1 <= 0.1; kk1 += 0.01) {
    //        //for (double kk2 = 0.01; kk2 <= 0.1; kk2 += 0.01) {
    //            int kk1 = 
    //            auto grid_final = run(al, kk1, kk2, 1); 
    //            double cur_dist = check_max_dist(grid_final); 
    //            if (cur_dist < best_dist) {
    //                best_dist = cur_dist; 
    //                best_alpha = al; 
    //                best_k1 = kk1; 
    //                best_k2 = kk2; 
    //            } 
    //            cout << "best_dist: " << best_dist << endl;
    //            cout << "alpha: " << best_alpha << endl;
    //            cout << "k1: " << best_k1 << endl;
    //            cout << "k2: " << best_k2 << endl;
    //            cout << "---------------------------------" << endl;
    //        //} 
    //    //} 
    //} 

    run_on_constants_and_show(); 

    //cout << "best_dist: " << best_dist << endl;
    //cout << "alpha: " << best_alpha << endl;
    //cout << "k1: " << best_k1 << endl;
    //cout << "k2: " << best_k2 << endl;
} 

