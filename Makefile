# Makefile for SputteringACS Project (Raspberry Pi Pico 2)
# Replicates common actions from the VS Code extension
#
# @author Ryan Massie (rmassie)
# @date 3/20/26
#
# Commands:
#
# make compile
#     The default action. It automatically runs CMake (if the build folder is missing) 
#     and then executes ninja to compile the project.
#
# make run
#     Compiles the project and then uses picotool to load the target into your 
#     Pico 2's RAM and execute it.
#
# make flash
#     Compiles the project and then uses openocd to program the binary into 
#     Flash memory via the CMSIS-DAP interface.
#
# make format
#     Recursively runs clang-format -i on all .cpp, .h, and .pio files across 
#     your workspace while ignoring the build folder entirely. 
#
# make test
#     Builds and runs the Google Test unit test suite on the host.
#
# make clean
#     Safely removes the build directory for a fresh state.
#
# make rescue
#     Runs the Pico extension's "Rescue Reset" command to unbrick or 
#     reset a stalled RP2350 target.

# --- Project Paths ---
BUILD_DIR ?= build
TEST_BUILD_DIR ?= build_tests
PICOTOOL ?= ${USERPROFILE}/.pico-sdk/picotool/2.2.0-a4/picotool/picotool.exe
OPENOCD ?= ${USERPROFILE}/.pico-sdk/openocd/0.12.0+dev/openocd.exe
OPENOCD_SCRIPTS ?= ${USERPROFILE}/.pico-sdk/openocd/0.12.0+dev/scripts
BIN_TARGET = SputteringACS.elf

# --- Default Goal ---
.PHONY: all
all: compile

# --- High-level Commands ---

# 1. Compile: Generates build files if needed then compiles
.PHONY: compile
compile:
	@if [ ! -d "$(BUILD_DIR)" ]; then cmake -B $(BUILD_DIR) -G Ninja; fi
	ninja -C $(BUILD_DIR)

# 2. Run: Loads the program using picotool
.PHONY: run
run: compile
	$(PICOTOOL) load $(BUILD_DIR)/$(BIN_TARGET) -fx

# 3. Flash: Flashes using OpenOCD
.PHONY: flash
flash: compile
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) -f interface/cmsis-dap.cfg -f target/rp2350.cfg -c "adapter speed 5000; program \"$(BUILD_DIR)/$(BIN_TARGET)\" verify reset exit"

# 4. Format: Runs clang-format on all relevant sources
.PHONY: format
format:
	@echo Formatting project files...
	@find . -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/$(BUILD_DIR)/*" -exec clang-format -i {} +

# 5. Clean: Removes build directory
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(TEST_BUILD_DIR)

# 6. Test: Builds and runs the Google Test unit test suite on the host
# Usage:
#   make test        => run the test suite
.PHONY: test memory-usage
test: compile
	@if [ ! -d "$(TEST_BUILD_DIR)" ]; then cmake -B $(TEST_BUILD_DIR) -S Tests -G Ninja; fi
	ninja -C $(TEST_BUILD_DIR)
	cd $(TEST_BUILD_DIR) && ctest --output-on-failure

# 7. Memory usage: Runs the Pico memory usage script against the current build.
memory-usage: compile
	python Tools/pico_memory_usage.py $(BUILD_DIR)/$(BIN_TARGET)

# 7. Helper: Rescue Reset
.PHONY: rescue
rescue:
	$(OPENOCD) -s $(OPENOCD_SCRIPTS) -f interface/cmsis-dap.cfg -f target/rp2350-rescue.cfg -c "adapter speed 5000; reset halt; exit"
