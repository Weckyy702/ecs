#include "socket.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <exception>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

Socket::Socket() : socket_{create_listener()} {}

Socket::~Socket() { check(close(socket_), "Socket::close"); }

void Socket::wait_for_connection() {
  sockaddr_in my_addr{};
  socklen_t len = sizeof(my_addr);
  check(getsockname(socket_, reinterpret_cast<sockaddr *>(&my_addr), &len),
        "getsockname");

  std::println(std::cout, "Listening on port {}...", my_addr.sin_port);

  len = sizeof(peer_address_);
  std::array<std::byte, 8> buf{};
  for (;;) {
    auto status = recvfrom(socket_, buf.data(), buf.size(), 0,
                           reinterpret_cast<sockaddr *>(&peer_address_), &len);
    if (status >= 0) {
      break;
    }
    if (errno == EAGAIN) {
      continue;
    }
    perror("wait_for_connection/recvfrom");
    std::terminate();
  }
}

void Socket::connect(std::string const &addr_string, in_port_t port) {
  in_addr addr{};
  if (inet_aton(addr_string.c_str(), &addr) == 0) {
    std::println(std::cerr, "Invalid address '{}'", addr_string);
    std::terminate();
  }

  peer_address_.sin_family = AF_INET;
  peer_address_.sin_addr = addr;
  peer_address_.sin_port = port;

  constexpr std::string_view msg = "PONG";
  check(sendto(socket_, msg.data(), msg.size(), 0,
               reinterpret_cast<sockaddr const *>(&peer_address_),
               sizeof(peer_address_)),
        "sendto");
}

int Socket::create_listener() {
  auto const sock =
      check(socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0), "socket");
  sockaddr_in const my_addr{.sin_family = AF_INET,
                            .sin_port = 0,
                            .sin_addr{INADDR_ANY},
                            .sin_zero = {}};
  check(
      bind(sock, reinterpret_cast<sockaddr const *>(&my_addr), sizeof(my_addr)),
      "bind");
  return sock;
}
