/*!
 * \file		resp_reading_discrete_inputs.h
 * \brief		The Implementation of response of read_discrete_inputs(0x02).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		class read_discrete_inputs_pdu_resp : public pdu_base<function_code::read_discrete_inputs>
		{
		public:
			read_discrete_inputs_pdu_resp(span<byte> v)
				: values(v)
				, byte_count(static_cast<uint8_t>(v.size()))
			{
			}

			uint8_t byte_count{ 0 };
			span<byte> values;
		};


		template <class Reader>
		error_code read(Reader& reader, read_discrete_inputs_pdu_resp& resp)
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
		void write(Writer& writer, const read_discrete_inputs_pdu_resp& v)
		{
			write(writer, read_discrete_inputs_pdu_resp::fc);
			write(writer, v.byte_count);
			write(writer, v.values);
		}
	} // namespace pdu
} // namespace mothbus
