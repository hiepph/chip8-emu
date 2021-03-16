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

// Keypad       Keyboard
// +-+-+-+-+    +-+-+-+-+
// |1|2|3|C|    |1|2|3|4|
// +-+-+-+-+    +-+-+-+-+
// |4|5|6|D|    |Q|W|E|R|
// +-+-+-+-+ => +-+-+-+-+
// |7|8|9|E|    |A|S|D|F|
// +-+-+-+-+    +-+-+-+-+
// |A|0|B|F|    |Z|X|C|V|
// +-+-+-+-+    +-+-+-+-+
void do_input(uint8_t *keypad) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      exit(0);

    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        exit(0);
      case SDLK_x:
        keypad[0] = 1;
        break;
      case SDLK_1:
        keypad[1] = 1;
        break;
      case SDLK_2:
        keypad[2] = 1;
        break;
      case SDLK_3:
        keypad[3] = 1;
        break;
      case SDLK_q:
        keypad[4] = 1;
        break;
      case SDLK_w:
        keypad[5] = 1;
        break;
      case SDLK_e:
        keypad[6] = 1;
        break;
      case SDLK_a:
        keypad[7] = 1;
        break;
      case SDLK_s:
        keypad[8] = 1;
        break;
      case SDLK_d:
        keypad[9] = 1;
        break;
      case SDLK_z:
        keypad[0xA] = 1;
        break;
      case SDLK_c:
        keypad[0xB] = 1;
        break;
      case SDLK_4:
        keypad[0xC] = 1;
        break;
      case SDLK_r:
        keypad[0xD] = 1;
        break;
      case SDLK_f:
        keypad[0xE] = 1;
        break;
      case SDLK_v:
        keypad[0xF] = 1;
        break;
      }
      break;

    case SDL_KEYUP:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
        exit(0);
      case SDLK_x:
        keypad[0] = 0;
        break;
      case SDLK_1:
        keypad[1] = 0;
        break;
      case SDLK_2:
        keypad[2] = 0;
        break;
      case SDLK_3:
        keypad[3] = 0;
        break;
      case SDLK_q:
        keypad[4] = 0;
        break;
      case SDLK_w:
        keypad[5] = 0;
        break;
      case SDLK_e:
        keypad[6] = 0;
        break;
      case SDLK_a:
        keypad[7] = 0;
        break;
      case SDLK_s:
        keypad[8] = 0;
        break;
      case SDLK_d:
        keypad[9] = 0;
        break;
      case SDLK_z:
        keypad[0xA] = 0;
        break;
      case SDLK_c:
        keypad[0xB] = 0;
        break;
      case SDLK_4:
        keypad[0xC] = 0;
        break;
      case SDLK_r:
        keypad[0xD] = 0;
        break;
      case SDLK_f:
        keypad[0xE] = 0;
        break;
      case SDLK_v:
        keypad[0xF] = 0;
        break;
      }
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
