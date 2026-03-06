# Nova Forge - Makefile
# Provides easy commands for common development tasks

# Detect OS
ifeq ($(OS),Windows_NT)
    RM := del /Q
    RMDIR := rmdir /S /Q
else
    RM := rm -f
    RMDIR := rm -rf
endif

# Logging setup
LOG_DIR := logs
TIMESTAMP := $(shell date '+%Y%m%d_%H%M%S')

# Default target
.DEFAULT_GOAL := help

.PHONY: help
help: ## Show this help message
	@echo "Nova Forge - Development Commands"
	@echo "================================="
	@echo ""
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?## "}; {printf "  \033[36m%-20s\033[0m %s\n", $$1, $$2}'
	@echo ""

.PHONY: build
build: ## Build client, server, engine, and editor (Release)
	./scripts/build.sh Release

.PHONY: build-debug
build-debug: ## Build client, server, engine, and editor (Debug)
	./scripts/build.sh Debug

.PHONY: build-client
build-client: ## Build C++ client only (use build-client-editor for F12 tools)
	@mkdir -p $(LOG_DIR)
	@mkdir -p cpp_client/build
	(cd cpp_client/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_LIBS=ON && cmake --build . --config Release) 2>&1 | tee $(LOG_DIR)/build-client_$(TIMESTAMP).log
	@echo ""
	@echo "NOTE: For F12 editor overlay, use 'make build-client-editor' instead."
	@echo ""

.PHONY: build-server
build-server: ## Build C++ server only
	@mkdir -p $(LOG_DIR)
	@mkdir -p cpp_server/build
	(cd cpp_server/build && cmake .. -DCMAKE_BUILD_TYPE=Release && cmake --build . --config Release) 2>&1 | tee $(LOG_DIR)/build-server_$(TIMESTAMP).log

.PHONY: clean
clean: ## Clean all build artifacts
	$(RMDIR) build 2>/dev/null || true
	$(RMDIR) cpp_client/build 2>/dev/null || true
	$(RMDIR) cpp_client/build_vs 2>/dev/null || true
	$(RMDIR) cpp_server/build 2>/dev/null || true
	$(RMDIR) build_vs 2>/dev/null || true
	find . -type d -name "__pycache__" -exec $(RMDIR) {} + 2>/dev/null || true
	find . -type f -name "*.pyc" -delete 2>/dev/null || true

.PHONY: check-deps
check-deps: ## Check if build dependencies are installed
	@echo "Checking build dependencies..."
	@command -v cmake >/dev/null 2>&1 && echo "  ✓ CMake" || echo "  ✗ CMake not found"
	@command -v g++ >/dev/null 2>&1 && echo "  ✓ g++" || (command -v clang++ >/dev/null 2>&1 && echo "  ✓ clang++" || echo "  ✗ No C++ compiler found")
	@pkg-config --exists glfw3 2>/dev/null && echo "  ✓ GLFW3" || echo "  ? GLFW3 (may be available via vcpkg)"
	@pkg-config --exists glew 2>/dev/null && echo "  ✓ GLEW" || echo "  ? GLEW (may be available via vcpkg)"
	@pkg-config --exists glm 2>/dev/null && echo "  ✓ GLM" || echo "  ? GLM (may be available via vcpkg)"

.PHONY: docs
docs: ## Show documentation location
	@echo "Documentation is in docs/ folder:"
	@echo ""
	@echo "  docs/guides/       - Build & setup guides"
	@echo "  docs/cpp_client/   - C++ client documentation"
	@echo "  docs/sessions/     - Development session notes"
	@echo ""
	@ls -1 docs/*.md 2>/dev/null || true

.PHONY: build-engine
build-engine: ## Build Atlas Engine library only
	@mkdir -p $(LOG_DIR)
	@mkdir -p build
	(cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ATLAS_ENGINE=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF -DBUILD_ATLAS_TESTS=OFF && cmake --build . --config Release --target AtlasEngine -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)) 2>&1 | tee $(LOG_DIR)/build-engine_$(TIMESTAMP).log

.PHONY: build-editor
build-editor: ## Build Atlas Editor (PCG design tools)
	@mkdir -p $(LOG_DIR)
	@mkdir -p build
	(cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ATLAS_ENGINE=ON -DBUILD_ATLAS_EDITOR=ON -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF -DBUILD_ATLAS_TESTS=OFF && cmake --build . --config Release --target AtlasEditor -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)) 2>&1 | tee $(LOG_DIR)/build-editor_$(TIMESTAMP).log
	@echo ""
	@echo "Atlas Editor built successfully!"
	@echo "Run with: ./build/bin/AtlasEditor"
	@echo ""
	@echo "Available editor panels:"
	@echo "  - Ship Archetype    (design reference ships per hull class)"
	@echo "  - Generation Style  (create PCG generation blueprints)"
	@echo "  - Asset Style       (modify shapes and visual styles)"
	@echo "  - PCG Preview       (preview generated content)"
	@echo "  - ECS Inspector     (inspect entity-component state)"
	@echo "  - Game Packager     (package builds)"
	@echo ""

.PHONY: build-client-editor
build-client-editor: ## Build client with editor tool layer (F12 toggle)
	@mkdir -p $(LOG_DIR)
	@mkdir -p build
	(cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ATLAS_ENGINE=ON -DBUILD_CLIENT=ON -DNOVAFORGE_EDITOR_TOOLS=ON -DBUILD_SERVER=OFF -DBUILD_ATLAS_EDITOR=OFF -DBUILD_ATLAS_TESTS=OFF && cmake --build . --config Release --target atlas_client -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)) 2>&1 | tee $(LOG_DIR)/build-client-editor_$(TIMESTAMP).log
	@echo ""
	@echo "Client built with editor tools!"
	@echo "Run with: ./build/bin/atlas_client"
	@echo "Press F12 in-game to toggle editor tool overlay"
	@echo ""

.PHONY: test
test: test-server test-engine ## Run all tests

.PHONY: test-engine
test-engine: ## Build and run Atlas Engine tests (editor disabled — requires OpenGL)
	@mkdir -p build
	cd build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_ATLAS_ENGINE=ON -DBUILD_ATLAS_TESTS=ON -DBUILD_ATLAS_EDITOR=OFF -DBUILD_CLIENT=OFF -DBUILD_SERVER=OFF && cmake --build . --config Release --target AtlasTests -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
	cd build && ./bin/AtlasTests

.PHONY: test-server
test-server: ## Build and run C++ server tests
	@mkdir -p cpp_server/build
	cd cpp_server/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DUSE_STEAM_SDK=OFF && cmake --build . --config Release --target test_systems -j$$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
	cd cpp_server/build && ./bin/test_systems

.PHONY: all
all: clean build ## Clean and build everything

.PHONY: build-all
build-all: ## Build engine, editor, client, server into build/bin/ (unified, shareable)
	./scripts/build_all.sh Release
