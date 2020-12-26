#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using nlohmann::json;

int main(int argc, char* argv[]) {
  try
  {
    // Check command line arguments.
    if(argc != 5)
    {
      throw std::runtime_error("Incorrect number of args");
    }
    auto const host = argv[1];
    auto const port = argv[2];
    auto const target = argv[3];
    auto const input = argv[4];
    int version = 11;
    // The io_context is required for all I/O
    net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);
    json send_data;
    send_data["input"] = input;

    // Set up an HTTP GET request message

    http::request<http::string_body> req{http::verb::post, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.body() = send_data.dump();
    req.prepare_payload();
  // Send the HTTP request to the remote host

    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::string_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to standard out
    std::cout << json::parse(res.body()).dump(2) << std::endl;

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
      throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
  }
  catch(std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    throw e;
  }
  return 0;
}