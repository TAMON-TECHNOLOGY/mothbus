#pragma once
#include <mothbus/mothbus.h>
#include <mothbus/pdu.h>
#include <mothbus/adu/buffer.h>
#include <mothbus/adu/master.h>

#include "crc.h"


namespace mothbus
{
	namespace rtu
	{
		template <class NextLayer>
		class stream
		{
		public:
			stream(NextLayer& next_layer)
				: m_next_layer(next_layer)
			{
			}

			template <class Req>
			void write_request(uint8_t slave, const Req& request)
			{
				adu::buffer sink{m_messageBuffer};
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, slave);
				pdu::write(writer, request);

				const uint16_t crc = CRC16(span<uint8_t>(m_messageBuffer.data(), sink.output_start));
				const uint8_t low = crc & 0xff;
				const uint8_t high = (crc >> 8) & 0xff;
				pdu::write(writer, low);
				pdu::write(writer, high);

				const auto index = sink.output_start;
				sink.commit(index);
				boost::system::error_code ec;
				mothbus::write(m_next_layer, sink.data(), ec);
			}


			// arrange interface for master_base
			template <class Resp>
			error_code read_response(uint16_t, uint8_t expected_slave, Resp& out)
			{
				error_code ec;
				receive_response(expected_slave, out, ec);
				return ec;
			}
			template <class Resp>
			void receive_response(uint8_t expectedSlave, Resp& out, boost::system::error_code& ec)
			{
				adu::buffer source(m_messageBuffer);
				auto readBuffer = boost::asio::buffer(m_messageBuffer);
				size_t readSize = 0;
				readSize += mothbus::read(m_next_layer, source.prepare(255), ec);
				source.commit(readSize);
				pdu::reader<adu::buffer> reader(source);
				uint8_t receivedSlave;
				pdu::read(reader, receivedSlave);
				if (receivedSlave != expectedSlave) {
					return;
				}

				/*readSize = mothbus::read(stream, source.prepare(254), ec);
				if (length + 6 > 255 || length <= 1)
				{
					return;
					//throw modbus_exception(10);
				}*/
				source.commit(readSize);
				pdu::pdu_resp<Resp> combinedResponse{out};
				pdu::read(reader, combinedResponse);
			}
		private:
			std::array<uint8_t, 320> m_messageBuffer;
			NextLayer& m_next_layer;
		};
	} // namespace rtu

	template<class Stream>
	using rtu_master = adu::master_base<rtu::stream<Stream>>;

} // namespace mothbus
