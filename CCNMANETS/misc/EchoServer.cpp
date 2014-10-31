#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>

boost::asio::io_service service;

size_t read_complete(char* buff, const boost::system::error_code & err, size_t bytes)
{
  if (err)    return 0;
  bool found = std::find(buff, buff+bytes, '\n') < buff + bytes;
  return found ? 0 : 1;
}

void handle_connections()
{
  boost::asio::ip::tcp::acceptor acceptor(service, 
                                          boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8001));
  char buff[1024];
  while ( true)
  {
    boost::asio::ip::tcp::socket sock( service);
    acceptor.accept(sock);
    int bytes= read(sock, boost::asio::buffer(buff), boost::bind(read_complete, buff, _1, _2));
    std::string msg(buff, bytes);
    sock.write_some(boost::asio::buffer(msg));
    sock.close();
  }
}

int main (int argc, char* argv[])
{
  handle_connections();
}
