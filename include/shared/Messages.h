/**
 * Messages.h
 * @brief Shared message types for inter-core queue communication.
 *
 * @author Ryan Massie (rmassie)
 * @date 3/31/26
 */
#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

#define OUTPUT_MSG_TEXT_LEN 64
#define COMMAND_QUEUE_SIZE  16
#define OUTPUT_QUEUE_SIZE   32

/**
 * @brief Runtime verbosity levels for output filtering.
 * Messages with a level <= the current verbosityLevel are printed.
 */
enum Verbosity : uint8_t
{
    V_CRITICAL = 0,
    V_STATUS   = 1,
    V_INFO     = 2,
    V_DEBUG    = 3,
};

/**
 * @brief Identifiers for inbound commands received over USB.
 */
enum CommandId : uint8_t
{
    Cmd_None = 0,
    Cmd_StartProcess,
    Cmd_StopProcess,
    Cmd_PressurizeChamber,
    Cmd_VentChamber,
    Cmd_ShutOffGas,
    Cmd_PollDevices,
    Cmd_SetArgonFlow,
    Cmd_SetOxygenFlow,
    Cmd_SetPumpSpeed,
    Cmd_EnablePump,
    Cmd_DisablePump,
    Cmd_Exit,
    Cmd_SetVerbosity,
};

/**
 * @brief Fixed-size command message pushed into the command queue.
 */
struct CommandMessage
{
    CommandId id;
    float     param1;
    float     param2;
};

/**
 * @brief Identifies which core originated an output message.
 */
enum MessageSource : uint8_t
{
    Source_Core0 = 0,
    Source_Core1 = 1,
};

/**
 * @brief Fixed-size output message pushed into the output queues.
 */
struct OutputMessage
{
    MessageSource source;
    Verbosity     level;
    char          text[OUTPUT_MSG_TEXT_LEN];
};

#endif // MESSAGES_H
