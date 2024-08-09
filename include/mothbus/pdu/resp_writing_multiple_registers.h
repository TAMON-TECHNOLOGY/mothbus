/*!
 * \file		resp_writing_multiple_registers.h
 * \brief		The Implementation of response of write_multiple_registers(0x10).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class write_multiple_registers_pdu_resp : public pdu_base<function_code::write_multiple_registers>
		{
		public:
			/*!
			 * \brief	ctor
			 * \param	starting_address	0x0000 to 0xFFFF
			 * \param	quantity_of_registers	0x0001 to 0x007B
			 */
			write_multiple_registers_pdu_resp(uint16_t starting_address, uint16_t quantity_of_registers)
				: starting_address(starting_address)
				, quantity_of_registers(quantity_of_registers)
			{
				assert(1 <= quantity_of_registers && quantity_of_registers <= 123);
			}

			uint16_t starting_address{ 0 };
			uint16_t quantity_of_registers{ 0 };
		};


		template <class Reader>
		error_code read(Reader& reader, write_multiple_registers_pdu_resp& resp)
		{
			MOTH_CHECKED_RETURN(read(reader, resp.starting_address));
			MOTH_CHECKED_RETURN(read(reader, resp.quantity_of_registers));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_multiple_registers_pdu_resp& v)
		{
			write(writer, write_multiple_registers_pdu_resp::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_registers);
		}
	} // namespace pdu
} // namespace mothbus
