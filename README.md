# Simulação de Colônia de Formigas

Simulação baseada em colônia de formigas para agrupamento de itens.

## Requisitos

- C++ com suporte a C++20
- `g++` compilador
- `ffmpeg` para geração de vídeos

### Instalar ffmpeg (macOS)
```bash
brew install ffmpeg
```

## Como usar

### Opção 1: Workflow completo (recomendado)
Execute tudo de uma vez (compilação + simulação + geração de vídeo):
```bash
make run
```

### Opção 2: Executar etapas separadamente

1. **Apenas compilar:**
```bash
make compile
```

2. **Compilar e executar simulação:**
```bash
make simulate
```

3. **Gerar vídeo a partir dos frames:**
```bash
make video
```

4. **Abrir vídeo gerado:**
```bash
make open
```

### Limpeza

- **Limpar executáveis e vídeos:**
```bash
make clean
```

- **Limpar tudo (incluindo frames):**
```bash
make clean-all
```

### Ver todos os comandos disponíveis
```bash
make help
```

## Saída

- **Frames:** Salvos em `frames/frame_XXXXX.ppm`
- **Vídeo:** `out.mp4`

## Cores no vídeo

- 🟢 **Verde suave (Forest Green):** Itens no grid
- ⚪ **Branco:** Formigas (carregando ou não)
- ⚫ **Preto:** Espaço vazio

## Parâmetros da simulação

Você pode ajustar os parâmetros no arquivo `simulacao.cpp`:

- `N`: Tamanho do grid (padrão: 40x40)
- `qnt_itens`: Quantidade de itens (padrão: 400)
- `qnt_formigas`: Quantidade de formigas (padrão: 50)
- `raio_visao`: Raio de visão das formigas (padrão: 1)
- `num_iteracoes`: Número de iterações (padrão: 500000)
- `num_iteracoes_print`: Intervalo para gerar frames (padrão: 1000)
