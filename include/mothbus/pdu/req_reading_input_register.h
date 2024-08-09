/*!
 * \file		req_reading_input_register.h
 * \brief		The Implementation of request of read_input_registers(0x04).
 */
#pragma once
#include "../pdu.h"

namespace mothbus
{
	namespace pdu
	{
		class read_input_pdu_req : public pdu_base<function_code::read_input_registers>
		{
		public:
			uint16_t starting_address{ 0 };
			uint16_t quantity_of_registers{ 0 };
		};


		template <class Reader>
		error_code read(Reader& reader, read_input_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_registers));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const read_input_pdu_req& v)
		{
			write(writer, read_input_pdu_req::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_registers);
		}
	} // namespace pdu
} // namespace mothbus
