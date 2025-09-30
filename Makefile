# Makefile for Ant Colony Simulation (Simple Version)
# Usage:
#   make run       - Compile, run simulation and generate video
#   make compile   - Only compile the simulation
#   make simulate  - Only run the simulation
#   make video     - Generate video from frames
#   make open      - Open the generated video
#   make clean     - Clean all generated files

CXX = g++
CXXFLAGS = -std=c++20
SOURCE = simulacao.cpp
EXECUTABLE = simulacao
OUTPUT_VIDEO = out.mp4

.PHONY: all run compile simulate video open clean clean-frames clean-all

all: run

# ============================================
# Complete workflow: compile + simulate + video
# ============================================
run: clean-frames compile simulate video
	@echo ""
	@echo "========================================="
	@echo "✅ CONCLUÍDO! Vídeo gerado: $(OUTPUT_VIDEO)"
	@echo "========================================="
	@echo "Para abrir o vídeo, execute: make open"

# ============================================
# Compile only
# ============================================
compile:
	@echo "Compilando $(SOURCE)..."
	@$(CXX) $(CXXFLAGS) $(SOURCE) -o $(EXECUTABLE)
	@echo "✅ Compilação concluída!"

# ============================================
# Run simulation only
# ============================================
simulate: compile
	@echo ""
	@echo "========================================="
	@echo "Executando simulação..."
	@echo "========================================="
	@./$(EXECUTABLE)

# ============================================
# Generate video from frames
# ============================================
video:
	@echo ""
	@echo "Gerando vídeo $(OUTPUT_VIDEO)..."
	@ffmpeg -y -framerate 20 -i frames/frame_%05d.ppm -vf scale=640:640 -c:v libx264 -pix_fmt yuv420p $(OUTPUT_VIDEO) 2>/dev/null || \
	(echo "❌ Erro ao gerar vídeo. Verifique se o ffmpeg está instalado." && exit 1)
	@echo "✅ Vídeo gerado com sucesso!"

# ============================================
# Open video (macOS)
# ============================================
open:
	@if [ -f $(OUTPUT_VIDEO) ]; then \
		open $(OUTPUT_VIDEO); \
	else \
		echo "❌ Arquivo $(OUTPUT_VIDEO) não encontrado. Execute 'make run' primeiro."; \
	fi

# ============================================
# Clean targets
# ============================================
clean-frames:
	@rm -rf frames/
	@mkdir -p frames/
	@echo "🧹 Diretório frames limpo!"

clean:
	@rm -f $(EXECUTABLE) *.tmp
	@rm -f $(OUTPUT_VIDEO)
	@echo "🧹 Arquivos executáveis e vídeos removidos!"

clean-all: clean clean-frames
	@echo "🧹 Todos os arquivos gerados foram removidos!"

# ============================================
# Help
# ============================================
help:
	@echo "Comandos disponíveis:"
	@echo "  make run       - Compila, executa e gera vídeo (workflow completo)"
	@echo "  make compile   - Apenas compila o código"
	@echo "  make simulate  - Compila e executa a simulação"
	@echo "  make video     - Gera vídeo a partir dos frames"
	@echo "  make open      - Abre o vídeo gerado"
	@echo "  make clean     - Remove executáveis e vídeos"
	@echo "  make clean-all - Remove tudo (incluindo frames)"
	@echo "  make help      - Mostra esta mensagem"
