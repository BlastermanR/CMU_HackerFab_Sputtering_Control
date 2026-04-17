/**
 * @file USBStream.cpp
 * @brief Implementation of the USBStream SputterOS IStream adapter.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/16/26
 */

#include "USBStream.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include <cstdio>

// ---------------------------------------------------------------------------
// IStream overrides
// ---------------------------------------------------------------------------

std::size_t USBStream::available() const
{
    if (m_hasPeeked)
    {
        return 1;
    }

    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT)
    {
        return 0;
    }

    m_peeked    = static_cast<uint8_t>(c);
    m_hasPeeked = true;
    return 1;
}

std::size_t USBStream::read(uint8_t *buffer, std::size_t max_len)
{
    if (max_len == 0)
    {
        return 0;
    }

    std::size_t count = 0;

    // Drain the peek buffer first.
    if (m_hasPeeked)
    {
        buffer[count++] = m_peeked;
        m_hasPeeked     = false;
        if (count >= max_len)
        {
            return count;
        }
    }

    // Non-blocking read of remaining bytes.
    while (count < max_len)
    {
        int c = getchar_timeout_us(0);
        if (c == PICO_ERROR_TIMEOUT)
        {
            break;
        }
        buffer[count++] = static_cast<uint8_t>(c);
    }

    return count;
}

std::size_t USBStream::write(const uint8_t *data, std::size_t len)
{
    return fwrite(data, 1, len, stdout);
}

bool USBStream::isConnected() const
{
    return stdio_usb_connected();
}
