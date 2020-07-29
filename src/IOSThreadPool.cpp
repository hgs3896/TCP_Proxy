#include <Proxy.hpp>

namespace tcp_proxy {
	ios_thread_pool::ios_thread_pool()
		: thread_count_(0)
	{}

	ios_thread_pool& ios_thread_pool::set_thread_count(const std::size_t thread_count)
	{
		thread_count_ = thread_count;
		return *this;
	}

	void ios_thread_pool::run(boost::asio::io_service& ios)
	{
		for (std::size_t i = 0; i < thread_count_; ++i)
		{
			thread_pool_.emplace_back(new boost::thread(boost::bind(&boost::asio::io_service::run, &ios)));
		}

		for (std::size_t i = 0; i < thread_pool_.size(); ++i)
		{
			thread_pool_[i]->join();
		}
	}
}