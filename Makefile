CMAKE     = cmake
CTEST     = ctest
NPROC     = $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

COMPILER_VERSIONS := 10 11 12 13
OPERATING_SYSTEMS := ubuntu debian
COMPILE_TARGETS   := $(foreach os,$(OPERATING_SYSTEMS),$(foreach ver,$(COMPILER_VERSIONS),compile-$(os)-$(ver)))

CC          ?= clang
TAG         ?= current
BUILD_DIR   ?= build_$(TAG)
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=$(CC)

.PHONY: all build clean clean-all help
.PHONY: debug debug-asan debug-tsan release relwithdeb
.PHONY: test test-with-asan test-with-tsan coverage format
.PHONY: compile compile-ubuntu compile-debian docker-execute $(COMPILE_TARGETS)

debug:      CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug
debug-asan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_ASAN=ON
debug-tsan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_TSAN=ON
release:    CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Release        -DBUILD_TESTING=OFF
relwithdeb: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF

all: help

build: clean
	@echo "⚙️ Building   :"
	@echo "└─ Flags      : $(CMAKE_FLAGS)"
	@echo "└─ Build Dir  : $(BUILD_DIR)"
	@echo "└─ Parallelism: $(NPROC)"
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)
	$(CMAKE) --build $(BUILD_DIR) -j $(NPROC) -- --output-sync=target

debug debug-asan debug-tsan release relwithdeb: build

test: debug
	@echo "🧪 Testing: [target=debug]"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

test-with-asan: debug-asan
	@echo "🧪 Testing: [target=debug-asan]"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

test-with-tsan: debug-tsan
	@echo "🧪 Testing: [target=debug-tsan]"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

coverage: test
	@echo "📊 Reporting: [target=debug]"
	$(CMAKE) --build $(BUILD_DIR) --target coverage

format:
	@echo "🔬 Formatting: [target=all]"
	$(CMAKE) --build $(BUILD_DIR) --target format

clean:
	@echo "🧹 Cleaning: [dir=/$(BUILD_DIR)]"
	rm -rf $(BUILD_DIR)

clean-all:
	@echo "🧹 Cleaning: [dir=./build_*]"
	rm -rf ./build_*

compile-ubuntu: $(filter compile-ubuntu-%,$(COMPILE_TARGETS))
compile-debian: $(filter compile-debian-%,$(COMPILE_TARGETS))

compile: compile-ubuntu compile-debian

$(COMPILE_TARGETS): compile-%:
	@$(MAKE) docker-execute OPERATING_SYSTEM=$(word 1,$(subst -, ,$*)) GCC_VERSION=$(word 2,$(subst -, ,$*)) 

docker-execute: # TODO: Implement Docker builds.
	@echo "🐳 Docker Building: [OS:$(OPERATING_SYSTEM)|GCC=$(GCC_VERSION)]"

help:
	@echo "[jpipe] Build System"
	@echo ""
	@echo "⚙️ Build Targets:"
	@echo "  make debug                  : Standard debug build (default)"
	@echo "  make release                : Optimized release build"
	@echo "  make relwithdeb             : Optimized release build with debug symbols"
	@echo "  make debug-asan             : Debug build with Address Sanitizer"
	@echo "  make debug-tsan             : Debug build with Thread Sanitizer"
	@echo ""
	@echo "🧪 Test & Analysis:"
	@echo "  make test                   : Run all tests"
	@echo "  make test-with-asan         : Run all tests with Memory Sanitizer"
	@echo "  make test-with-tsan         : Run all tests with Thread Sanitizer"
	@echo "  make coverage               : Generate code coverage report"
	@echo "  make format                 : Run clang-format"
	@echo ""
	@echo "🐳 Docker & Multi-Compiler:"
	@echo "  make compile                : Build all OS (Ubuntu & Debian) and GCC versions [10-13]"
	@echo "  make compile-ubuntu         : Build only for Ubuntu and all GCC versions [10-13]"
	@echo "  make compile-ubuntu-[10-13] : Build only for Ubuntu and specific GCC version"
	@echo "  make compile-debian         : Build only for Debian and all GCC versions [10-13]"
	@echo "  make compile-debian-[10-13] : Build only for Debian and specific GCC version"
	@echo ""
	@echo "🧹 Cleanup:"
	@echo "  make clean                  : Remove the current BUILD_DIR ($(BUILD_DIR))"
	@echo "  make clean-all              : Remove all build directories (./build_*)"
	@echo ""
	@echo "💡 Variables (Override via command line):"
	@echo "  CC                          : Compiler selection (e.g., make debug CC=gcc)"
	@echo "  TAG                         : Suffix for build directory (e.g., make release TAG=v1)"
	@echo ""
	@echo "📌️ Example:"
	@echo "  make release TAG=v1 CC=clang"
	@echo "  make relwithdeb TAG=pre-release CC=gcc"
