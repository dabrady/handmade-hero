#include <SDL.h>

int main(int argc, char** argv) {
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
                           "Handmade Hero",
                           "This is handmade. I made it.",
                           NULL);

  return 0;
}
