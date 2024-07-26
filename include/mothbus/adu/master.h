#pragma once
#include "../mothbus.h"
#include "../pdu.h"
#include <vector>

namespace mothbus
{
	namespace adu
	{
		namespace detail
		{
			// convert msb data to native uint16 data.
			constexpr inline std::uint16_t make_uint16(byte b1, byte b2) noexcept
			{
				return (std::to_integer<std::uint16_t>(b1) << 8u) + std::to_integer<std::uint16_t>(b2);
			}
		} // namespace detail

		/**
		 * Base for high level functions used when acting as a modbus master (aka client)
		 * Stream has to be tcp::stream or rtu::stream
		 */
		template <class Stream>
		class master_base
		{
		public:
			using stream_next_layer_type = typename Stream::next_layer_type;
			master_base(stream_next_layer_type& next_layer)
				: m_stream(next_layer)
			{}

			// TODO: is need span<byte> out interface?

			/*!
			 * \brief			read coils function. (0x01)
			 * \param			slave		slave(or unit id)
			 * \param			address		start (pdu) address. 0x0000 to 0xFFFF.
			 * \param			quantity	quantity of coils. 1 to 2000.
			 * \param[out]		out			coil status. (quantity - 1) / 8 + 1 <= out.size().
			 *
			 * The LSB of the first data byte contains the output addressed in the query.
			 */
			error_code read_coils(uint8_t slave, uint16_t address, uint16_t quantity, span<byte> out)
			{
				assert(1 <= quantity && quantity <= 2000);
				assert((quantity - 1) / 8 + 1 <= out.size());

				pdu::read_coils_pdu_req req;
				req.starting_address = address;
				req.quantity_of_coils = quantity; 
				const auto transaction_id = m_stream.write_request(slave, req);

				pdu::read_coils_pdu_resp resp;
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				if (resp.byte_count != out.size()) {
					return make_error_code(modbus_exception_code::invalid_response);
				}
				return{};
			}
			/*!
			 * \brief			write single coil function. (0x05)
			 * \param			slave		slave(or unit id)
			 * \param			address		write (pdu) address. 0x0000 to 0xFFFF.
			 * \param			value		write value
			 */
			error_code write_single_coil(uint8_t slave, uint16_t address, bool value)
			{
				pdu::write_single_coil_pdu_req req;
				req.address = address;
				req.value = value ? 0xFF00 : 0x0000; 
				const auto transaction_id = m_stream.write_request(slave, req);

				pdu::write_single_coil_pdu_resp resp;
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				return{};
			}

			/*!
			 * \brief			read discrete inputs function. (0x02)
			 * \param			slave		slave(or unit id)
			 * \param			address		start (pdu) address. 0x0000 to 0xFFFF.
			 * \param			quantity	quantity of discrete inputs. 1 to 2000.
			 * \param[out]		out			coil status. (quantity - 1) / 8 + 1 <= out.size().
			 *
			 * The LSB of the first data byte contains the output addressed in the query.
			 */
			error_code read_discrete_inputs(uint8_t slave, uint16_t address, uint16_t quantity, span<byte> out)
			{
				assert(1 <= quantity && quantity <= 2000);
				assert((quantity - 1) / 8 + 1 <= out.size());

				pdu::read_discrete_inputs_pdu_req req;
				req.starting_address = address;
				req.quantity_of_discrete_inputs = quantity;
				const auto transaction_id = m_stream.write_request(slave, req);

				pdu::read_discrete_inputs_pdu_resp resp;
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				if (resp.byte_count != out.size()) {
					return make_error_code(modbus_exception_code::invalid_response);
				}
				return{};
			}

			// 0x03
			error_code read_registers(uint8_t slave, uint16_t address, span<byte> out)
			{
				pdu::read_holding_pdu_req req;
				req.starting_address = address;
				req.quantity_of_registers = static_cast<uint16_t>(out.size() / 2);
				const auto transaction_id = m_stream.write_request(slave, req);
				
				pdu::read_holding_pdu_resp resp(out);
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				if (resp.byte_count != out.size()) {
					return make_error_code(modbus_exception_code::invalid_response);
				}
				return{};
			}
			error_code read_registers(uint8_t slave, uint16_t address, span<std::uint16_t> out)
			{
				std::vector<byte> buffer(out.size() * 2);
				auto ec = read_registers(slave, address, buffer);
				if (!ec) {
					out = out.subspan(0, buffer.size() / 2);
					for (size_t i = 0; i < out.size(); ++i) {
						out[i] = detail::make_uint16(buffer[2 * i + 0], buffer[2 * i + 1]);
					}
				}
				return ec;
			}
			/*!
			 * \brief			write single register function. (0x06)
			 * \param			slave		slave(or unit id)
			 * \param			address		write (pdu) address. 0x0000 to 0xFFFF.
			 * \param			value		write value
			 */
			error_code write_single_register(uint8_t slave, uint16_t address, uint16_t value)
			{
				pdu::write_single_register_pdu_req req;
				req.address = address;
				req.value = value;
				const auto transaction_id = m_stream.write_request(slave, req);

				pdu::write_single_register_pdu_resp resp;
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				return{};
			}

			// 0x04
			error_code read_input_registers(uint8_t slave, uint16_t address, span<byte> out)
			{
				pdu::read_input_pdu_req req;
				req.starting_address = address;
				req.quantity_of_registers = static_cast<uint16_t>(out.size() / 2);
				const auto transaction_id = m_stream.write_request(slave, req);

				pdu::read_input_pdu_resp resp(out);
				auto ec = m_stream.read_response(transaction_id, slave, resp);
				if (!!ec) {
					return ec;
				}
				if (resp.byte_count != out.size()) {
					return make_error_code(modbus_exception_code::invalid_response);
				}
				return{};
			}
			error_code read_input_registers(uint8_t slave, uint16_t address, span<std::uint16_t> out)
			{
				std::vector<byte> buffer(out.size() * 2);
				auto ec = read_input_registers(slave, address, buffer);
				if (!ec) {
					out = out.subspan(0, buffer.size() / 2);
					for (size_t i = 0; i < out.size(); ++i) {
						out[i] = detail::make_uint16(buffer[2 * i + 0], buffer[2 * i + 1]);
					}
				}
				return ec;
			}

		private:
			Stream m_stream;
		};		
	} // namespace adu
} // namespace mothbus
