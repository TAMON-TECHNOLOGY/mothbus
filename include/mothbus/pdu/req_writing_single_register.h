/*!
 * \file		req_writing_single_register.h
 * \brief		The Implementation of request of write single register(0x06).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class write_single_register_pdu_req : public pdu_base<function_code::write_single_register>
		{
		public:
			uint16_t address{ 0 };
			uint16_t value{ 0 };
		};


		template <class Reader>
		error_code read(Reader& reader, write_single_register_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.address));
			MOTH_CHECKED_RETURN(read(reader, req.value));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_single_register_pdu_req& v)
		{
			write(writer, write_single_register_pdu_req::fc);
			writer.write(v.address);
			writer.write(v.value);
		}
	} // namespace pdu
} // namespace mothbus
