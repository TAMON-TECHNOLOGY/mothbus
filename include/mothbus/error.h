#pragma once
#include <type_traits>

#define MOTHBUS_USE_BOOST_ERROR


#ifdef MOTHBUS_USE_BOOST_ERROR
#include <boost/system/system_error.hpp>
#else
#include <system_error>
#endif

namespace mothbus
{
#ifdef MOTHBUS_USE_BOOST_ERROR
	using error_code = boost::system::error_code;
	using error_category = boost::system::error_category;
#else
	using error_code = std::error_code;
	using error_category = std::error_category;
#endif



	enum class modbus_exception_code
	{
		success = 0,
		illegal_function = 0x01,
		illegal_data_address = 0x02,
		illegal_data_value = 0x03,
		slave_device_failure = 0x04,
		acknowledge = 0x05,
		slave_device_busy = 0x06,
		negative_acknowledge = 0x07,
		memory_parity_error = 0x08,
		gateway_path_unavailable = 0x0a,
		gateway_target_device_failed_to_respond = 0x0b,
		invalid_response,
		too_many_bytes_received,
		transaction_id_invalid,
		illegal_protocol,
		slave_id_invalid,
		request_too_big
	};



	struct ModbusErrorCategory : error_category
	{
		const char* name() const noexcept override final
		{
			return "modbus error";
		}

		std::string message(int ev) const override final
		{
			switch (static_cast<modbus_exception_code>(ev))
			{
			using enum modbus_exception_code;
			case success:					return "no error";
			case illegal_function:			return "The function code received in the request is not an authorized action for the slave.";
			case illegal_data_address:		return "The data address received by the slave is not an authorized address for the slave.";
			case illegal_data_value:		return "The value in the request data field is not an authorized value for the slave.";
			case slave_device_failure:		return "The slave fails to perform a requested action because of an unrecoverable error.";
			case acknowledge:				return "The slave accepts the request but needs a long time to process it.";
			case slave_device_busy:			return "The slave is busy processing another command.";
			case negative_acknowledge:		return "The slave cannot perform the programming request sent by the master.";
			case memory_parity_error:		return "The slave detects a parity error in the memory when attempting to read extended memory.";
			case gateway_path_unavailable:	return "The gateway is overloaded or not correctly configured.";
			case gateway_target_device_failed_to_respond:	return "The slave is not present on the network.";

			default:
				return "(unrecognized error)";
			}
		}
	};

	inline const ModbusErrorCategory& modbus_category() noexcept
	{
		static ModbusErrorCategory category;
		return category;
	};

	inline error_code make_error_code(modbus_exception_code c)
	{
		return error_code(static_cast<int>(c), modbus_category());
	}
} // namespace mothbus

#ifdef MOTHBUS_USE_BOOST_ERROR
namespace boost {
	namespace system {

		template<> struct is_error_code_enum<mothbus::modbus_exception_code>
		{
			static const bool value = true;
		};
	}
}
#else
namespace std
{
	template <>
	struct is_error_code_enum<mothbus::modbus_exception_code> : std::true_type {};
}
#endif
