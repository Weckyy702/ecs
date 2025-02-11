#include "ecs/EntityID.hpp"
#include "ecs/System.hpp"
#include "ecs/ecs.hpp"
#include "socket.hpp"

#include "raylib.h"
#include <functional>
#include <iostream>
#include <memory>

struct Ball;
struct Player;
struct Physics;
struct Score;
struct PlayerController;
struct Server;
struct Client;

using Ecs =
    ECS::Ecs<Ball, Player, Physics, Score, PlayerController, Server, Client>;

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
  static void run(Ball const &ball) {
    DrawCircleV(ball.position, ball.radius, RED);
  }
};

struct PlayerRenderer : ECS::BaseSystem<PlayerRenderer, Player> {
  static void run(Player const &player) {
    DrawRectangleV(player.position, player.size, BLUE);
  }
};

struct ScoreRenderer : ECS::BaseSystem<ScoreRenderer, Score> {
  void run(Score const &score) const {
    {
      const auto txt = std::format("{}", score.left);
      DrawText(txt.c_str(), width / 4, 20, 32, RAYWHITE);
    }
    {
      const auto txt = std::format("{}", score.right);
      DrawText(txt.c_str(), 3 * width / 4, 20, 32, RAYWHITE);
    }
  }

  int width;
};

struct BallUpdate : ECS::BaseSystem<BallUpdate, Ball, Physics> {
  void run(Ball &ball, Physics &physics) const {
    auto &[x, y] = ball.position;
    auto &[vx, vy] = physics.direction;

    if (y >= height - ball.radius || y <= ball.radius) {
      vy *= -1;
    }

    if (collides(ball, left)) {
      vx = 1;
    }
    if (collides(ball, right)) {
      vx = -1;
    }

    x += vx * physics.speed;
    y += vy * physics.speed;

    physics.speed *= 1 + 5e-4F;
  }

  [[nodiscard]] bool collides(Ball const &ball, ECS::EntityID player) const {
    const auto [x, y] =
        ecs->get_component<Player>(player).value().get().position;
    const auto [w, h] = Player::size;
    return CheckCollisionCircleRec(ball.position, ball.radius,
                                   Rectangle{x, y, w, h});
  }

  float width{}, height{};
  std::shared_ptr<Ecs> ecs;
  ECS::EntityID left, right;
};

struct PlayerUpdate : ECS::BaseSystem<PlayerUpdate, Player, PlayerController> {
  void run(Player &player, PlayerController const &pc) const {
    auto &[_, y] = player.position;
    if (IsKeyDown(pc.up)) {
      y = std::max(0.F, y - 5.F);
    }
    if (IsKeyDown(pc.down)) {
      y = std::min(height - Player::size.y, y + 5.F);
    }
  }

  float height;
};

struct ScoreUpdate : ECS::BaseSystem<ScoreUpdate, Score, Ball> {
  void run(Score &score, Ball &ball) const {
    auto &[x, _] = ball.position;

    if (x - ball.radius <= 0) {
      TraceLog(LOG_DEBUG, "Right score");
      x = width / 2;
      score.right += 1;
      return;
    }

    if (x + ball.radius >= width) {
      TraceLog(LOG_DEBUG, "Left score");
      x = width / 2;
      score.left += 1;
      return;
    }
  }

  float width{};
};

struct [[gnu::packed]] ServerPacket {
  Ball ball;
  Player player;
};

struct [[gnu::packed]] ClientPacket {
  Player player;
};

struct ServerUpdate : ECS::BaseSystem<ServerUpdate, Server> {
  void run(Server const &server) const {
    auto ball = ecs->get_component<Ball>(server.ball).value().get();
    ball.position.x = width - ball.position.x;
    auto player = ecs->get_component<Player>(server.left).value().get();
    player.position.x = width - player.position.x;

    socket.send(ServerPacket{.ball = ball, .player = player});
    auto const response = socket.receive<ClientPacket>();

    if (!response.has_value()) {
      return;
    }

    ecs->get_component<Player>(server.right).value().get() = response->player;
  }

  void wait_for_connection() { socket.wait_for_connection(); }

  Socket socket;
  std::shared_ptr<Ecs> ecs;
  float width;
};

struct ClientUpdate : ECS::BaseSystem<ClientUpdate, Client> {
  void run(Client &c) const {
    auto player = ecs->get_component<Player>(c.left).value().get();
    player.position.x = width - player.position.x;

    socket.send(ClientPacket{player});

    auto const response = socket.receive<ServerPacket>();
    if (!response) {
      return;
    }

    ecs->get_component<Ball>(c.ball).value().get() = response->ball;
    ecs->get_component<Player>(c.right).value().get() = response->player;
  }

  void connect() {
    std::string addr_string;
    std::cout << "Server addresss: " << std::flush;
    std::cin >> addr_string;

    in_port_t port{};
    std::cout << "Server port: " << std::flush;
    std::cin >> port;

    socket.connect(addr_string, port);
  }

  Socket socket;
  std::shared_ptr<Ecs> ecs;
  float width;
};

int main() {
  constexpr auto width = 800;
  constexpr auto height = 600;

  SetTraceLogLevel(LOG_DEBUG);

  auto ecs = std::make_shared<Ecs>();

  const auto ball =
      ecs->create(Ball{Vector2{width / 2., height / 2.}}, Score{});
  const auto left =
      ecs->create(Player{{.x = 10, .y = height / 2.}},
                  PlayerController{.up = KEY_UP, .down = KEY_DOWN});
  const auto right = ecs->create(Player{{
      .x = width - 10 - Player::size.x,
      .y = height / 2.,
  }});

  std::println(std::cout, "Is this the server? y/N");
  char answer{};
  std::cin >> answer;

  bool const is_server = std::tolower(answer) == 'y';

  if (is_server) {
    ecs->add_components(ball, Physics{});
    ecs->create(Server{.left = left, .right = right, .ball = ball});
  } else {
    ecs->create(Client{.left = left, .right = right, .ball = ball});
  }

  ServerUpdate server_update{.ecs = ecs, .width = width};
  ClientUpdate client_update{.ecs = ecs, .width = width};

  if (is_server) {
    server_update.wait_for_connection();
  } else {
    client_update.connect();
  }

  BallUpdate ball_update{.width = width,
                         .height = height,
                         .ecs = ecs,
                         .left = left,
                         .right = right};

  InitWindow(width, height, "ECS Pong");
  SetTargetFPS(24);

  while (!WindowShouldClose()) {
    BeginDrawing();
    {
      ClearBackground(DARKGRAY);

      ecs->run(BallRenderer{});
      ecs->run(PlayerRenderer{});
      ecs->run(ScoreRenderer{.width = width});
    }
    EndDrawing();

    ecs->run(PlayerUpdate{.height = height});
    ecs->run(ball_update);
    ecs->run(server_update);
    ecs->run(client_update);
    ecs->run(ScoreUpdate{.width = width});
  }

  CloseWindow();
}
