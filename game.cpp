#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <ctime>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

const int FPS_LIMIT = 60;

const float M_TO_PX = 7529.f;
const float PX_TO_M = 1.f / M_TO_PX;

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

class Body {
public:
  Body(float x, float y, float width, float height, float mass)
      : x(x), y(y), width(width), height(height), mass(mass), x_force(0),
        y_force(0), x_vel(0), y_vel(0) {}

  void apply_newton_force(float x, float y) {
    x_force += x * M_TO_PX; // kg * px / s^2
    y_force += y * M_TO_PX; // kg * px / s^2
  }

  Body update(Uint64 delta_time_ms) {
    float delta_time_s = float(delta_time_ms) / 1000.0f;

    Body new_body = *this;

    // a = F / m
    // [a] = px / s^2
    auto x_acc = x_force / mass;
    auto y_acc = y_force / mass;

    // s = s0 + v0 * t + a * t^2 / 2
    // [s] = px
    new_body.x +=
        (x_vel * delta_time_s) + (x_acc * delta_time_s * delta_time_s / 2);
    new_body.y +=
        (y_vel * delta_time_s) + (y_acc * delta_time_s * delta_time_s / 2);

    // v = v0 + a * t
    // [v] = px / s
    new_body.x_vel += x_acc * delta_time_s;
    new_body.y_vel += y_acc * delta_time_s;

    return new_body;
  }

  SDL_Rect get_sdl_rect() const {
    return SDL_Rect{
        int(std::round(x)),
        int(std::round(y)),
        int(std::round(width)),
        int(std::round(height)),
    };
  }

  void redraw(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_Rect rect = get_sdl_rect();
    SDL_RenderFillRect(renderer, &rect);
  }

public:
  float x;
  float y;
  float width;
  float height;
  float mass;
  float x_force;
  float y_force;
  float x_vel;
  float y_vel;
};

class Scene {
public:
  virtual void update(Uint64 delta_time_ms) = 0;
  virtual void redraw(SDL_Renderer *renderer) = 0;
};

class CollisionChecker {
public:
  CollisionChecker() {}

  std::set<std::shared_ptr<Body>>
  check_collisions(const std::vector<std::shared_ptr<Body>> &bodies) {
    std::set<std::shared_ptr<Body>> colliding_bodies;
    for (size_t i = 0; i < bodies.size(); i++) {
      for (size_t j = i + 1; j < bodies.size(); j++) {
        if (bodies[i]->x < bodies[j]->x + bodies[j]->width &&
            bodies[i]->x + bodies[i]->width > bodies[j]->x &&
            bodies[i]->y < bodies[j]->y + bodies[j]->height &&
            bodies[i]->y + bodies[i]->height > bodies[j]->y) {
          colliding_bodies.insert(bodies[i]);
        }
      }
    }
    return colliding_bodies;
  }
};

class PlaygroundScene : public Scene {
public:
  PlaygroundScene() {
    for (int i = 0; i < 5; i++) {
      bodies_.push_back(std::make_shared<Body>(
          rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, 50, 50, 1));
    }
  }

  virtual void update(Uint64 delta_time_ms) {
    auto requested_bodies = std::vector<std::shared_ptr<Body>>(bodies_.size());
    for (size_t i = 0; i < bodies_.size(); i++) {
      requested_bodies[i] =
          std::make_shared<Body>(bodies_[i]->update(delta_time_ms));
    }

    auto collision_checker = CollisionChecker();
    auto colliding_bodies =
        collision_checker.check_collisions(requested_bodies);

    for (auto &body : bodies_) {

      // std::vector<std::shared_ptr<Body>> requested_bodies(bodies_.size());
      // for (auto &body : bodies_) {
      //   body->update(delta_time_ms);
      // }
    }
  }

  virtual void redraw(SDL_Renderer *renderer) {
    for (auto &body : bodies_) {
      body->redraw(renderer);
    }
  }

private:
  std::vector<std::shared_ptr<Body>> bodies_;
};

class Game {
public:
  Game()
      : window_(build_window()), renderer_(build_renderer()),
        scene_(std::make_unique<PlaygroundScene>()) {
    if (window_ == nullptr) {
      std::cout << "Error window creation";
      return;
    }

    if (renderer_ == nullptr) {
      std::cout << "Error renderer creation";
      return;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
      std::cout << "Error SDL2 Initialization : " << SDL_GetError();
      return;
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
      std::cout << "Error SDL2_image Initialization";
      return;
    }

    if (renderer_ == nullptr) {
      std::cout << "Error renderer creation";
      return;
    }

    running_ = true;
  }

  ~Game() {
    IMG_Quit();
    SDL_Quit();
  }

  std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> build_window() {
    return std::unique_ptr<SDL_Window, void (*)(SDL_Window *)>(
        SDL_CreateWindow("First program", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI),
        SDL_DestroyWindow);
  }

  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> build_renderer() {
    return std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)>(
        SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED),
        SDL_DestroyRenderer);
  }

  void run() {
    auto last_frame_end = SDL_GetTicks64();
    while (running_) {
      auto frame_start = SDL_GetTicks64();
      auto delta_time = frame_start - last_frame_end;

      update(delta_time);
      redraw();

      auto frame_end = SDL_GetTicks64();
      last_frame_end = frame_end;
      if (frame_end - frame_start < 1000 / FPS_LIMIT) {
        auto frame_delay = 1000 / FPS_LIMIT - (frame_end - frame_start);
        SDL_Delay(frame_delay);
      }
    }
  }

private:
  void handle_events() {
    auto event = SDL_Event();
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running_ = false;
      } else if (event.type == SDL_KEYDOWN) {
        handle_key_down_event(event.key);
      } else if (event.type == SDL_KEYUP) {
        handle_key_up_event(event.key);
      }
    }
  }

  void handle_keyboard_state() {
    auto keyboard_state = SDL_GetKeyboardState(nullptr);
    if (keyboard_state[SDL_SCANCODE_LEFT]) {
      std::cout << "L" << std::endl;
    }
    if (keyboard_state[SDL_SCANCODE_RIGHT]) {
      std::cout << "R" << std::endl;
    }
  }

  void handle_key_up_event(const SDL_KeyboardEvent &keyboard_event) {}

  void handle_key_down_event(const SDL_KeyboardEvent &keyboard_event) {}

  void update(Uint64 delta_time_ms) {
    handle_events();
    handle_keyboard_state();

    scene_->update(delta_time_ms);
  }

  void redraw() {
    SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer_.get());

    scene_->redraw(renderer_.get());

    SDL_RenderPresent(renderer_.get());
  }
  const std::string WINDOW_TITLE = "First program";

  bool running_;
  std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window_;
  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer_;
  std::unique_ptr<Scene> scene_;
};

int main(int argc, char *argv[]) {
  std::srand(0);
  Game game;
  game.run();
  return 0;
}