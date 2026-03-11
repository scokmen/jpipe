BINARY    = jpipe
CMAKE     = cmake
CTEST     = ctest
NPROC     = $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)

COMPILER_VERSIONS := 10 11 12 13
OPERATING_SYSTEMS := ubuntu debian
COMPILE_TARGETS   := $(foreach os,$(OPERATING_SYSTEMS),$(foreach ver,$(COMPILER_VERSIONS),compile-$(os)-$(ver)))

CC          ?= clang
BUILD_DIR   ?= build
CMAKE_FLAGS ?= -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=$(CC)

.PHONY: all build clean
.PHONY: debug debug-asan debug-tsan release relwithdeb
.PHONY: test test-asan-flow test-tsan-flow coverage format
.PHONY: compile compile-ubuntu compile-debian docker-execute $(COMPILE_TARGETS)

debug:      CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug
debug-asan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_ASAN=ON
debug-tsan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_TSAN=ON
release:    CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=Release        -DBUILD_TESTING=OFF
relwithdeb: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(CC) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF

all: debug

build: clean
	@echo "⚙️ Building   :"
	@echo "└─ Flags      : $(CMAKE_FLAGS)"
	@echo "└─ Parallelism: $(NPROC)"
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)
	$(CMAKE) --build $(BUILD_DIR) -j $(NPROC) -- --output-sync=target

debug debug-asan debug-tsan release relwithdeb: build

test: debug
	@echo "🧪 Testing: [target=debug]"
	cd $(BUILD_DIR) && $(CTEST) --output-on-failure

test-asan-flow: debug-asan
	@echo "🧪 Testing: [target=debug-asan]"
	cd $(BUILD_DIR) && $(CTEST) -L unit --output-on-failure

test-tsan-flow: debug-tsan
	@echo "🧪 Testing: [target=debug-tsan]"
	cd $(BUILD_DIR) && $(CTEST) -L stress --output-on-failure

coverage: test
	@echo "📊 Reporting: [target=debug-asan]"
	$(CMAKE) --build $(BUILD_DIR) --target coverage

format:
	@echo "🔬 Formatting: [target=all]"
	$(CMAKE) --build $(BUILD_DIR) --target format

clean:
	@echo "🧹 Cleaning: [dir=/$(BUILD_DIR)]"
	rm -rf $(BUILD_DIR)

compile-ubuntu: $(filter compile-ubuntu-%,$(COMPILE_TARGETS))
compile-debian: $(filter compile-debian-%,$(COMPILE_TARGETS))

compile: compile-ubuntu compile-debian

$(COMPILE_TARGETS): compile-%:
	@$(MAKE) docker-execute OPERATING_SYSTEM=$(word 1,$(subst -, ,$*)) GCC_VERSION=$(word 2,$(subst -, ,$*)) 

docker-execute:
	@echo "🐳 Docker Building: [OS:$(OPERATING_SYSTEM)|GCC=$(GCC_VERSION)]"
