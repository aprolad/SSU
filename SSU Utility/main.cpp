
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

#include <time.h>
#include <fstream>
using boost::asio::ip::tcp;

std::string buffer_to_string(boost::asio::streambuf *buffer, std::size_t bytesRead, std::string delimiter)
{
  std::string t{
        buffers_begin(buffer->data()),
        buffers_begin(buffer->data()) + bytesRead
          - delimiter.size()};

        buffer->consume(bytesRead);
        return t;
}
int main(int argc, char* argv[])
{
  try
  {
   // if (argc != 3)
   // {
   //   std::cerr << "Usage: blocking_tcp_echo_client <host> <port>\n";
   //   return 1;
  //  }

    boost::asio::io_service io_service;

    tcp::socket s(io_service);
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({"localhost", "1234"}));
    //while (true)
    {
    std::cout << "Started: ";
    boost::asio::streambuf response;

    sleep(1);
    while (true)
    {
        std::size_t bytesAvailable = s.available();
        boost::asio::write(s, boost::asio::buffer("1", 1));
        std::size_t bytesRead = 0;
        std::cout << "Bytes available to read: " << bytesAvailable << std::endl;

        const std::string end_com = "SSU_EOC";
        bytesRead = boost::asio::read_until(s, response, end_com);
        std::string command = buffer_to_string(&response, bytesRead, end_com);
        std::cout<<command<<std::endl;
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;

        const std::string delimiter = "SSU_FNE";
        bytesRead = boost::asio::read_until(s, response, delimiter);
        std::string filePath = buffer_to_string(&response, bytesRead, delimiter);
        std::cout<<filePath<<std::endl;
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
  
      

        
         std::ofstream outputFile(filePath, std::ios::binary);
         if (!outputFile)
        {
             std::cerr << "Failed to open output file: " << filePath << std::endl;
             return 0;
        }
        const std::string eof_del = "SSU_EOF";
      //boost::asio::write(s, boost::asio::buffer("1", 1));

        bytesRead = boost::asio::read_until(s, response, eof_del);


        outputFile<<buffer_to_string(&response, bytesRead, eof_del);
        outputFile.close();
       // std::cout << "Bytes written " << data.size() << std::endl;
        std::cout << "File received and saved as: " << filePath << std::endl;

        getchar();
    }
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}