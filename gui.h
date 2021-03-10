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
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
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
                                   SDL_PIXELFORMAT_BGR888,
                                   SDL_TEXTUREACCESS_STREAMING,
                                   textureWidth, textureHeight);
}

void cleanup(App* app) {
	SDL_DestroyRenderer(app->renderer);
	SDL_DestroyWindow(app->window);
	SDL_Quit();
}

void doInput(void) {
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
