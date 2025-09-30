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
#include <filesystem>
namespace fs = std::filesystem; 

mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
int uniform(int l, int r) { return uniform_int_distribution<int>(l, r)(rng); }

const int N = 40; 
const int qnt_itens = 400; 
const int qnt_formigas = 50; 
const int raio_visao = 1; // tem que ser <= N
const int num_iteracoes = 500000; 
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

void pegar_ou_largar(int idx, bool after = 0) {
    int livres = 0; 
    int com_itens = 0; 

    formiga& f = formigas[idx]; 

    for (int ii = -raio_visao; ii <= raio_visao; ii++) {
        for (int jj = -raio_visao; jj <= raio_visao; jj++) {
            auto [ni, nj] = get_move(f.i + ii, f.j + jj); 
            livres++; 
            com_itens += grid[ni][nj]; 
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

struct RGB { int r,g,b; };

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
                
                // Verifica se tem formiga na posição
                bool tem_formiga = false;
                for (int idx = 0; idx < (int)formigas.size(); idx++) {
                    if (formigas[idx].i == i && formigas[idx].j == j) {
                        tem_formiga = true;
                        break;
                    }
                }
                
                if (tem_formiga) {
                    // Formiga = branco (sempre)
                    rr = 255; gg = 255; bb = 255;
                } else if (grid[i][j]) {
                    // Item = verde suave
                    rr = 34; gg = 139; bb = 34;  // Forest Green
                }
                // Caso contrário, fica preto (rr=0, gg=0, bb=0)
                
                for (int sx = 0; sx < SCALE; ++sx) {
                    out << rr << " " << gg << " " << bb << " ";
                }
            }
            out << "\n";
        }
    }
}


int main() {
    iniciar_grid(); 
    iniciar_formigas(); 

    auto grid_inicial = grid; 
    
    int last_frame_id = 0;
    int it = 0;
    
    // Primeira fase: iterações principais
    for (it = 0; it < num_iteracoes; it++) {
        int idx = it % qnt_formigas; 
        deslocar_formiga(idx); 
        pegar_ou_largar(idx); 

        if (it % num_iteracoes_print == 0) {
            int frame_id = it / num_iteracoes_print; 
            last_frame_id = frame_id; 
            save_ppm_frame(frame_id);
        } 
    } 

    // Segunda fase: até todas as formigas largarem os itens
    for (;;it++) {
        if (it == (int)2e9) break; 
        int idx = it % qnt_formigas; 
        bool ninguem_carregando = 1; 
        
        for (formiga f : formigas) {
            ninguem_carregando &= !f.carregando;
        } 

        if (ninguem_carregando) break; 
        
        deslocar_formiga(idx); 
        pegar_ou_largar(idx, 1); 

        if (it % num_iteracoes_print == 0) {
            int frame_id = it / num_iteracoes_print; 
            last_frame_id = frame_id; 
            save_ppm_frame(frame_id);
        } 
    }

    // Frame final sem formigas
    auto grid_final = grid;
    vector<formiga> temp_formigas = formigas;
    formigas.clear();
    save_ppm_frame(last_frame_id + 1);
    formigas = temp_formigas;
    
    int cnt = 0; 
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cnt += grid[i][j]; 
        } 
    } 
} 
