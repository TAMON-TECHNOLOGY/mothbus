/*!
 * \file		resp_writing_multiple_coils.h
 * \brief		The Implementation of response of write_multiple_coils(0x0F).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class write_multiple_coils_pdu_resp : public pdu_base<function_code::write_multiple_coils>
		{
		public:
			/*!
			 * \brief	ctor
			 * \param	starting_address	0x0000 to 0xFFFF
			 * \param	quantity_of_coils	0x0001 to 0x07B0
			 */
			write_multiple_coils_pdu_resp(uint16_t starting_address, uint16_t quantity_of_coils)
				: starting_address(starting_address)
				, quantity_of_coils(quantity_of_coils)
			{
				assert(1 <= quantity_of_coils && quantity_of_coils <= 1968);
			}

			uint16_t starting_address;
			uint16_t quantity_of_coils;
		};


		template <class Reader>
		error_code read(Reader& reader, write_multiple_coils_pdu_resp& resp)
		{
			MOTH_CHECKED_RETURN(read(reader, resp.starting_address));
			MOTH_CHECKED_RETURN(read(reader, resp.quantity_of_coils));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_multiple_coils_pdu_resp& v)
		{
			write(writer, write_multiple_coils_pdu_resp::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_coils);
		}
	} // namespace pdu
} // namespace mothbus
