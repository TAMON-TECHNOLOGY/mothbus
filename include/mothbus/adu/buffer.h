#pragma once
#include <array>
#include <boost/asio/buffer.hpp>

namespace mothbus
{
	namespace adu
	{
		class buffer
		{
		public:
			buffer(std::span<uint8_t> b)
				: message_buffer(b)
			{
			}
			template<std::size_t N>
			buffer(std::array<uint8_t, N>& b)
				: message_buffer(std::span<uint8_t>{ b.begin(), b.size() })
			{
			}

			std::span<uint8_t> message_buffer;

			void put(uint8_t v)
			{
				message_buffer[output_start++] = v;
			}

			error_code get(uint8_t& out)
			{
				if (input_start >= input_end) {
					return make_error_code(modbus_exception_code::too_many_bytes_received);
				}

				out = message_buffer[input_start];
				input_start++;
				return{};
			}

			boost::asio::const_buffers_1 data()
			{
				return boost::asio::const_buffers_1(boost::asio::const_buffer(message_buffer.data() + input_start, input_end - input_start));
			}

			boost::asio::mutable_buffers_1 prepare(size_t n)
			{
				return boost::asio::mutable_buffers_1(boost::asio::mutable_buffer(message_buffer.data() + output_start, n));
			}

			void commit(size_t n)
			{
				input_end += n;
				output_start += n;
			}

			void consume(size_t n)
			{
				input_start += n;
			}

			size_t input_start = 0;
			size_t input_end = 0;
			size_t output_start = 0;
		};

		inline error_code read(buffer& in, uint8_t& out)
		{
			return in.get(out);
		}
	} // namespace adu
} // namespace mothbus
