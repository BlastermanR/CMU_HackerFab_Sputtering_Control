#ifndef ALICAT_MFC
#define ALICAT_MFC

/**
 * Implements the Alicat Mass Flow Controller (MFC).
 * - Utilizes UART --> RS232 converter (MAX3232)
 * - Interrupt driven reciever handler
 */

#include <stdio.h>
#include "UARTInterface.h"

class AlicatMFC
{
    private:
    // Define the serial port
    IUart* serialPort;

    void onDataReceived(char c) {
        printf("MessageRecieved: %c\n", c);
    }

    public:
    // Constructor
    AlicatMFC(IUart* uart);

    // Destructor
    ~AlicatMFC();

    // Initialization Function
    void init();

    void sendMessage(const char* message);
};


#endif // ALICAT_MFC