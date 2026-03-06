#ifndef ALICAT_MFC
#define ALICAT_MFC

/**
 * Defines the Alicat Mass Flow Controller (MFC).
 * - Utilizes UART --> RS232 converter (MAX3232)
 * - Interrupt driven reciever handler
 * 
 * @author Ryan Massie (rmassie)
 * @date 3/4/26
 */

#include <stdio.h>
#include "UARTInterface.h"

class AlicatMFC
{
    private:
    // Define the serial port
    IUart* serialPort;

    // Define recieve buffer
    static constexpr int BUFFER_SIZE = 100;

    // Recieve Buffer
    char recieveBuffer[BUFFER_SIZE + 1];

    // Read buffer
    int bufferIndex;

    // Handler for chars recieved by device
    void onDataReceived(char c);

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