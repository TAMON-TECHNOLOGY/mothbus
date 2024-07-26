#pragma once
#include "mothbus.h"
#include "../error.h"
#include "pdu/reader.h"
#include "pdu/writer.h"

namespace mothbus
{
	namespace pdu
	{
#define MOTH_CHECKED_RETURN(expr) { auto ec = expr; if (!!ec) return ec; }

		enum class function_code : uint8_t
		{
			read_coils = 0x01,
			read_discrete_inputs,
			read_holding_registers,
			read_input_registers,
			write_single_coil,
			write_single_register,
		};

		template <class Reader>
		error_code read(Reader& reader, function_code& v)
		{
			uint8_t h;
			MOTH_CHECKED_RETURN(read(reader, h));
			v = static_cast<function_code>(h);
			return{};
		}

		class read_coils_pdu_req;
		class read_coils_pdu_resp;
		class read_discrete_inputs_pdu_req;
		class read_discrete_inputs_pdu_resp;
		class read_holding_pdu_req;
		class read_holding_pdu_resp;
		class read_input_pdu_req;
		class read_input_pdu_resp;

		class write_single_coil_pdu_req;
		class write_single_coil_pdu_resp;
		class write_single_register_pdu_req;
		class write_single_register_pdu_resp;

		class not_implemented
		{
		public:
			uint8_t fc = 0;
		};

		template <function_code FunctionCode>
		class pdu_base
		{
		public:
			constexpr static function_code fc = FunctionCode;
		};

		// template<function_code FunctionCode>
		// constexpr function_code pdu_base<FunctionCode>::fc;

		using pdu_req = variant<read_holding_pdu_req, read_input_pdu_req, not_implemented>;


		namespace detail
		{
			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<std::is_same<Head, not_implemented>::value, error_code>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, [[maybe_unused]] function_code functionCode)
			{
				resp = not_implemented{};
				return make_error_code(modbus_exception_code::illegal_function);
			}

			template <class Head, class ...Tail, class Reader>
			typename std::enable_if<!std::is_same<Head, not_implemented>::value, error_code>::type
				read_pdu_variant(Reader& reader, pdu_req& resp, function_code functionCode)
			{
				if (Head::fc == functionCode)
				{
					Head real{};
					MOTH_CHECKED_RETURN(read(reader, real));
					resp = real;
					return{};
				}
				return read_pdu_variant<Tail...>(reader, resp, functionCode);
			}

			template <class Reader, class ...t>
			error_code read_pdu_req(Reader& reader, variant<t...>& resp, function_code functionCode)
			{
				return read_pdu_variant<t...>(reader, resp, functionCode);
			}
		} // namespace detail

		/*!
		 * \brief			read function code from reader.
		 * \param[in, out]	reader	reader.
		 * \param[out]		req		set value according to read function code.
		 *
		 * if there is no define type or set variant correspoinding to read function code,
		 * set req to \a not_implemented .
		 */
		template <class Reader>
		error_code read(Reader& reader, pdu_req& req)
		{
			function_code functionCode;
			MOTH_CHECKED_RETURN(read(reader, functionCode));
			return detail::read_pdu_req(reader, req, functionCode);
		}

		template <class Writer>
		void write(Writer& writer, const function_code& functionCode)
		{
			writer.write(static_cast<uint8_t>(functionCode));
		}


		class pdu_exception_resp
		{
		public:
			function_code fc;
			modbus_exception_code exceptionCode;
		};

		template <class Writer>
		void write(Writer& writer, const pdu_exception_resp& v)
		{
			uint8_t error_function_code = static_cast<uint8_t>(v.fc);
			error_function_code |= 0x80;
			write(writer, error_function_code);
			write(writer, v.exceptionCode);
		}


		template <class Resp>
		class pdu_resp
		{
		public:
			Resp& resp;
		};

		template <class Reader, class Response>
		error_code read(Reader& reader, pdu_resp<Response>& resp)
		{
			uint8_t fC;
			MOTH_CHECKED_RETURN(read(reader, fC));
			// 0x80 marks an modbus exception
			if (fC & 0x80) {
				pdu_exception_resp exc;
				exc.fc = static_cast<function_code>(fC & 0x7f);
				MOTH_CHECKED_RETURN(read(reader, exc.exceptionCode));
				return make_error_code(exc.exceptionCode);
			}

			const function_code function_code_value = static_cast<function_code>(fC);
			if (function_code_value != Response::fc) {
				return make_error_code(modbus_exception_code::invalid_response);
			}
			return read(reader, resp.resp); // call read function in pdu/resp_*.h
		}
	} // namespace pdu
} // namespace mothbus

#include "pdu/req_reading_coils.h"
#include "pdu/resp_reading_coils.h"
#include "pdu/req_reading_discrete_inputs.h"
#include "pdu/resp_reading_discrete_inputs.h"
#include "pdu/req_reading_register.h"
#include "pdu/resp_reading_register.h"
#include "pdu/req_reading_input_register.h"
#include "pdu/resp_reading_input_register.h"

#include "pdu/req_writing_single_coil.h"
#include "pdu/resp_writing_single_coil.h"
#include "pdu/req_writing_single_register.h"
#include "pdu/resp_writing_single_register.h"
