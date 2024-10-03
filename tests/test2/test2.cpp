
#include <boost/asio.hpp>
#include <iostream>
#include "ucoro/awaitable.hpp"

boost::asio::io_context main_ioc;

ucoro::awaitable<int> coro_compute_int(int value)
{
	auto ret = co_await executor_awaitable<int>([value](auto handle) {
		main_ioc.post([value, handle = std::move(handle)]() mutable {
			std::this_thread::sleep_for(std::chrono::seconds(0));
			std::cout << value << " value\n";
			handle(value * 100);
		});
	});


	std::coroutine_handle<> this_coro_handle = co_await ucoro::this_coro_handle;

	std::cout << this_coro_handle.address() << " this_coro_handle\n";

	main_ioc.post([&ret, value, this_coro_handle]() mutable {
		std::this_thread::sleep_for(std::chrono::seconds(0));
		std::cout << value << " value\n";
		//handle(value * 100);
		ret = value * 100;
		this_coro_handle.resume();
	});

	co_await ucoro::suspend;

	co_return (value + ret);
}

ucoro::awaitable<void> coro_compute_exec(int value)
{
	auto ret = co_await coro_compute_int(value);
	std::cout << "return: " << ret << std::endl;
	co_return;
}

ucoro::awaitable<void> coro_compute()
{
	auto x = co_await ucoro::local_storage;
	std::cout << "local storage: " << std::any_cast<std::string>(x) << std::endl;

	for (auto i = 0; i < 100; i++)
	{
		co_await coro_compute_exec(i);
	}
}

int main(int argc, char **argv)
{
	std::string str = "hello";

	coro_start(coro_compute(), str);

	main_ioc.run();

	return 0;
}
