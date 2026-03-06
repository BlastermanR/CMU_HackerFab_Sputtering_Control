#ifndef USB_SERIAL
#define USB_SERIAL

/**
 * Implements a Serial interface for recieving input from a host PC.
 * To be used for simple tests, not final serial implementation
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <string>
#include <functional>

// Callback passes a full string command once the user hits 'Enter'
typedef std::function<void(const std::string&)> UsbCommandCallback;

class USBSerial {
private:
    std::string inputBuffer;
    UsbCommandCallback commandCallback;

public:
    // Constructor
    USBSerial();

    // Setup and main loop functions
    void begin();
    void update(); 
    
    // Assign the callback function
    void setCallback(UsbCommandCallback cb);

    // Output functions
    void print(const char* str);
    void println(const char* str);
};

#endif // USB_SERIAL