/*!
 * \file		req_writing_multiple_coils.h
 * \brief		The Implementation of request of write_multiple_coils(0x0F).
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
		class write_multiple_coils_pdu_req : public pdu_base<function_code::write_multiple_coils>
		{
		public:
			uint16_t starting_address{ 0 };
			uint16_t quantity_of_coils{ 0 };
			uint8_t byte_count{ 0 };
			std::vector<byte> values;
		};


		template <class Reader>
		error_code read(Reader& reader, write_multiple_coils_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.starting_address));
			MOTH_CHECKED_RETURN(read(reader, req.quantity_of_coils));
			MOTH_CHECKED_RETURN(read(reader, req.byte_count));

			req.values.resize(req.byte_count, byte{ 0 });
			auto s = span{ req.values };
			MOTH_CHECKED_RETURN(read(reader, s));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_multiple_coils_pdu_req& v)
		{
			write(writer, write_multiple_coils_pdu_req::fc);
			writer.write(v.starting_address);
			writer.write(v.quantity_of_coils);
			writer.write(v.byte_count);
			for (auto b : v.values) {
				writer.write(b);
			}
		}
	} // namespace pdu
} // namespace mothbus
