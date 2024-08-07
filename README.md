# mothbus, a c++17 library for the modbus protocol

Original code is [here](https://github.com/ChrisBFX/mothbus).

## Introduction

The goal of the mothbus library is to provide a complete and simple implementation of the 
modbus protocol as defined on modbus.org. It provides role agnostic intefaces for modbus-tcp 
and modbus-rtu, which means you can use the same code for server (called slaves in modbus context)
and clients (called master), as well a simple client interface.
The design goals (which set it apart of other modbus implementations) are:

* **Scalability.** Mothbus scales from tiny microcontrollers to the cloud.

* **Network layer agnostic.**  Mothbus does not implement the network layer, which allows it to 
be ported to almost everything. As it is heavily influenced by boost::asio it works best with it.

* **Async api.** A simple, based on boost::asio, asynchronous api (if your network layer support it).

* **Low memory footprint** Dynamic memory allocations are kept to a minimum.

## Status

The library is currently in a proof-of-concept state and while it maybe already works, there are many
unaddressed issues and internal inconsistencies.

## Requirements

* C++17
* boost (asio, system (program_options for examples))
* google-test for unit tests


## To Build

Mothbus is a header only library, so simple add the include path to your project.
For tests and examples you can use cmake.

## Example

Reading a register from a modbus tcp device:
```C++
#include <iostream>
#include <boost/asio.hpp>
#include <mothbus/adu/tcp.h>

int main(int argc, char** argv)
{	
	// network code
	using boost::asio::ip::tcp;
	std::string host = "localhost";	
	boost::asio::io_service io_service;
	tcp::resolver resolver(io_service);
	tcp::socket socket(io_service);
	boost::asio::connect(socket, resolver.resolve(tcp::resolver::query{host, "502"}));

	// mothbus reads from server
	mothbus::tcp_master<tcp::socket> client(socket);
	std::array<mothbus::byte, 2> singleRegister;
	client.read_registers(slave, register_address, singleRegister);
	
	// output value
	uint16_t value = (std::to_integer<uin16_t>(singleRegister[0]) << 8) + std::to_integer<uin16_t>(singleRegister[0]);
	std::cout << value;	
	return 0;
}
```

modbus tcp slave example with C++20 coroutine:
```C++
#include <iostream>
#include <memory>
#include <mothbus/adu/tcp.h>

using boost::asio::ip::tcp;


template <class Stream>
struct reqest_handler
{
	mothbus::tcp::stream<Stream>& stream;
	uint16_t transactionId;
	uint8_t slave;

	reqest_handler(mothbus::tcp::stream<Stream>& stream, uint16_t transactionId, uint8_t slave) :
		stream(stream), transactionId(transactionId), slave(slave)
	{}


	void operator()(auto& req)
	{
		std::cout << "not implemented handler. fc = " << static_cast<int>(req.fc) << ".\n";
		write_exception_response(stream, req.fc, mothbus::modbus_exception_code::illegal_function, transactionId, slave);
	}

	void operator()(mothbus::pdu::read_holding_pdu_req& req)
	{
		std::cout << "read holding register request received:\n";
		std::cout << "slave: " << (int)slave << "\n";
		std::cout << "register address: " << req.starting_address << "\n";

		// check quantity of registers
		if (!(1 <= req.quantity_of_registers && req.quantity_of_registers <= 125)) {
			mothbus::pdu::pdu_exception_resp resp;
			resp.fc = req.fc;
			resp.exceptionCode = mothbus::modbus_exception_code::illegal_data_value;
			stream.write_response(transactionId, slave, resp);
			return;
		}

		std::vector<mothbus::byte> regs(req.quantity_of_registers * 2, mothbus::byte(0));
		mothbus::pdu::read_holding_pdu_resp resp(regs);
		stream.write_response(transactionId, slave, resp);
		return;
	}
};

boost::asio::awaitable<void> handle_message(tcp::socket socket)
{
	mothbus::tcp::stream<tcp::socket> mothbus_stream{ socket };

	std::vector<std::uint8_t> message_buffer(320);
	mothbus::adu::buffer source(message_buffer);

	mothbus::pdu::pdu_req request;
	for (;;)
	{
		// parse header
		std::size_t read_size = co_await boost::asio::async_read(socket, source.prepare(7), boost::asio::use_awaitable);
		const auto [transactionId, protocol, messageLength, unit_id] = mothbus::tcp::parse_header(source, read_size);
		// PDU max size is 253, min size is 1. messageLength is 1(unit_id) + PDU.
		if (!((1 + 1 + 0) <= messageLength) && (messageLength <= (1 + 253 + 0))) {
			mothbus::pdu::pdu_exception_resp resp;
			resp.fc = static_cast<mothbus::pdu::function_code>(0);
			resp.exceptionCode = mothbus::modbus_exception_code::request_too_big;
			stream.write_response(transactionId, unit_id, resp);
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
			[socket = std::move(socket)]() mutable {
				return handle_message(std::move(socket));
			},
			boost::asio::detached);
	} // for (;;)
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
```
