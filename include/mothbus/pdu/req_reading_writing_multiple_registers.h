/*!
 * \file		req_reading_writing_multiple_registers.h
 * \brief		The Implementation of request of read_write_multiple_registers(0x17).
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
		class read_write_multiple_registers_pdu_req : public pdu_base<function_code::read_write_multiple_registers>
		{
		public:
			uint16_t read_starting_address;
			uint16_t read_quantity_of_registers;
            uint16_t write_starting_address;
			uint16_t write_quantity_of_registers;
			uint8_t write_byte_count;
			std::vector<uint16_t> values; // native endian
		};


		template <class Reader>
		error_code read(Reader& reader, read_write_multiple_registers_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.read_starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.read_quantity_of_registers));
            MOTH_CHECKED_RETURN(read(reader, req.write_starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.write_quantity_of_registers));
			MOTH_CHECKED_RETURN(read(reader, req.write_byte_count));

			req.values.resize(req.write_byte_count / 2, 0);
			auto s = span{ req.values };
			MOTH_CHECKED_RETURN(read(reader, s));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const read_write_multiple_registers_pdu_req& v)
		{
			write(writer, read_write_multiple_registers_pdu_req::fc);
			writer.write(v.read_starting_address);
			writer.write(v.read_quantity_of_registers);
            writer.write(v.write_starting_address);
			writer.write(v.write_quantity_of_registers);
			writer.write(v.write_byte_count);
			for (const auto reg : v.values) { writer.write(reg); }
		}
	} // namespace pdu
} // namespace mothbus
