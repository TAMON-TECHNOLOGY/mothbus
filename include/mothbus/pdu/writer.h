#pragma once
//#include <mothbus/mothbus.h>
#include "../mothbus.h"


namespace mothbus::pdu
{
    template <class Writer, class C>
    void write(Writer& writer, const C& v);

    /*!
     * \brief	write data to sink
     */
    template <class Sink>
    class writer
    {
    public:
        writer(Sink& sink)
            : m_sink(sink)
        {
        };

        /*!
         * \brief	write word to sink, considering endianness.
         */
        inline void write(uint16_t v)
        {
            m_sink.put((v >> 8) & 0xff);
            m_sink.put(v & 0xff);
        }

        inline void write(uint8_t v)
        {
            m_sink.put(v);
        }

        inline void write(byte v)
        {
            m_sink.put(static_cast<uint8_t>(v));
        }


    private:
        Sink& m_sink;
    };

    template <class Writer>
    void write(Writer& writer, uint8_t v)
    {
        writer.write(v);
    }

    template <class Writer>
    void write(Writer& writer, uint16_t v)
    {
        writer.write(v);
    }

    template <class Writer>
    inline void write(Writer& writer, const span<byte>& v)
    {
        for (const auto b : v)
        {
            writer.write(std::to_integer<uint8_t>(b));
        }
    }

    template <class Writer>
    inline void write(Writer& writer, const span<uint16_t>& v)
    {
        for (const auto u : v)
        {
            writer.write(u);
        }
    }

    template <class Writer>
    void write(Writer& writer, const modbus_exception_code& v)
    {
        write(writer, static_cast<uint8_t>(v));
    }
} // namespace mothbus::pdu
