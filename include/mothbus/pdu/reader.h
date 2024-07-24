#pragma once
#include <cstdint>
#include "../mothbus.h"
#include "../error.h"


namespace mothbus::pdu
{
    template <class Reader, class C>
    error_code read(Reader& reader, C& v);

    template <class Reader>
    error_code read(Reader& reader, uint8_t& v)
    {
        reader.get(v);
        return{};
    }

    template <class Reader>
    error_code read(Reader& reader, uint16_t& v)
    {
        uint8_t b = 0;
        { auto ec = read(reader, b); if (!!ec) return ec; }
        v = b << 8;
        { auto ec = read(reader, b); if (!!ec) return ec; }
        v |= b;
        return{};
    }

    template <class Reader>
    error_code read(Reader& reader, span<byte>& v)
    {
        for (auto& b : v)
        {
            uint8_t temp = 0;
            { auto ec = read(reader, temp); if (!!ec) return ec; }
            b = byte(temp);
        }
        return{};
    }

    template <class Reader>
    error_code read(Reader& reader, modbus_exception_code& v)
    {
        uint8_t h = 0;
        { auto ec = read(reader, h); if (!!ec) return ec; }
        v = static_cast<modbus_exception_code>(h);
        return{};
    }
} // namespace mothbus::pdu
