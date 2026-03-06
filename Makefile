BINARY    = jpipe
CMAKE     = cmake
CTEST     = ctest
NPROC     = 1

COMPILER  ?= clang
BUILD_DIR ?= build

.PHONY: all build debug debug-asan debug-tsan release relwithdeb test-unit test-stress coverage format clean

debug:      CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(COMPILER) -DCMAKE_BUILD_TYPE=Debug
debug-asan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(COMPILER) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_ASAN=ON
debug-tsan: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(COMPILER) -DCMAKE_BUILD_TYPE=Debug          -DENABLE_TSAN=ON
release:    CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(COMPILER) -DCMAKE_BUILD_TYPE=Release        -DBUILD_TESTING=OFF
relwithdeb: CMAKE_FLAGS = -DCMAKE_C_COMPILER=$(COMPILER) -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILD_TESTING=OFF

all: debug

build:
	@echo "Build: [$(CMAKE_FLAGS)]"
	$(CMAKE) -B $(BUILD_DIR) $(CMAKE_FLAGS)
	$(CMAKE) --build $(BUILD_DIR) -j $(NPROC)

debug debug-asan debug-tsan release relwithdeb: clean build

test-unit: debug-asan
	@echo "Testing [target=debug-asan]"
	cd $(BUILD_DIR) && $(CTEST) -L unit --output-on-failure

test-stress: debug-tsan
	@echo "Testing [target=debug-tsan]"
	cd $(BUILD_DIR) && $(CTEST) -L stress --output-on-failure

coverage: test-unit
	@echo "Coverage [target=debug-asan]"
	$(CMAKE) --build $(BUILD_DIR) --target coverage

format:
	$(CMAKE) --build $(BUILD_DIR) --target format

clean:
	@echo "Cleaning [dir=/$(BUILD_DIR)]"
	rm -rf $(BUILD_DIR)
