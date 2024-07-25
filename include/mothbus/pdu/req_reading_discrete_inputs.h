/*!
 * \file		req_reading_discrete_inputs.h
 * \brief		The Implementation of request of read_discrete_inputs(0x02).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class read_discrete_inputs_pdu_req : public pdu_base<function_code::read_discrete_inputs>
		{
		public:
			uint16_t starting_address;
			uint16_t quantity_of_discrete_inputs;
		};


		template <class Reader>
		error_code read(Reader& reader, read_discrete_inputs_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_discrete_inputs));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const read_discrete_inputs_pdu_req& v)
		{
			write(writer, read_discrete_inputs_pdu_req::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_discrete_inputs);
		}
	} // namespace pdu
} // namespace mothbus
