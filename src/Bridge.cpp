#include <Proxy.hpp>

namespace tcp_proxy {

	bridge::bridge(boost::asio::io_service& ios)
		: downstream_socket_(ios),
		upstream_socket_(ios)
	{}

	socket_type& bridge::downstream_socket()
	{
		return downstream_socket_;
	}

	socket_type& bridge::upstream_socket()
	{
		return upstream_socket_;
	}

	void bridge::start(const std::string& upstream_host, unsigned short upstream_port)
	{
		/* 서버에 연결하는 부분 */
		upstream_socket_.async_connect(
			ip::tcp::endpoint(
				ip::address::from_string(upstream_host),
				upstream_port
			),
			boost::bind(
				&bridge::handle_upstream_connect,
				shared_from_this(),
				boost::asio::placeholders::error
			)
		);
	}

	void bridge::handle_upstream_connect(const boost::system::error_code& error)
	{
		if (error)
		{
			std::cout << "handle_upstream_connect 에러: " << error.message() << std::endl;
			close();
			return;
		}

		std::cout << "Connected" << std::endl;

		upstream_socket_.async_read_some(
			boost::asio::buffer(upstream_data_, max_data_length),
			boost::bind(&bridge::handle_upstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);

		downstream_socket_.async_read_some(
			boost::asio::buffer(downstream_data_, max_data_length),
			boost::bind(&bridge::handle_downstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred
			)
		);
	}

	void bridge::handle_downstream_write(const boost::system::error_code& error)
	{
		if (error)
		{
			close();
			return;
		}

		upstream_socket_.async_read_some(
			boost::asio::buffer(upstream_data_, max_data_length),
			boost::bind(&bridge::handle_upstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void bridge::handle_downstream_read(const boost::system::error_code& error, const size_t& bytes_transferred)
	{
		if (error)
		{
			close();
			return;
		}

		async_write(upstream_socket_,
			boost::asio::buffer(downstream_data_, bytes_transferred),
			boost::bind(&bridge::handle_upstream_write,
				shared_from_this(),
				boost::asio::placeholders::error)
		);
	}

	void bridge::handle_upstream_write(const boost::system::error_code& error)
	{
		if (error)
		{
			close();
			return;
		}

		downstream_socket_.async_read_some(
			boost::asio::buffer(downstream_data_, max_data_length),
			boost::bind(&bridge::handle_downstream_read,
				shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void bridge::handle_upstream_read(const boost::system::error_code& error, const size_t& bytes_transferred)
	{
		if (error)
		{
			close();
			return;
		}

		std::cout << upstream_data_ << std::endl;
		async_write(downstream_socket_,
			boost::asio::buffer(upstream_data_, bytes_transferred),
			boost::bind(&bridge::handle_downstream_write,
				shared_from_this(),
				boost::asio::placeholders::error));
	}

	void bridge::close()
	{
		boost::mutex::scoped_lock lock(mutex_);

		if (downstream_socket_.is_open())
			downstream_socket_.close();

		if (upstream_socket_.is_open())
			upstream_socket_.close();
	}
}