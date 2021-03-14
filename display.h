#include <SDL2/SDL.h>

typedef struct {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Texture *texture;
} App;


void initSDL(App* app,
             unsigned int windowWidth, unsigned int windowHeight,
             unsigned int textureWidth, unsigned int textureHeight) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  app->window = SDL_CreateWindow("Chip8",
                                 0, 0,
                                 windowWidth, windowHeight,
                                 SDL_WINDOW_SHOWN);
  if (!app->window) {
    printf("Failed to open window: %s\n", SDL_GetError());
    exit(1);
 }

  app->renderer = SDL_CreateRenderer(app->window, -1,
                                     SDL_RENDERER_ACCELERATED);
  if (!app->renderer) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    exit(1);
  }

  app->texture = SDL_CreateTexture(app->renderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   textureWidth, textureHeight);
}

void cleanup(App* app) {
	SDL_DestroyRenderer(app->renderer);
	SDL_DestroyWindow(app->window);
	SDL_DestroyTexture(app->texture);
	SDL_Quit();
}

void do_input(void) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
      break;

    case SDL_KEYDOWN:
      //
      break;

    case SDL_KEYUP:
      //
      break;

    default:
      break;
    }
  }
}

void update_display(App* app, void const* display, int pitch) {
  SDL_UpdateTexture(app->texture, NULL, display, pitch);
  SDL_RenderClear(app->renderer);
  SDL_RenderCopy(app->renderer, app->texture, NULL, NULL);
  SDL_RenderPresent(app->renderer);
}
