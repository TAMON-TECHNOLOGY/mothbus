#pragma once
#include "../mothbus.h"
#include "../pdu.h"
#include "buffer.h"
#include "master.h"

#include "crc.h"


namespace mothbus
{
	namespace rtu
	{
		template <class NextLayer>
		class stream
		{
		public:
			using next_layer_type = typename std::remove_reference<NextLayer>::type;

			stream(NextLayer& next_layer)
				: m_next_layer(next_layer)
				, m_io_context(static_cast<boost::asio::io_context&>(next_layer.get_executor().context()))
			{
			}

			template <class Req>
			uint16_t write_request(uint8_t slave, const Req& request)
			{
				adu::buffer sink{ m_messageBuffer };
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

				return m_transaction_id++;
			}


			// maybe not called in slave mode
			template <class Resp>
			error_code read_response(uint16_t, uint8_t expected_slave, Resp& out)
			{
				error_code ec;
				receive_response(expected_slave, out, ec);
				return ec;
			}
			template <class Resp>
			void receive_response(uint8_t expectedSlave, Resp& out, error_code& ec)
			{
				using pdu::read;
				adu::buffer source(m_messageBuffer);

				std::size_t readSize = read_byte_with_timeout(source, std::chrono::microseconds(m_timeoutResponse));
				if (!readSize) {
					ec = make_error_code(boost::system::errc::timed_out);
					return;
				}
				source.commit(readSize);


				uint8_t receivedSlave;
				read(source, receivedSlave);
				if (receivedSlave != expectedSlave) {
					ec = make_error_code(boost::system::errc::protocol_error);
					return;
				}


				while (true) {
					readSize = read_byte_with_timeout(source, std::chrono::microseconds(m_timeoutInterCharacter));

					if (!readSize) {
						break;
					}
					source.commit(readSize);
				} // while (true)


				pdu::pdu_resp<Resp> combinedResponse{ out };
				read(source, combinedResponse);


				const uint16_t crc = CRC16(span{ m_messageBuffer.data(), source.input_start });
				std::uint8_t rcvCrcL;
				std::uint8_t rcvCrcH;
				read(source, rcvCrcL);
				read(source, rcvCrcH);
				if (rcvCrcL != (crc & 0xff) || rcvCrcH != ((crc >> 8) & 0xff)) {
					ec = make_error_code(modbus_exception_code::invalid_response);
					return;
				}

				ec.clear();
			}

		private:
			/*!
			 * \brief タイムアウト付きで1バイトを非同期に読み込む
			 * \param source 読み込み先のバッファ
			 * \param timeout タイムアウト時間
			 * \return 読み込んだバイト数. タイムアウトまたはエラーの場合は0を返す
			 */
			std::size_t read_byte_with_timeout(adu::buffer& source, std::chrono::microseconds timeout)
			{
				boost::asio::steady_timer timer{ m_io_context };
				timer.expires_from_now(timeout);

				std::size_t bytes_read = 0;

				mothbus::async_read(m_next_layer, source.prepare(1),
					[&timer, &bytes_read](const boost::system::error_code& e, std::size_t size) {
						if (!e && size > 0) {
							bytes_read = size;
							timer.cancel();
						}
					});
				timer.async_wait(
					[this](const boost::system::error_code& e) {
						if (e != boost::asio::error::operation_aborted) {
							m_next_layer.cancel();
						}
					});

				m_io_context.run();
				m_io_context.reset(); // for next call

				return bytes_read;
			}

		private:
			std::array<uint8_t, 320> m_messageBuffer = { 0 };
			NextLayer& m_next_layer;
			uint16_t m_transaction_id{ 0 };
			boost::asio::io_context& m_io_context;
			std::uint32_t m_timeoutInterCharacter{ 1800 }; // [us], T1.5
			//std::uint32_t m_timeoutInterFrame{ 4000 }; // [us], T3.5
			std::uint32_t m_timeoutResponse{ 10000 }; // [us]
		};
	} // namespace rtu

	template<class Stream>
	using rtu_master = adu::master_base<rtu::stream<Stream>>;

} // namespace mothbus
