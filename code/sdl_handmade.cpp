#include <SDL.h>
#include <stdlib.h>

/* Macros */

// The many faces of 'static'
#define internal static
#define local_persist static
#define global_variable static

/* Globals */
global_variable bool Running;


/* Forward declarations */
internal int WindowResizeEventFilter(void*, SDL_Event*);
internal void HandleEvent(SDL_Event*, SDL_Renderer*);

int main(int ArgCount, char** ArgValues)
{
#if 1
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

  Uint32 Subsystems =
    SDL_INIT_VIDEO; // graphics and window management

  // Initialize SDL
  if (SDL_InitSubSystem(Subsystems) != 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
    return(1);
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized");

  // Shutdown SDL on exit
  atexit(SDL_Quit);

  // Setup initial Window
  Uint32 WindowFlags =
    SDL_WINDOW_RESIZABLE;
  SDL_Window* Window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        640,                     // int         w,
                                        480,                     // int         h,
                                        WindowFlags              // Uint32      flags
                                        );
  // Check if the window creation was successful.
  if (Window == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);W
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  Uint32 RenderFlags = 0;
  SDL_Renderer* Renderer = SDL_CreateRenderer(Window, AUTODETECT_DRIVER, RenderFlags);
  if(Renderer == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create rendering context: %s", SDL_GetError());
    return(1);
  }

  // Main event loop
  Running = true;
  SDL_AddEventWatch(WindowResizeEventFilter, Window);
  while(Running) {
    SDL_Event Event;
    if(SDL_WaitEvent(&Event)) {
      HandleEvent(&Event, Renderer);
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

internal int
WindowResizeEventFilter(void* Data, SDL_Event* Event)
{
  switch(Event->type) {
    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        {
          SDL_Window* Window = SDL_GetWindowFromID(Event->window.windowID);
          // Ensure this event came from our main window.
          if( Window == (SDL_Window*) Data ) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window size changed: (%d x %d)", Event->window.data1, Event->window.data2);

            // Draw something.
            SDL_Renderer* Renderer = SDL_GetRenderer(Window);
            // Toggle between black and white.
            static bool drawWhite = true;
            if (drawWhite) {
              SDL_SetRenderDrawColor(Renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
              drawWhite = false;
            }
            else
              {
                SDL_SetRenderDrawColor(Renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
                drawWhite = true;
              }

          }
        } break;

        // case SDL_WINDOWEVENT_RESIZED:
        // {
        //   SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window resized: (%d x %d)", Event->window.data1, Event->window.data2);
        // } break;
      }
    } break;

    default:
    {
    } break;
  }

  return(0);
}

internal void
HandleEvent(SDL_Event* Event, SDL_Renderer* Renderer)
{
  switch(Event->type) {
    case SDL_QUIT:
    {
      // TODO: Handle this with an error?
      Running = false;
    } break;

    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window gained focus");
        } break;

        case SDL_WINDOWEVENT_CLOSE:
        {
          // TODO: Handle this with message to the user?
          Running = false;
        } break;

        case SDL_WINDOWEVENT_EXPOSED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window exposed");

          // Redraw only if the window has been exposed
          SDL_RenderClear(Renderer);
          SDL_RenderPresent(Renderer);
        } break;
      }
    } break;

    default:
    {
      // SDL_LogDebug("default");
    } break;
  }
}
