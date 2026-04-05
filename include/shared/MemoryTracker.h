/**
 * @file MemoryTracker.h
 * @brief Memory profiling helper for Raspberry Pi Pico.
 *
 * Tracks current stack and heap usage using Pico linker symbols and newlib
 * allocation statistics. Attach an optional USBSerial logger to emit runtime
 * memory reports over USB.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/21/26
 */
#ifndef MEMORY_TRACKER
#define MEMORY_TRACKER

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <malloc.h>
#include "Messages.h"
#include "picoDefinitions.h"
#include "USBSerial.h"

// These symbols are provided by the Raspberry Pi Pico SDK linker script (memmap_default.ld).
// They mark the highest and lowest addresses reserved for the system stack.
extern char __StackTop;
extern char __StackBottom;

/**
 * @brief Lightweight memory profiler for Pico applications.
 *
 * This class tracks stack and heap usage by querying the current stack pointer
 * and the heap allocation statistics from newlib's mallinfo().
 * It is intended to be used as a static utility only.
 */
class MemoryProfiler 
{
private:
    /**
     * @brief Total stack capacity computed from linker-provided bounds.
     */
    inline static size_t totalStackCapacity = 0;

    /**
     * @brief Current stack usage in bytes.
     */
    inline static size_t currentStackUsage = 0;

    /**
     * @brief Current heap usage in bytes from newlib mallinfo().
     */
    inline static size_t currentHeapUsage = 0;

    /**
     * @brief Total combined stack and heap usage.
     */
    inline static size_t totalUsage = 0;

    /**
     * @brief Optional USB serial logger used by print().
     */
    inline static USBSerial *usbSerialLogger = nullptr;
public:
    // Prevent instantiation; all members are static.
    MemoryProfiler() = delete;

    /**
     * @brief Initialize stack capacity and take the first usage snapshot.
     *
     * Call this once early in main() so the stack capacity is computed correctly.
     */
    static void init(USBSerial *serial = nullptr) {
        // On the Cortex-M architecture used by the Pico, the stack grows downward.
        // The total usable stack capacity is the difference between the top of stack
        // and the stack bottom marker defined by the linker.
        totalStackCapacity = static_cast<size_t>(&__StackTop - &__StackBottom);
        usbSerialLogger = serial;
        updateUsage();
    }

    /**
     * @brief Attach a USBSerial logger instance for later print() calls.
     *
     * This allows print() to be invoked without passing the logger each time.
     */
    static void attachLogger(USBSerial *serial) {
        usbSerialLogger = serial;
    }

    /**
     * @brief Sample the current stack and heap usage.
     *
     * The current stack pointer is approximated by taking the address of a
     * local volatile variable. Heap usage is read from newlib's mallinfo().
     */
    static void updateUsage() {
        // 1. Determine current stack usage.
        // Taking the address of a local volatile variable gives a stable
        // approximation for the current stack pointer.
        volatile char current_sp;
        currentStackUsage = static_cast<size_t>(&__StackTop - const_cast<char*>(&current_sp));

        // 2. Determine current heap usage.
        // mallinfo().uordblks reports the total allocated heap bytes.
        struct mallinfo m = mallinfo();
        currentHeapUsage = static_cast<size_t>(m.uordblks);

        // 3. Compute combined dynamic memory usage.
        totalUsage = currentHeapUsage + currentStackUsage;
    }

    /**
     * Getter Functions
     */
    static size_t getTotalStackCapacity() { return totalStackCapacity; }
    static size_t getCurrentStackUsage()  { return currentStackUsage; }
    static size_t getCurrentHeapUsage()   { return currentHeapUsage; }
    static size_t getTotalUsage()         { return totalUsage; }

    /**
     * @brief Emit the current memory profile through the attached USB serial logger.
     * @param source Originating message source for the log entry.
     * @param level  Verbosity level for the log entry.
     * @note If no logger is attached, this method does nothing.
     */
    static void print(MessageSource source = Source_Core0,
                      Verbosity level = V_STATUS)
    {
        USBSerial *serial = usbSerialLogger;
        if (serial == nullptr)
        {
            return;
        }

        updateUsage();

        size_t used = totalUsage;
        size_t remaining = (PICO2_SRAM_BYTES > used) ? (PICO2_SRAM_BYTES - used) : 0;
        char buffer[96];

        std::snprintf(buffer, sizeof(buffer), "[ MEMORY USAGE ]");
        serial->log(source, buffer, level);

        std::snprintf(buffer, sizeof(buffer), " Used:                %zu bytes (%.2f KiB)",
                      used, used / 1024.0);
        serial->log(source, buffer, level);

        std::snprintf(buffer, sizeof(buffer), " Memory Remaining:    %zu bytes (%.2f KiB)",
                      remaining, remaining / 1024.0);
        serial->log(source, buffer, level);

        std::snprintf(buffer, sizeof(buffer), " Capacity:            %zu bytes (%.2f KiB) on Pico 2",
                      PICO2_SRAM_BYTES, PICO2_SRAM_BYTES / 1024.0);
        serial->log(source, buffer, level);
    }


};

#endif // MEMORY_TRACKER
