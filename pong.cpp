#include "ecs.hpp"

#include "raylib.h"
#include "socket.hpp"
#include <iostream>

struct Ball;
struct Player;
struct Physics;
struct Score;
struct PlayerController;
struct Server;
struct Client;

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

struct Server {
  ECS::EntityID left, right, ball;
};

struct Client {
  ECS::EntityID left, right, ball;
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

struct ScoreUpdate : ECS::BaseSystem<ScoreUpdate, Score, Ball> {
  void run(Score &s, Ball &ball) const {
    auto &[x, _] = ball.position;

    if (x - ball.radius <= 0) {
      TraceLog(LOG_DEBUG, "Right score");
      x = width / 2;
      s.right += 1;
      return;
    }

    if (x + ball.radius >= width) {
      TraceLog(LOG_DEBUG, "Left score");
      x = width / 2;
      s.left += 1;
      return;
    }
  }

  Ecs &ecs;
  float width;
};

struct [[gnu::packed]] ServerPacket {
  Ball ball;
  Player player;
};

struct [[gnu::packed]] ClientPacket {
  Player player;
};

struct ServerUpdate : ECS::BaseSystem<ServerUpdate, Server> {
  void run(Server const &s) const {
    auto ball = ecs.get_component<Ball>(s.ball).value().get();
    ball.position.x = width - ball.position.x;
    auto player = ecs.get_component<Player>(s.left).value().get();
    player.position.x = width - player.position.x;

    socket.send(ServerPacket{ball, player});
    auto const response = socket.receive<ClientPacket>();

    if (!response.has_value())
      return;

    ecs.get_component<Player>(s.right).value().get() = response->player;
  }

  void wait_for_connection() { socket.wait_for_connection(); }

  Socket socket{};
  Ecs &ecs;
  float width;
};

struct ClientUpdate : ECS::BaseSystem<ClientUpdate, Client> {
  void run(Client &c) const {
    auto player = ecs.get_component<Player>(c.left).value().get();
    player.position.x = width - player.position.x;

    socket.send(ClientPacket{player});

    auto const response = socket.receive<ServerPacket>();
    if (!response)
      return;

    ecs.get_component<Ball>(c.ball).value().get() = response->ball;
    ecs.get_component<Player>(c.right).value().get() = response->player;
  }

  void connect() {
    std::string addr_string;
    std::cout << "Server addresss: " << std::flush;
    std::cin >> addr_string;

    in_port_t port;
    std::cout << "Server port: " << std::flush;
    std::cin >> port;

    socket.connect(addr_string, port);
  }

  Socket socket{};
  Ecs &ecs;
  float width;
};

int main() {
  constexpr auto width = 800, height = 600;

  Ecs ecs{};

  const auto ball = ecs.create(Ball{Vector2{width / 2., height / 2.}}, Score{});
  const auto left =
      ecs.create(Player{{.x = 10, .y = height / 2.}},
                 PlayerController{.up = KEY_UP, .down = KEY_DOWN});
  const auto right = ecs.create(Player{{
      .x = width - 10 - Player::size.x,
      .y = height / 2.,
  }});

  std::println(std::cout, "Is this the server? y/N");
  char answer{};
  std::cin >> answer;

  bool const is_server = std::tolower(answer) == 'y';

  if (is_server) {
    ecs.add_components(ball, Physics{});
    ecs.create(Server{left, right, ball});
  } else {
    ecs.create(Client{left, right, ball});
  }

  InitWindow(width, height, "ECS Pong");
  SetTargetFPS(24);

  SetTraceLogLevel(LOG_DEBUG);

  ServerUpdate server_update{.ecs = ecs, .width = width};
  ClientUpdate client_update{.ecs = ecs, .width = width};

  if (is_server) {
    server_update.wait_for_connection();
  } else {
    client_update.connect();
  }

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
    ecs.run(server_update);
    ecs.run(client_update);
    ecs.run(ScoreUpdate{.ecs = ecs, .width = width});
  }

  CloseWindow();
}
