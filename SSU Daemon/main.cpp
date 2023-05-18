#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <fstream>
#include <string>
#include <filesystem>
using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket) : socket_(std::move(socket))
  {}

  void start()
  {

    std::string path = "./tet";
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_regular_file())
        {
          std::cout<<std::string(entry.path())<<std::endl;
            file_paths.push_back(std::string(entry.path()));
            
            file_pathsw.push_back(std::string(entry.path())+ "SSU_FNE");
        }
    }

    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
           // std::cout<<data_<<std::endl;
            do_write(length);
          }
        });
  }
  bool end;
  void do_write(std::size_t length)
  {
    std::string command;
    if (!end)
      command = "CRF";
    else
      command = "END";
    boost::asio::write(socket_, boost::asio::buffer(command));
    std::string end_com("SSU_EOC");
    boost::asio::write(socket_, boost::asio::buffer(end_com));
       
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(file_pathsw[file_pointer].data(), file_pathsw[file_pointer].length()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {



            std::string filePath = file_paths[file_pointer];
            std::cout<<file_pointer<<filePath<<std::endl;  
            std::ifstream file(filePath, std::ios::binary);
            if (!file)
            {
                std::cerr << "Failed to open file: " << filePath << std::endl;
                return;
            }
            // Read file data into a buffer
          std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
          file.close();

    

            boost::asio::write(socket_, boost::asio::buffer(buffer.data(), buffer.size()));
            std::string end_seq("SSU_EOF");
            boost::asio::write(socket_, boost::asio::buffer(end_seq));

            file_pointer++;
            if (file_pointer > file_paths.size() - 1)
              end = true;
            do_read();
          }
        });

        
  }
  uint32_t file_pointer;
  std::vector<std::string> file_paths;
  std::vector<std::string> file_pathsw;
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};

class server
{
public:
  server(boost::asio::io_service& io_service, short port) : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), socket_(io_service)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket_))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

int main(int argc, char* argv[])
{
  try
  {
   // if (argc != 2)
  //  {
  //    std::cerr << "Usage: async_tcp_echo_server <port>\n";
  //    return 1;
  //  }

    boost::asio::io_service io_service;

    server s(io_service, 1234);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
