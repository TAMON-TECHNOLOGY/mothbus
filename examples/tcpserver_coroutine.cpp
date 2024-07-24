/*!
 * Copyright shousen
 * Released under the MIT license
 *
 * Date: 2024-07-24
 */
#include <iostream>
#include <memory>
#include <mothbus/adu/tcp.h>

using boost::asio::ip::tcp;


namespace {
	template<typename FunctionCode>
	void write_exception_response(mothbus::tcp::stream<tcp::socket>& stream, FunctionCode fc, mothbus::modbus_exception_code ec, std::uint16_t transactionId, std::uint8_t unit_id)
	{
		mothbus::pdu::pdu_exception_resp resp;
		resp.fc = static_cast<mothbus::pdu::function_code>(fc);
		resp.exceptionCode = ec;
		stream.write_response(transactionId, unit_id, resp);
	}


	template <class Stream>
	struct reqest_handler
	{
		mothbus::tcp::stream<Stream>& stream;
		uint16_t transactionId;
		uint8_t unitID;

		reqest_handler(mothbus::tcp::stream<Stream>& stream, uint16_t transactionId, uint8_t unitID) :
			stream(stream), transactionId(transactionId), unitID(unitID)
		{}

		void operator()(auto& req)
		{
            std::cout << "not implemented function code received:\n";
			std::cout << "unitID: " << (int)unitID << "\n";
			std::cout << "functionCode " << static_cast<int>(req.fc) << "\n";

			write_exception_response(stream, req.fc, mothbus::modbus_exception_code::illegal_function, transactionId, unitID);
		}

		void operator()(mothbus::pdu::read_holding_pdu_req& req)
		{
			std::cout << "read holding register request received:\n";
			std::cout << "unitID: " << (int)unitID << "\n";
			std::cout << "register address: " << req.starting_address << "\n";

			// check quantity of registers
			if (!(1 <= req.quantity_of_registers && req.quantity_of_registers <= 125)) {
				write_exception_response(stream, req.fc, mothbus::modbus_exception_code::illegal_data_value, transactionId, unitID);
				return;
			}

			std::vector<mothbus::byte> regs(req.quantity_of_registers * 2, mothbus::byte(0));
            mothbus::pdu::read_holding_pdu_resp resp(regs);
            stream.write_response(transactionId, unitID, resp);
            return;
		}
	};

	boost::asio::awaitable<void> handle_message(tcp::socket socket)
	{
		mothbus::tcp::stream<tcp::socket> mothbus_stream{ socket };

		std::array<std::uint8_t, 320> message_buffer;
		mothbus::adu::buffer source(message_buffer);

		mothbus::pdu::pdu_req request;
		for (;;)
		{
			// parse header
			std::size_t read_size = co_await boost::asio::async_read(socket, source.prepare(7), boost::asio::use_awaitable);
			const auto [transactionId, protocol, messageLength, unit_id] = mothbus::tcp::parse_header(source, read_size);
			// PDU max size is 253, min size is 1. messageLength is 1(unit_id) + PDU.
			if (!((1 + 1 + 0) <= messageLength) && (messageLength <= (1 + 253 + 0))) {
				write_exception_response(
					mothbus_stream,
					0,
					mothbus::modbus_exception_code::request_too_big,
					transactionId,
					unit_id
				);
				continue;
			}


			// parse body
			read_size = co_await boost::asio::async_read(socket, source.prepare(messageLength - 1), boost::asio::use_awaitable);
			const auto ec = mothbus::tcp::parse_body(source, read_size, request);
			if (!!ec) {
				std::cerr << ec;
				co_return;
			}


			// do action corresponding to request
			reqest_handler<tcp::socket> handler{ mothbus_stream, transactionId, unit_id };
			std::visit(handler, request);
		} // for (;;)
	}


	boost::asio::awaitable<void> listener()
	{
		auto executor = co_await boost::asio::this_coro::executor;
		tcp::acceptor acceptor(executor, { tcp::v4(), 502 });
		for (;;)
		{
			tcp::socket socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
			boost::asio::co_spawn(executor,
				[socket = std::move(socket)]() mutable
				{
					return handle_message(std::move(socket));
				},
				boost::asio::detached);
		} // for (;;)
	}
}


int main()
{
	try {
		boost::asio::io_context io_context(1);

		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&](auto, auto) { io_context.stop(); });

		boost::asio::co_spawn(io_context, listener, boost::asio::detached);

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

    return 0;
}
