//
// tcpproxy_server_v01.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2007 Arash Partow (http://www.partow.net)
// URL: http://www.partow.net/programming/tcpproxy/index.html
//
// Distributed under the Boost Software License, Version 1.0.
//
//
// Variation: Multi-threaded I/O service
// In this example, multiple threads will be instantiated and associated
// with the io_service. This will allow the io_service to handle bridges
// using seperate threads, allowing for the more efficient use of the
// processor's capabilities.
//
//

#pragma once

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <string>
#include <deque>

#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace tcp_proxy
{
	namespace ip = boost::asio::ip;

	class bridge;
	class acceptor;
	class ios_thread_pool;

	using socket_type = ip::tcp::socket;
	using ptr_type = boost::shared_ptr<bridge>;
}

class tcp_proxy::bridge : public boost::enable_shared_from_this<bridge>
{
private:
	socket_type downstream_socket_;
	socket_type upstream_socket_;
	enum { max_data_length = 8192 }; //8KB
	unsigned char downstream_data_[max_data_length];
	unsigned char upstream_data_[max_data_length];
	boost::mutex mutex_;

public:
	bridge(boost::asio::io_service& ios);
	socket_type& downstream_socket();
	socket_type& upstream_socket();
	void start(const std::string& upstream_host, unsigned short upstream_port);
	void handle_upstream_connect(const boost::system::error_code& error);

private:
	void handle_downstream_write(const boost::system::error_code& error);
	void handle_downstream_read(const boost::system::error_code& error, const size_t& bytes_transferred);
	void handle_upstream_write(const boost::system::error_code& error);
	void handle_upstream_read(const boost::system::error_code& error, const size_t& bytes_transferred);
	void close();
};

class tcp_proxy::acceptor
{
private:
	void handle_accept(const boost::system::error_code& error);
	boost::asio::io_service& io_service_;
	ip::address_v4 localhost_address;
	ip::tcp::acceptor acceptor_;
	ptr_type session_;
	unsigned short upstream_port_;
	std::string upstream_host_;

public:
	acceptor(boost::asio::io_service& io_service,
		const std::string& local_host, unsigned short local_port,
		const std::string& upstream_host, unsigned short upstream_port);
	bool accept_connections();
};

class tcp_proxy::ios_thread_pool
{
public:
	using thread_t = boost::shared_ptr<boost::thread>;
private:
	std::size_t thread_count_;
	std::deque<thread_t> thread_pool_;

public:	
	ios_thread_pool();
	ios_thread_pool& set_thread_count(const std::size_t thread_count);
	void run(boost::asio::io_service& ios);
};