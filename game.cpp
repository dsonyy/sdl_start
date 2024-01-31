#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

const int FPS_LIMIT = 1;

const float S_TO_FRAME = FPS_LIMIT;
const float FRAME_TO_S = 1.0f / S_TO_FRAME;
const float M_TO_PX = 1.0f;
const float PX_TO_M = 1.0f / M_TO_PX;

class Body {
public:
  Body(float x, float y, float width, float height, float mass)
      : x(x), y(y), width(width), height(height), mass(mass) {}

  void apply_force(float x, float y) {
    x_force += x * M_TO_PX; // kg * px / s^2
    y_force += y * M_TO_PX; // kg * px / s^2
  }

  void update(Uint64 delta_time_ms) {
    float delta_time_s = float(delta_time_ms) / 1000.0f;

    float x_acc = x_force / mass * delta_time_s * delta_time_s;

    // x_vel = M_TO_PX * delta_time_s;
    // y_vel = 0;
    // float y_delta = y_force * delta_time_s * delta_time_s / mass;
    // y += y_delta;

    y_vel += y_force / mass * delta_time_s;
    y += y_vel * delta_time_s;

    std::cout << y_vel << " " << y << std::endl;
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

SDL_Rect get_sdl_rect(const Body &body) {
  SDL_Rect rect = {
      int(std::round(body.x)),
      int(std::round(body.y)),
      int(std::round(body.width)),
      int(std::round(body.height)),
  };
  return rect;
}

void sdl_draw_body(SDL_Renderer *renderer, const Body &body) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
  SDL_Rect rect = get_sdl_rect(body);
  SDL_RenderFillRect(renderer, &rect);
}

class Game {
public:
  Game() : window_(build_window()), renderer_(build_renderer()) {
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

    bodies_.push_back(Body(100, 100, 50, 50, 1));
    bodies_.front().apply_force(0, 10.0f);
  }
  ~Game() {
    IMG_Quit();
    SDL_Quit();
  }

  std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> build_window() {
    return std::unique_ptr<SDL_Window, void (*)(SDL_Window *)>(
        SDL_CreateWindow("First program", SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 1280, 720,
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

    for (auto &body : bodies_) {
      body.update(delta_time_ms);
    }
  }

  void redraw() {
    SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer_.get());

    for (auto &body : bodies_) {
      sdl_draw_body(renderer_.get(), body);
    }

    SDL_RenderPresent(renderer_.get());
  }

  const int SCREEN_WIDTH = 1280;
  const int SCREEN_HEIGHT = 720;
  const std::string WINDOW_TITLE = "First program";

  bool running_;
  std::unique_ptr<SDL_Window, void (*)(SDL_Window *)> window_;
  std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer *)> renderer_;
  std::vector<Body> bodies_;
};

int main(int argc, char *argv[]) {
  Game game;
  game.run();
  return 0;
}