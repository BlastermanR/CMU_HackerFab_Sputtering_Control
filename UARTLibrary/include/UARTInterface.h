#ifndef UARTI
#define UARTI

/**
 * Implements a UART Interface to be extended to equip both PIO and hardware UART
 * for portability.
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <functional>

// Use std::function so complex objects can pass lambdas or bound member functions
typedef std::function<void(char)> UartCallback;

class IUart {
public:
    /**
     * @brief Virtual destructor for IUart.
     */
    virtual ~IUart() = default;

    /**
     * @brief Initializes the UART interface.
     */
    virtual void begin() = 0;

    /**
     * @brief Writes a single character to the UART.
     * @param c The character to write.
     */
    virtual void write(char c) = 0;

    /**
     * @brief Prints a null-terminated string to the UART.
     * @param str The string to print.
     */
    virtual void print(const char* str) = 0;

    /**
     * @brief Waits for any pending transmissions to complete.
     */
    virtual void waitTxComplete() = 0;

    /**
     * @brief Sets the callback function to be called when data is received.
     * @param cb The callback function taking a char parameter.
     */
    virtual void setCallback(UartCallback cb) = 0;
};

#endif // UARTI