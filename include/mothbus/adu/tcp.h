/*!
 * \file		tcp.h
 * \brief		The Implementation of Application Data Unit for modbus/TCP.
 */
#pragma once
#include <boost/asio.hpp>
#include "../mothbus.h"
#include "../pdu.h"
#include "buffer.h"
#include "master.h"

namespace mothbus
{
	namespace tcp
	{
		/*!
		 * \brief			parse MBAP header
		 * \return			<transaction_id, protocol, message_length, message_length, unit_id>
		 *
		 * if there is no define type or set variant correspoinding to read function code,
		 * set req to \a not_implemented .
		 */
		inline std::tuple<std::uint16_t, std::uint16_t, std::uint16_t, std::uint8_t>
			parse_header(adu::buffer& source, std::size_t read_size)
		{
			using mothbus::pdu::read;

			std::uint16_t transaction_id = 0;
			std::uint16_t protocol = 0;
			std::uint16_t message_length = 0;
			std::uint8_t unit_id = 0;

			source.commit(read_size);
			read(source, transaction_id);
			read(source, protocol);
			read(source, message_length);
			read(source, unit_id);

			return { transaction_id , protocol, message_length, unit_id};
		}

		/*!
		 * \brief		parse PDU
		 * \param		source		message source
		 * \param		read_size	read size for source
		 * \param[out]	request		value according to read function code
		 */
		inline error_code parse_body(adu::buffer& source, std::size_t read_size, pdu::pdu_req& request)
		{
			using pdu::read;
			source.commit(read_size);
			return read(source, request); // call read function in pdu.h ("error_code read(Reader& reader, pdu_req& req)")
		}


		template <class NextLayer>
		class stream
		{
		public:
			using next_layer_type = typename std::remove_reference<NextLayer>::type;

			stream(NextLayer& next_layer)
				: m_next_layer(next_layer)
			{
			}

			template <class Req>
			uint16_t write_request(uint8_t slave, const Req& request)
			{
				adu::buffer sink{m_message_buffer};
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, m_transaction_id);
				pdu::write(writer, m_protocol);
				uint16_t length = 0;
				pdu::write(writer, length); // place holder
				pdu::write(writer, slave);

				pdu::write(writer, request);

				const auto index = sink.output_start;
				length = static_cast<uint16_t>(sink.output_start - 6);
				sink.output_start = 4;
				pdu::write(writer, length); // write actual length

				sink.output_start = index;
				sink.commit(index);
				mothbus::write(m_next_layer, sink.data());
				return m_transaction_id++;
			}

			template <class Resp>
			error_code read_response(uint16_t expected_transaction_id, uint8_t expected_slave, Resp& out)
			{
				if (expected_slave == 0) { // boradcast
					ec.clear();
					return;
				}

				using pdu::read;
				adu::buffer source(m_message_buffer);
				size_t read_size = 0;
				read_size += mothbus::read(m_next_layer, source.prepare(7));
				source.commit(read_size);
				uint16_t received_transaction_id = 0;
				uint16_t protocol = 0;
				uint16_t length = 0;
				uint8_t received_slave = 0;
				MOTH_CHECKED_RETURN(read(source, received_transaction_id));
				MOTH_CHECKED_RETURN(read(source, protocol));
				MOTH_CHECKED_RETURN(read(source, length));
				MOTH_CHECKED_RETURN(read(source, received_slave)); // unit identifier

				if (received_transaction_id != expected_transaction_id) {
					return make_error_code(modbus_exception_code::transaction_id_invalid);
				}
				if (protocol != m_protocol) {
					return make_error_code(modbus_exception_code::illegal_protocol);
				}
				if (received_slave != expected_slave) {
					return make_error_code(modbus_exception_code::slave_id_invalid);
				}
				if (length + 6 > 255 || length <= 1) {
					return make_error_code(modbus_exception_code::invalid_response);
				}

				read_size = mothbus::read(m_next_layer, source.prepare(length - 1));
				source.commit(read_size);
				pdu::pdu_resp<Resp> combined_response{out};
				MOTH_CHECKED_RETURN(read(source, combined_response)); // call read function in pdu/resp_*.h
				return{};
			}


			// normal code path: parse_header -> parse_body -> Callback
			template <class Callback>
			struct request_read_op
			{
				request_read_op(NextLayer& next_layer, std::span<uint8_t> buffer, pdu::pdu_req& request, Callback&& callback) :
					next_layer(next_layer),
					source(buffer),
					request(request),
					callback(callback)
				{
				}

				NextLayer& next_layer;
				adu::buffer source;
				pdu::pdu_req& request;
				Callback callback;
				uint16_t transaction_id = 0;
				uint16_t protocol = 0;
				uint16_t length = 0;
				uint8_t slave = 0;

				void parse_header(size_t size)
				{
					using pdu::read;

					// read MBAP
					std::tie(transaction_id, protocol, length, slave)
						= tcp::parse_header(source, size);

					// TODO: max length size is 254. (max adu size is 260. MBAP size is 7. "length" include "slave".)
					if (length + 6 > 255 || length <= 1)
					{
						callback(0, 0, make_error_code(modbus_exception_code::request_too_big));
						return;
					}
					mothbus::async_read(next_layer, source.prepare(length - 1), [op = std::move(*this)](auto ec, size_t read_size) mutable
					{
						if (!!ec) {
							op.callback(0, 0, ec);
							return;
						}
						op.parse_body(read_size);
					});
				}

				void parse_body(size_t size)
				{
					auto ec = tcp::parse_body(source, size, request);
					callback(transaction_id, slave, ec);
				}
			};


			template <class Callback>
			void async_read_request(pdu::pdu_req& request, Callback&& callback)
			{
				request_read_op<Callback> op(m_next_layer, m_message_buffer, request, std::forward<Callback>(callback));
				mothbus::async_read(m_next_layer, op.source.prepare(7), [op = std::move(op)](auto ec, size_t read_size) mutable
				{
					if (!!ec) {
						op.callback(0, 0, ec);
						return;
					}
					op.parse_header(read_size);
				});
			}

			template <class Resp>
			void write_response(uint16_t transaction_id, uint8_t slave, const Resp& response)
			{
				adu::buffer sink{ m_message_buffer };
				pdu::writer<adu::buffer> writer(sink);
				pdu::write(writer, transaction_id);
				pdu::write(writer, m_protocol);
				uint16_t length = 0;
				pdu::write(writer, length); // place holder
				pdu::write(writer, slave);

				pdu::write(writer, response); // call write function in pdu/resp_*.h

				const auto index = sink.output_start;
				length = static_cast<uint16_t>(sink.output_start - 6);
				sink.output_start = 4;
				pdu::write(writer, length); // actual length

				sink.output_start = index;
				sink.commit(index);
				boost::system::error_code ec;
				mothbus::write(m_next_layer, sink.data(), ec);
			}

		private:
			std::array<uint8_t, 320> m_message_buffer = { 0 };
			NextLayer& m_next_layer;
			uint16_t m_protocol = 0;
			uint16_t m_transaction_id = 0;
		};
	} // namespace tcp

	template<class Stream>
	using tcp_master = adu::master_base<tcp::stream<Stream>>;
} // namespace mothbus
