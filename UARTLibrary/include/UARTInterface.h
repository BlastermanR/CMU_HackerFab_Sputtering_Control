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
    virtual ~IUart() = default;

    // Forces derived classes to implement these basic functions
    virtual void begin() = 0;
    virtual void write(char c) = 0;
    virtual void print(const char* str) = 0;
    virtual void waitTxComplete() = 0;
    virtual void setCallback(UartCallback cb) = 0;
};

#endif // UARTI