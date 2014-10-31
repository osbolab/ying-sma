#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <iostream>

boost::asio::io_service service;

boost::asio::ip::tcp::endpoint ep( boost::asio::ip::address::from_string("127.0.0.1"), 8001);

size_t read_complete( char* buf, const boost::system::error_code & err, size_t bytes)
{
  if (err)  return 0;
  bool found = std::find(buf, buf+bytes, '\n') < buf + bytes;
  return found ? 0 : 1;
}

void sync_echo(std::string msg)
{
  msg += "\n";
  boost::asio::ip::tcp::socket sock(service);
  sock.connect(ep);
  sock.write_some(boost::asio::buffer(msg));
  char buf[1024];
  int bytes = read(sock, boost::asio::buffer(buf), boost::bind(read_complete, buf, _1, _2));
  std::string copy(buf, bytes-1);
  msg = msg.substr(0, msg.size()-1);
  std::cout << "server echoed our " << msg << ": "
            << (copy == msg ? "OK" : "FAIL") << std::endl;
  sock.close();
}

int main (int argc, char* argv[])
{
  char* messages[] = { "John says hi",
                       "So does James",
                       "Lucy just got home",
                       "Boost.Asio is Fun!",
                        0
                     };

  boost::thread_group threads;
  for (char** message = messages; *message; ++message)
  {
    threads.create_thread( boost::bind(sync_echo, std::string(*message)));
    boost::this_thread::sleep( boost::posix_time::millisec(100));
  }

  threads.join_all();
}
