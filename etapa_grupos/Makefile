# Makefile for Ant Colony Simulation
# Usage:
#   make 4grupos   - Run simulation for 4 groups (dados_4_grupos.txt)
#   make 15grupos  - Run simulation for 15 groups (dados_15_grupos.txt)
#   make clean     - Clean all generated files

CXX = g++
CXXFLAGS = -std=c++20
SOURCE = simulacao.cpp
EXECUTABLE = simulacao

# ============================================
# CONFIGURAÇÕES PARA 4 GRUPOS (dados_4_grupos.txt)
# ============================================
PARAMS_4GRUPOS = \
	-DDADOS_FILE='"dados_4_grupos.txt"' \
	-DEXP_DECAY=0 \
	-DRAIO_VISAO_START=5 \
	-DRAIO_VISAO_END=2 \
	-DALPHA_START=1.5 \
	-DALPHA_END=1.5 \
	-DK1_START=0.05 \
	-DK1_END=0.05 \
	-DK2_START=0.200 \
	-DK2_END=0.200 \
	-DNORMALIZE=1

# ============================================
# CONFIGURAÇÕES PARA 15 GRUPOS (dados_15_grupos.txt)
# ============================================
PARAMS_15GRUPOS = \
	-DDADOS_FILE='"dados_15_grupos.txt"' \
	-DEXP_DECAY=0 \
	-DRAIO_VISAO_START=5 \
	-DRAIO_VISAO_END=2 \
	-DALPHA_START=0.1 \
	-DALPHA_END=0.005 \
	-DK1_START=0.010 \
	-DK1_END=0.00050 \
	-DK2_START=0.030 \
	-DK2_END=0.00010 \
	-DNORMALIZE=1

.PHONY: all 4grupos 15grupos clean clean-frames clean-all open-4 open-15

all: 4grupos

# ============================================
# Target for 4 groups simulation
# ============================================
4grupos: clean-frames
	@echo "========================================="
	@echo "   SIMULAÇÃO PARA 4 GRUPOS (dados_4_grupos.txt)"
	@echo "========================================="
	@echo "Compilando com as seguintes configurações:"
	@echo "  - Arquivo de dados: dados_4_grupos.txt"
	@echo "  - Raio visão: 5 -> 2"
	@echo "  - Alpha: 1.5 -> 1.5"
	@echo "  - K1: 0.05 -> 0.05"
	@echo "  - K2: 0.200 -> 0.200"
	@echo ""
	@$(CXX) $(CXXFLAGS) $(PARAMS_4GRUPOS) $(SOURCE) -o $(EXECUTABLE)
	@echo "Executando simulação..."
	@./$(EXECUTABLE)
	@echo ""
	@echo "Gerando vídeo out_4grupos.mp4..."
	@$(MAKE) video OUTPUT=out_4grupos.mp4
	@echo ""
	@echo "========================================="
	@echo "✅ CONCLUÍDO! Video: out_4grupos.mp4"
	@echo "========================================="

# ============================================
# Target for 15 groups simulation  
# ============================================
15grupos: clean-frames
	@echo "========================================="
	@echo "  SIMULAÇÃO PARA 15 GRUPOS (dados_15_grupos.txt)"
	@echo "========================================="
	@echo "Compilando com as seguintes configurações:"
	@echo "  - Arquivo de dados: dados_15_grupos.txt"
	@echo "  - Raio visão: 5 -> 2"
	@echo "  - Alpha: 0.1 -> 0.005"
	@echo "  - K1: 0.010 -> 0.00050"
	@echo "  - K2: 0.030 -> 0.00010"
	@echo ""
	@$(CXX) $(CXXFLAGS) $(PARAMS_15GRUPOS) $(SOURCE) -o $(EXECUTABLE)
	@echo "Executando simulação..."
	@./$(EXECUTABLE)
	@echo ""
	@echo "Gerando vídeo out_15grupos.mp4..."
	@$(MAKE) video OUTPUT=out_15grupos.mp4
	@echo ""
	@echo "========================================="
	@echo "✅ CONCLUÍDO! Video: out_15grupos.mp4"
	@echo "========================================="

# ============================================
# Generate video from frames
# ============================================
video:
	@ffmpeg -y -framerate 20 -i frames/frame_%05d.ppm -vf scale=640:640 -c:v libx264 -pix_fmt yuv420p $(OUTPUT) 2>/dev/null || \
	(echo "❌ Erro ao gerar vídeo. Verifique se o ffmpeg está instalado." && exit 1)

# ============================================
# Open videos (macOS)
# ============================================
open-4:
	@if [ -f out_4grupos.mp4 ]; then \
		open out_4grupos.mp4; \
	else \
		echo "❌ Arquivo out_4grupos.mp4 não encontrado. Execute 'make 4grupos' primeiro."; \
	fi

open-15:
	@if [ -f out_15grupos.mp4 ]; then \
		open out_15grupos.mp4; \
	else \
		echo "❌ Arquivo out_15grupos.mp4 não encontrado. Execute 'make 15grupos' primeiro."; \
	fi

# ============================================
# Clean targets
# ============================================
clean-frames:
	@rm -rf frames/
	@mkdir -p frames/

clean:
	@rm -f $(EXECUTABLE) *.tmp
	@rm -f out_4grupos.mp4 out_15grupos.mp4
	@echo "🧹 Arquivos limpos!"

clean-all: clean clean-frames
	@echo "🧹 Todos os arquivos gerados foram removidos!"

# ============================================
# Help
# ============================================
help:
	@echo "Comandos disponíveis:"
	@echo "  make 4grupos   - Executa simulação para 4 grupos"
	@echo "  make 15grupos  - Executa simulação para 15 grupos"
	@echo "  make open-4    - Abre vídeo de 4 grupos"
	@echo "  make open-15   - Abre vídeo de 15 grupos"
	@echo "  make clean     - Remove executáveis e vídeos"
	@echo "  make clean-all - Remove tudo (incluindo frames)"
	@echo "  make help      - Mostra esta mensagem"