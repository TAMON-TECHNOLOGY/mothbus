/*!
 * \file		req_writing_multiple_registers.h
 * \brief		The Implementation of request of write_multiple_registers(0x10).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include <vector>
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class write_multiple_registers_pdu_req : public pdu_base<function_code::write_multiple_registers>
		{
		public:
			uint16_t starting_address;
			uint16_t quantity_of_registers;
			uint8_t byte_count;
			std::vector<uint16_t> values; // native endian
		};


		template <class Reader>
		error_code read(Reader& reader, write_multiple_registers_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_registers));
			MOTH_CHECKED_RETURN(read(reader, req.byte_count));

			req.values.resize(req.byte_count / 2, 0);
			auto s = span{ req.values };
			MOTH_CHECKED_RETURN(read(reader, s));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_multiple_registers_pdu_req& v)
		{
			write(writer, write_multiple_registers_pdu_req::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_registers);
			writer.write(v.byte_count);
			writer.write(v.values);
		}
	} // namespace pdu
} // namespace mothbus
