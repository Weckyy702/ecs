#include "EntityID.hpp"

#include "System.hpp"
#include "ecs.hpp"

#include "raylib.h"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <format>

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

struct Ball;
struct Player;
struct Physics;
struct Score;
struct PlayerController;
class Server;
class Client;

using Ecs =
    ECS::Ecs<Ball, Player, Physics, Score, PlayerController, Server, Client>;

#define IGNORE (void)

struct Ball {
  constexpr static float radius = 10;
  Vector2 position;
};

struct Physics {
  Vector2 direction{.x = 1, .y = 1};
  float speed = 2.;
};

struct Player {
  constexpr static Vector2 size{8, 80};

  Vector2 position;
};

struct PlayerController {
  int up, down;
};

struct Score {
  uint8_t left, right;
};

auto check(std::integral auto status, std::string_view msg) {
  if (status < 0) {
    perror(msg.data());
    exit(1);
  }
  return status;
}

int create_listener() {
  auto const sock =
      check(socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0), "socket");
  sockaddr_in const my_addr{.sin_family = AF_INET,
                            .sin_port = 0,
                            .sin_addr = INADDR_ANY,
                            .sin_zero = {}};
  check(
      bind(sock, reinterpret_cast<sockaddr const *>(&my_addr), sizeof(my_addr)),
      "bind");
  return sock;
}

class Server {
public:
  Server(ECS::EntityID player, ECS::EntityID ball)
      : player_{player}, ball_{ball}, socket_{create_listener()} {}

  Server(Server const &) = delete;
  Server &operator=(Server const &) = delete;

  void wait_for_connection() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    check(getsockname(socket_, reinterpret_cast<sockaddr *>(&addr), &len),
          "getsockname");

    std::println(std::cout, "Listening on port {}...", addr.sin_port);
    char buf[8];
    check(recv(socket_, buf, sizeof(buf), 0), "recv");
  }

  ~Server() { close(socket_); }

private:
  ECS::EntityID player_, ball_;
  int socket_;
};

class Client {
public:
  Client(ECS::EntityID player, ECS::EntityID ball)
      : player_{player}, ball_{ball} {}

private:
  ECS::EntityID player_, ball_;
};

struct BallRenderer : ECS::BaseSystem<BallRenderer, Ball> {
  void run(Ball const &ball) const {
    DrawCircleV(ball.position, ball.radius, RED);
  }
};

struct PlayerRenderer : ECS::BaseSystem<PlayerRenderer, Player> {
  void run(Player const &player) const {
    DrawRectangleV(player.position, player.size, BLUE);
  }
};

struct ScoreRenderer : ECS::BaseSystem<ScoreRenderer, Score> {
  void run(Score const &s) const {
    {
      const auto t = std::format("{}", s.left);
      DrawText(t.c_str(), width / 4, 20, 32, RAYWHITE);
    }
    {
      const auto t = std::format("{}", s.right);
      DrawText(t.c_str(), 3 * width / 4, 20, 32, RAYWHITE);
    }
  }

  int width;
};

struct BallUpdate : ECS::BaseSystem<BallUpdate, Ball, Physics> {
  void run(Ball &ball, Physics &p) const {
    auto &[x, y] = ball.position;
    auto &[vx, vy] = p.direction;

    if (y >= height - ball.radius || y <= ball.radius)
      vy *= -1;

    if (collides(ball, left))
      vx = 1;
    if (collides(ball, right))
      vx = -1;

    TraceLog(LOG_DEBUG, "Ball Speed: %f", p.speed);

    x += vx * p.speed;
    y += vy * p.speed;

    p.speed *= 1 + 5e-4f;
  }

  bool collides(Ball const &ball, ECS::EntityID player) const {
    const auto [x, y] = ecs.get_component<Player>(player)->get().position;
    const auto [w, h] = Player::size;
    return CheckCollisionCircleRec(ball.position, ball.radius,
                                   Rectangle{x, y, w, h});
  }

  float width, height;
  Ecs &ecs;
  ECS::EntityID left, right;
};

struct PlayerUpdate : ECS::BaseSystem<PlayerUpdate, Player, PlayerController> {
  void run(Player &player, PlayerController const &pc) const {
    auto &[_, y] = player.position;
    if (IsKeyDown(pc.up)) {
      y = std::max(0.f, y - 5.f);
    }
    if (IsKeyDown(pc.down)) {
      y = std::min(height - Player::size.y, y + 5.f);
    }
  }

  float height;
};

struct ScoreUpdate : ECS::BaseSystem<ScoreUpdate, Score> {
  void run(Score &s) const {
    auto &ball = ecs.get_component<Ball>(this->ball).value().get();
    auto &[x, _] = ball.position;

    if (x - ball.radius <= 0) {
      TraceLog(LOG_DEBUG, "Right score");
      x = width / 2;
      s.left += 1;
      return;
    }

    if (x + ball.radius >= width) {
      TraceLog(LOG_DEBUG, "Left score");
      x = width / 2;
      s.right += 1;
      return;
    }
  }

  Ecs &ecs;
  ECS::EntityID ball;
  float width;
};

struct ServerUpdate : ECS::BaseSystem<ServerUpdate, Server> {
  void run(Server &s) const { IGNORE s; }
};

struct ClientUpdate : ECS::BaseSystem<ClientUpdate, Client> {
  void run(Client &client) const { IGNORE client; }
};

int main() {
  constexpr auto width = 800, height = 600;

  Ecs ecs{};

  const auto ball = ecs.create(Ball{Vector2{width / 2., height / 2.}});
  const auto left =
      ecs.create(Player{{.x = 10, .y = height / 2.}},
                 PlayerController{.up = KEY_UP, .down = KEY_DOWN});
  const auto right = ecs.create(Player{{
      .x = width - 10 - Player::size.x,
      .y = height / 2.,
  }});

  if (true) {
    ecs.add_components(ball, Physics{});
    auto const id = ecs.create(Server{left, ball});
    auto &server = ecs.get_component<Server>(id).value().get();

    server.wait_for_connection();
  } else {
    ecs.create(Client(right, ball));
  }

  ecs.create(Score{});

  InitWindow(width, height, "ECS Pong");
  SetTargetFPS(24);

  SetTraceLogLevel(LOG_DEBUG);

  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(DARKGRAY);

      ecs.run(BallRenderer{});
      ecs.run(PlayerRenderer{});
      ecs.run(ScoreRenderer{.width = width});
    }
    EndDrawing();

    ecs.run(PlayerUpdate{.height = height});
    ecs.run(BallUpdate{.width = width,
                       .height = height,
                       .ecs = ecs,
                       .left = left,
                       .right = right});
    ecs.run(ServerUpdate{});
    ecs.run(ClientUpdate{});
    ecs.run(ScoreUpdate{.ecs = ecs, .ball = ball, .width = width});
  }

  CloseWindow();
}
