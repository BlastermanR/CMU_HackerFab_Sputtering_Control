/**
 * @file USBStream.h
 * @brief SputterOS `IStream` implementation backed by the Pico SDK's USB CDC stdio.
 *
 * Wraps `getchar_timeout_us`, `fwrite`, and `stdio_usb_connected` to expose
 * the non-blocking byte-stream interface that `SputterOS::CommsTask` expects.
 * A one-byte look-ahead buffer lets `available()` remain non-consuming.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */
#ifndef USB_STREAM_H
#define USB_STREAM_H

#include "sputteros/hal/devices/IStream.h"
#include <cstddef>
#include <cstdint>

/**
 * @brief Non-blocking USB CDC byte-stream implementing `SputterOS::IStream`.
 *
 * All methods are non-blocking. `available()` uses a single-byte peek buffer
 * so the byte is not consumed until `read()` is called.
 */
class USBStream : public SputterOS::IStream
{
  public:
    /**
     * @brief Default constructor. No hardware initialisation required here;
     * call `stdio_init_all()` before using this stream.
     */
    USBStream() = default;

    /**
     * @brief Return the number of bytes available without blocking.
     * @return 1 if a byte is waiting (in the peek buffer or the FIFO), 0 otherwise.
     */
    std::size_t available() const override;

    /**
     * @brief Read up to `max_len` bytes from the USB receive buffer.
     * @param buffer Destination for incoming bytes.
     * @param max_len Maximum number of bytes to read.
     * @return Number of bytes placed in `buffer`.
     */
    std::size_t read(uint8_t *buffer, std::size_t max_len) override;

    /**
     * @brief Write `len` bytes to the USB transmit buffer.
     * @param data Source buffer to transmit.
     * @param len Number of bytes to send.
     * @return Number of bytes accepted.
     */
    std::size_t write(const uint8_t *data, std::size_t len) override;

    /**
     * @brief Check whether a USB host is connected.
     * @return true if the USB CDC link is active.
     */
    bool isConnected() const override;

  private:
    /** @brief One-byte look-ahead buffer for non-consuming `available()`. */
    mutable bool    m_hasPeeked{false};
    mutable uint8_t m_peeked{0};
};

#endif // USB_STREAM_H
