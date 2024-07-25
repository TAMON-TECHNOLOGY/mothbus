/*!
 * \file		req_reading_coils.h
 * \brief		The Implementation of request of read_coils(0x01).
 *
 * Copyright (c) shousen. Released under the MIT license. Date: 2024-07-24
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class read_coils_pdu_req : public pdu_base<function_code::read_coils>
		{
		public:
			uint16_t starting_address;
			uint16_t quantity_of_coils;
		};


		template <class Reader>
		error_code read(Reader& reader, read_coils_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_coils));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const read_coils_pdu_req& v)
		{
			write(writer, read_coils_pdu_req::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_coils);
		}
	} // namespace pdu
} // namespace mothbus