#include <SDL.h>
#include <stdlib.h>

/* Forward declarations */
bool HandleEvent(SDL_Event*, SDL_Renderer*);

int main(int argc, char** argv) {
#if 1
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

  Uint32 subsystems =
    SDL_INIT_VIDEO; // graphics and window management

  // Initialize SDL
  if (SDL_InitSubSystem(subsystems) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
    return(1);
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized");

  // Shutdown SDL on exit
  atexit(SDL_Quit);

  // Setup initial window
  Uint32 windowFlags =
    SDL_WINDOW_RESIZABLE;
  SDL_Window* window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        640,                     // int         w,
                                        480,                     // int         h,
                                        windowFlags              // Uint32      flags
                                        );
  // Check if the window creation was successful.
  if (window == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  Uint32 renderFlags = 0;
  SDL_Renderer* renderer = SDL_CreateRenderer(window, AUTODETECT_DRIVER, renderFlags);
  if(renderer == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create rendering context: %s", SDL_GetError());
    return(1);
  }

  // Main event loop
  for(;;) {
    SDL_Event event;
    if(SDL_WaitEvent(&event)) {
      if(HandleEvent(&event, renderer)) {
        SDL_DestroyWindow(window);
        break;
      }
    }
    else
    {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error waiting for main application event: %s", SDL_GetError());
      // Keep going.
    }
  }

  return(0);
}

// ********

bool HandleEvent(SDL_Event* Event, SDL_Renderer* renderer) {
  bool shouldQuit = false;

  switch(Event->type) {
    case SDL_QUIT:
    {
      SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "quit");
      shouldQuit = true;
    } break;

    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window resized: (%d x %d)", Event->window.data1, Event->window.data2);
        } break;

        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window gained focus");
        } break;

        case SDL_WINDOWEVENT_CLOSE:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window closed");
        } break;

        case SDL_WINDOWEVENT_EXPOSED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window exposed");

          // Draw something.
          // Toggle between black and white.
          static bool drawWhite = true;
          if (drawWhite) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
            drawWhite = false;
          }
          else
          {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            drawWhite = true;
          }

          // Redraw only if the window has been re-exposed
          SDL_RenderClear(renderer);
          SDL_RenderPresent(renderer);
        } break;
      }
    } break;

    default:
    {
      // SDL_LogDebug("default");
    } break;
  }

  return(shouldQuit);
}
