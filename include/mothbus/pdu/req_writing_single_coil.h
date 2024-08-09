/*!
 * \file		req_writing_single_coil.h
 * \brief		The Implementation of request of write single coil(0x05).
 *
 * Copyright (c) shousen. Released under the MIT license.
 */
#pragma once
#include "../pdu.h"


namespace mothbus
{
	namespace pdu
	{
		class write_single_coil_pdu_req : public pdu_base<function_code::write_single_coil>
		{
		public:
			uint16_t address{ 0 };
			uint16_t value{ 0 };     // 0x0000 or 0xFF00
		};


		template <class Reader>
		error_code read(Reader& reader, write_single_coil_pdu_req& req)
		{
			MOTH_CHECKED_RETURN(read(reader, req.address));
			MOTH_CHECKED_RETURN(read(reader, req.value));
			return{};
		}


		template <class Writer>
		void write(Writer& writer, const write_single_coil_pdu_req& v)
		{
			assert(v.value == 0x0000 || v.value == 0xFF00);

			write(writer, write_single_coil_pdu_req::fc);
			writer.write(v.address);
			writer.write(v.value);
		}
	} // namespace pdu
} // namespace mothbus
