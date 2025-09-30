# Simulação de Colônia de Formigas

## 📋 Descrição

Simulação de clustering usando algoritmo de formigas para agrupar dados em diferentes grupos de cores.

## 🚀 Como usar

### Comandos principais

```bash
# Simular 4 grupos (dados_4_grupos.txt)
make 4grupos

# Simular 15 grupos (dados_15_grupos.txt)
make 15grupos

# Abrir vídeos gerados
make open-4    # Abre out_4grupos.mp4
make open-15   # Abre out_15grupos.mp4

# Limpar arquivos
make clean     # Remove executáveis e vídeos
make clean-all # Remove tudo incluindo frames

# Ver ajuda
make help
```

## ⚙️ Configurações

As configurações são definidas no **Makefile** e passadas como flags de compilação:

### Configuração 4 Grupos (dados_4_grupos.txt)
- **Arquivo**: `dados_4_grupos.txt`
- **Raio visão**: 5 → 2
- **Alpha**: 1.5 → 1.5 (constante)
- **K1**: 0.05 → 0.05 (constante)
- **K2**: 0.200 → 0.200 (constante)
- **Decay exponencial**: Não

### Configuração 15 Grupos (dados_15_grupos.txt)
- **Arquivo**: `dados_15_grupos.txt`
- **Raio visão**: 5 → 2
- **Alpha**: 0.1 → 0.005 (decai)
- **K1**: 0.010 → 0.00050 (decai)
- **K2**: 0.030 → 0.00010 (decai)
- **Decay exponencial**: Não

## 📝 Modificando configurações

Para alterar os parâmetros, edite o **Makefile** nas seções:
- `PARAMS_4GRUPOS` - Configurações para 4 grupos
- `PARAMS_15GRUPOS` - Configurações para 15 grupos

Exemplo:
```makefile
PARAMS_4GRUPOS = \
    -DDADOS_FILE='"dados_4_grupos.txt"' \
    -DALPHA_START=2.0 \           # Modificar aqui
    -DK1_START=0.1 \               # Modificar aqui
    ...
```

## 📦 Arquivos gerados

- `out_4grupos.mp4` - Vídeo da simulação de 4 grupos
- `out_15grupos.mp4` - Vídeo da simulação de 15 grupos
- `frames/` - Diretório com frames PPM da simulação
- `simulacao` - Executável compilado

## 🔧 Requisitos

- `g++` com suporte a C++20
- `ffmpeg` para geração de vídeos
- Sistema macOS (para comando `open`)

## 📊 Estrutura do código

- **Grid**: Matriz NxN com os dados
- **Formigas**: Agentes que movem e organizam os dados
- **Similaridade**: Calculada pela distância euclidiana
- **Probabilidades**: 
  - `prob_pick`: Probabilidade de pegar item
  - `prob_drop`: Probabilidade de largar item
