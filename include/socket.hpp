#pragma once

#include <netinet/in.h>
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

  template <typename Packet> void send(Packet const &p) const {
    using Data = std::array<std::byte, sizeof(Packet)>;
    Data const data = std::bit_cast<Data>(p);

    assert(check(sendto(socket_, data.data(), data.size(), 0,
                        reinterpret_cast<sockaddr const *>(&peer_address_),
                        sizeof(peer_address_)),
                 "sendto") == sizeof(Packet));
  }

  template <typename Packet> Packet receive() const {
    using Data = std::array<std::byte, sizeof(Packet)>;
    Data data;

    auto const sz = check(recv(socket_, data.data(), data.size(), 0), "recv");
    assert(sz == sizeof(Packet));
    return std::bit_cast<Packet>(data);
  }

  void wait_for_connection();

  void connect(std::string const &addr_string, in_port_t port);

private:
  template <std::integral T> static T check(T status, std::string_view msg) {
    if (status < 0) {
      perror(msg.data());
      exit(1);
    }
    return status;
  }

  static int create_listener();

  int socket_;
  sockaddr_in peer_address_;
};
