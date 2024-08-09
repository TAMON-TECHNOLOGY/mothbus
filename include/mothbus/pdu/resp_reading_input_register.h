/*!
 * \file		resp_reading_input_register.h
 * \brief		The Implementation of responese of read_input_registers(0x04).
 */
#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		class read_input_pdu_resp : public pdu_base<function_code::read_input_registers>
		{
		public:
			read_input_pdu_resp(span<byte> v)
				: values(v),
				  byte_count(static_cast<uint8_t>(v.size()))
			{
			}

			uint8_t byte_count{ 0 };
			span<byte> values;
		};


		template <class Reader>
		error_code read(Reader& reader, read_input_pdu_resp& resp)
		{
			MOTH_CHECKED_RETURN(read(reader, resp.byte_count));
			if (resp.byte_count > resp.values.size()) {
				return make_error_code(modbus_exception_code::too_many_bytes_received);
			}
			if (resp.byte_count < resp.values.size()) {
				resp.values = resp.values.subspan(0, resp.byte_count);
			}
			MOTH_CHECKED_RETURN(read(reader, resp.values));
			return{};
		}

		template <class Writer>
		void write(Writer& writer, const read_input_pdu_resp& v)
		{
			write(writer, read_input_pdu_resp::fc);
			write(writer, v.byte_count);
			write(writer, v.values);
		}
	} // namespace pdu
} // namespace mothbus
