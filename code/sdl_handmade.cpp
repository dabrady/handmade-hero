#include <SDL.h>
#include <stdlib.h>

/* Macros */

// The many faces of 'static'
#define internal static
#define local_persist static
#define global_variable static

/* Globals */
struct sdl_offscreen_buffer
{
  SDL_Texture *Texture;
  void *Memory;
  int Width;
  int Height;
  int Pitch;
  int BytesPerPixel;
};

struct sdl_window_dimension
{
  int Width;
  int Height;
};

global_variable bool Running;
global_variable sdl_offscreen_buffer GlobalBackBuffer;

/* Forward declarations */
internal int SDLWindowResizeEventFilter(void *Data, SDL_Event *Event);
internal void SDLHandleEvent(SDL_Event *Event);
internal sdl_window_dimension SDLGetWindowDimension(SDL_Window *Window);
internal void SDLResizeBuffer(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height);
internal void SDLDisplayBufferInWindow(sdl_offscreen_buffer Buffer, SDL_Renderer *Renderer);
internal void RenderWeirdGradient(sdl_offscreen_buffer Buffer, int XOffset, int YOffset);

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
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized");

  // Shutdown SDL on exit
  atexit(SDL_Quit);

  // Setup initial Window
  Uint32 WindowFlags =
    SDL_WINDOW_RESIZABLE;
  SDL_Window *Window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        1280,                     // int         w,
                                        720,                     // int         h,
                                        WindowFlags              // Uint32      flags
                                        );
  // Check if the window creation was successful.
  if (Window == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);
  }
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  Uint32 RenderFlags = 0;
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, AUTODETECT_DRIVER, RenderFlags);
  if(Renderer == NULL) {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create rendering context: %s", SDL_GetError());
    return(1);
  }

  sdl_window_dimension Dimension = SDLGetWindowDimension(Window);
  SDLResizeBuffer(&GlobalBackBuffer, Renderer, Dimension.Width, Dimension.Height);

  // Main event loop
  Running = true;
  SDL_AddEventWatch(SDLWindowResizeEventFilter, Window);
  int XOffset = 0;
  int YOffset = 0;
  while(Running) {
    SDL_Event Event;
    while(SDL_PollEvent(&Event)) {
      if(Event.type == SDL_QUIT)
      {
        Running = false;
      }
      SDLHandleEvent(&Event);
    }

    RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
    SDLDisplayBufferInWindow(GlobalBackBuffer, Renderer);
    ++XOffset;
  }

  return(0);
}

// ********

internal int
SDLWindowResizeEventFilter(void *Data, SDL_Event *Event)
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
            // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window size changed: (%d x %d)", Event->window.data1, Event->window.data2);

            int Width = Event->window.data1;
            int Height = Event->window.data2;
            // SDL_GetWindowSize(Window, &Width, &Height);

            // Update our buffer for next paint.
            SDL_Renderer *Renderer = SDL_GetRenderer(Window);
            SDLResizeBuffer(&GlobalBackBuffer, Renderer, Width, Height);
          }
        } break;

        // case SDL_WINDOWEVENT_RESIZED:
        // {
          // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Window resized: (%d x %d)", Event->window.data1, Event->window.data2);
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
SDLHandleEvent(SDL_Event *Event)
{
  switch(Event->type) {
    // case SDL_QUIT:
    // {
    //   // TODO: Handle this with an error?
    //   Running = false;
    // } break;

    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        {
          // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window gained focus");
        } break;

        case SDL_WINDOWEVENT_CLOSE:
        {
          // TODO: Handle this with message to the user?
          Running = false;
        } break;

        // This is the main "paint" event.
        case SDL_WINDOWEVENT_EXPOSED:
        {
          // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "window exposed");

          SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
          SDL_Renderer *Renderer = SDL_GetRenderer(Window);
          SDLDisplayBufferInWindow(GlobalBackBuffer, Renderer);
        } break;
      }
    } break;

    default:
    {
      // SDL_LogDebug("default");
    } break;
  }
}

internal sdl_window_dimension
SDLGetWindowDimension(SDL_Window *Window)
{
  sdl_window_dimension Result;
  SDL_GetWindowSize(Window, &Result.Width, &Result.Height);
  return(Result);
}

internal void
SDLResizeBuffer(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height)
{
  // TODO: Bulletproof this.
  // Maybe don't free first, free after, then free first if that fails.

  // Free any previously created memory.
  if (Buffer->Texture)
  {
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "texture exists: destroying");
    SDL_DestroyTexture(Buffer->Texture);
  }

  if (Buffer->Memory)
  {
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "bitmap exits: freeing");
    free(Buffer->Memory);
  }

  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "creating new texture");
  // Create new texture buffer.
  Buffer->Texture = SDL_CreateTexture(Renderer,
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

  Buffer->Width = Width;
  Buffer->Height = Height;
  Buffer->BytesPerPixel = 4;
  Buffer->Pitch = Width * Buffer->BytesPerPixel; // number of bytes in a row of pixels

  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "allocating new bitmap memory");
  int BitmapMemorySize = (Width * Height) * Buffer->BytesPerPixel;
  Buffer->Memory = malloc(BitmapMemorySize);

  RenderWeirdGradient(GlobalBackBuffer, 0,0);
}

internal void
SDLDisplayBufferInWindow(sdl_offscreen_buffer Buffer, SDL_Renderer *Renderer)
{
  // Give our new texture fresh pixel data.
  // TODO: Should we use SDL_{Lock,Unlock}Texture instead?
  if (SDL_UpdateTexture(Buffer.Texture, NULL, Buffer.Memory, Buffer.Pitch))
  {
    // TODO: Handle error.
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error updating texture map: %s", SDL_GetError());
  }
  // Copy the texture to the screen.
  SDL_RenderCopy(Renderer,
                 Buffer.Texture,
                 NULL, // Source rectangle (NULL means whole texture)
                 NULL  // Destination rectangle (NULL means whole texture)
                 );
  // Show it.
  SDL_RenderPresent(Renderer);
}

internal void
RenderWeirdGradient(sdl_offscreen_buffer Buffer, int XOffset, int YOffset)
{
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "painting pixels");
  Uint8 *Row = (Uint8 *)Buffer.Memory;
  for(int Y = 0;
      Y < Buffer.Height;
      ++Y)
  {
    Uint32 *Pixel = (Uint32 *)Row;
    for(int X = 0;
        X < Buffer.Width;
        ++X)
    {
      /*
       * Pixel (32 bits) structure:
       *
       * Memory:    RR GG BB xx
       * Register:  xx GG BB RR
       */
      Uint8 Blue = (X + XOffset);
      Uint8 Green = (Y + YOffset);
      Uint8 Red = 0;
      // Uint8 Padding = 0;

      // Write the pixel to our buffer.
      *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
    }

    // Move to the next row.
    Row += Buffer.Pitch;
  }
}
