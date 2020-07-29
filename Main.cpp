#include <proxy.hpp>

int main(int argc, char* argv[])
{
   if (argc != 6)
   {
      std::cerr << "usage: tcpproxy_server <local host ip> <local port> <forward host ip> <forward port> <number of threads>" << std::endl;
      return 1;
   }

   const unsigned short local_port      = static_cast<unsigned short>(::atoi(argv[2]));
   const unsigned short forward_port    = static_cast<unsigned short>(::atoi(argv[4]));
   const std::string local_host         = argv[1];
   const std::string forward_host       = argv[3];
   const unsigned int number_of_threads = static_cast<unsigned int>(::atoi(argv[5]));

   boost::asio::io_service ios;

   try
   {
      tcp_proxy::acceptor acceptor(ios, local_host, local_port, forward_host, forward_port);
      acceptor.accept_connections();

      tcp_proxy::ios_thread_pool()
         .set_thread_count(number_of_threads)
         .run(ios);
   }
   catch(std::exception& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
   }

   return 0;
}