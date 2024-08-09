/*!
 * \file		resp_writing_single_register.h
 * \brief		The Implementation of response of write_single_register(0x06).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		class write_single_register_pdu_resp : public pdu_base<function_code::write_single_register>
		{
		public:
			write_single_register_pdu_resp(uint16_t address, uint16_t value)
				: address(address)
				, value(value)
			{
			}

			uint16_t address{ 0 };
			uint16_t value{ 0 };
		};


		template <class Reader>
		error_code read(Reader& reader, write_single_register_pdu_resp& resp)
		{
			MOTH_CHECKED_RETURN(read(reader, resp.address));
			MOTH_CHECKED_RETURN(read(reader, resp.value));
			if (!(resp.value == 0x0000 || resp.value == 0xFF00)) {
                return make_error_code(modbus_exception_code::illegal_data_value);
            }

			return{};
		}

		template <class Writer>
		void write(Writer& writer, const write_single_register_pdu_resp& v)
		{
			write(writer, write_single_register_pdu_resp::fc);
			write(writer, v.address);
			write(writer, v.value);
		}
	} // namespace pdu
} // namespace mothbus
