#include <SDL.h>
#include <stdlib.h>

/* Macros */

// The many faces of 'static'
#define internal static
#define local_persist static
#define global_variable static

// The size of a pixel, in bytes
#define BYTES_PER_PIXEL 4

/* Globals */
global_variable bool Running;
global_variable SDL_Texture *Texture;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

/* Forward declarations */
internal int WindowResizeEventFilter(void *, SDL_Event *);
internal void HandleEvent(SDL_Event *);
internal void ResizeTexture(SDL_Renderer *, int, int);
internal void UpdateWindow(SDL_Renderer *);

int main(int ArgCount, char **ArgValues)
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
  SDL_Window *Window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        640,                     // int         w,
                                        480,                     // int         h,
                                        WindowFlags              // Uint32      flags
                                        );
  // Check if the window creation was successful.
  if (Window == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);
  }
  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  Uint32 RenderFlags = 0;
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, AUTODETECT_DRIVER, RenderFlags);
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
      HandleEvent(&Event);
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
WindowResizeEventFilter(void *Data, SDL_Event *Event)
{
  switch(Event->type) {
    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        {
          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          // Ensure this event came from our main window.
          if( Window == (SDL_Window *) Data ) {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window size changed: (%d x %d)", Event->window.data1, Event->window.data2);

            int Width = Event->window.data1;
            int Height = Event->window.data2;
            // SDL_GetWindowSize(Window, &Width, &Height);

            // Update our buffer for next paint.
            SDL_Renderer *Renderer = SDL_GetRenderer(Window);
            ResizeTexture(Renderer, Width, Height);
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
HandleEvent(SDL_Event *Event)
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

        // This is the main "paint" event.
        case SDL_WINDOWEVENT_EXPOSED:
        {
          SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window exposed");

          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);

          if(NULL == Texture)
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "no texture creating");
            int Width, Height;
            SDL_GetWindowSize(Window, &Width, &Height);
            ResizeTexture(Renderer, Width, Height);
          }
          UpdateWindow(Renderer);

          // Redraw only if the window has been exposed
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

internal void
ResizeTexture(SDL_Renderer *Renderer, int Width, int Height)
{
  // TODO: Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  // Free any previously created memory.
  if (Texture)
  {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "texture exists: destroying");
    SDL_DestroyTexture(Texture);
  }

  if (BitmapMemory)
  {
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "bitmap exits: freeing");
    free(BitmapMemory);
  }

  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "creating new texture");
  // Create new texture buffer.
  Texture = SDL_CreateTexture(Renderer,
                              /**
                               * This describes the format of our pixel data.
                               * We are going to use 32 bits (4 bytes) for each pixel:
                               * 8 bits (1 byte) each for our Red, Green, Blue, and Alpha values.
                               *
                               * A big endian format (RGBA) would likely be more readable, but
                               * to follow along with Casey's lessons, I'm choosing to use a
                               * little endian forat to match Windows-style architecture.
                               */
                              SDL_PIXELFORMAT_BGRA32,
                              // A hint for the graphics driver about how we will access the texture.
                              SDL_TEXTUREACCESS_STREAMING,
                              Width,
                              Height);

  BitmapWidth = Width;
  BitmapHeight = Height;

  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "allocating new bitmap memory");
  int BitmapMemorySize = (Width * Height) * BYTES_PER_PIXEL;
  BitmapMemory = malloc(BitmapMemorySize);

  SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "painting pixels");
  int Pitch = Width * BYTES_PER_PIXEL; // number of bytes in a row of pixels
  Uint8 *Row = (Uint8 *)BitmapMemory;
  for(int Y = 0;
      Y < BitmapHeight;
      ++Y)
  {
    Uint8 *Pixel = (Uint8 *)Row;
    for(int X = 0;
        X < BitmapWidth;
        ++X)
    {
      /*
       * Pixel in memory:  BB GG RR xx
       *
       * 0x xxRRGGBB
       */
      *Pixel = 0;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;

      *Pixel = 255;
      ++Pixel;

      *Pixel = 0;
      ++Pixel;
    }

    // Move pointer to the next row.
    Row += Pitch;
  }

  // Give our new texture fresh pixel data.
  // TODO: Should we use SDL_{Lock,Unlock}Texture instead?
  if (SDL_UpdateTexture(Texture, NULL, BitmapMemory, Pitch))
  {
    // TODO: Handle error.
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error updating texture map: %s", SDL_GetError());
  }
}

internal void
UpdateWindow(SDL_Renderer *Renderer)
{
  // Copy the texture to the screen.
  SDL_RenderCopy(Renderer,
                 Texture,
                 NULL, // Source rectangle (NULL means whole texture)
                 NULL  // Destination rectangle (NULL means whole texture)
                 );
}
