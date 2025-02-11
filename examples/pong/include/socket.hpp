#pragma once

#include <cerrno>
#include <exception>
#include <netinet/in.h>
#include <optional>
#include <string>
#include <sys/socket.h>

class Socket {
public:
  Socket();

  Socket(Socket const &) = delete;
  Socket &operator=(Socket const &) = delete;

  Socket(Socket &&) = default;
  Socket &operator=(Socket &&) = default;

  ~Socket();

  template <typename Packet> void send(Packet const &packet) const {
    using Data = std::array<std::byte, sizeof(Packet)>;
    Data const data = std::bit_cast<Data>(packet);

    assert(check(sendto(socket_, data.data(), data.size(), 0,
                        reinterpret_cast<sockaddr const *>(&peer_address_),
                        sizeof(peer_address_)),
                 "sendto") == sizeof(Packet));
  }

  template <typename Packet> std::optional<Packet> receive() const {
    using Data = std::array<std::byte, sizeof(Packet)>;
    Data data;

    auto const sz = recv(socket_, data.data(), data.size(), 0);
    if (sz < 0) {
      if (errno == EAGAIN) {
        return std::nullopt;
      }
      perror(__PRETTY_FUNCTION__);
      std::terminate();
    }
    assert(sz == sizeof(Packet));
    return std::bit_cast<Packet>(data);
  }

  void wait_for_connection();

  void connect(std::string const &addr_string, in_port_t port);

private:
  template <std::integral T> static T check(T status, std::string const &msg) {
    if (status < 0) {
      perror(msg.c_str());
      std::terminate();
    }
    return status;
  }

  static int create_listener();

  int socket_;
  sockaddr_in peer_address_{};
};
