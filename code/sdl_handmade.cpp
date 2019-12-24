#include <SDL.h>
#include <stdlib.h>

/* Macros */

// The many faces of 'static'
#define internal static
#define local_persist static
#define global_variable static

// The maximum number of game controllers we'll allow to be used at once
#define MAX_CONTROLLERS 4

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
internal void SDLHandleEvent(SDL_Event *Event, int *XOffset, int *YOffset);
internal sdl_window_dimension SDLGetWindowDimension(SDL_Window *Window);
internal void SDLResizeBuffer(sdl_offscreen_buffer *Buffer, SDL_Renderer *Renderer, int Width, int Height);
internal void SDLDisplayBufferInWindow(sdl_offscreen_buffer Buffer, SDL_Renderer *Renderer);
internal void RenderWeirdGradient(sdl_offscreen_buffer Buffer, int XOffset, int YOffset);

// The `Foo *(&X)[BAR]` syntax in a type signature means "X is a reference to an array of pointers to Foo and has a size of BAR".
internal void SDLStartGameControllers(SDL_GameController *(&Controllers)[MAX_CONTROLLERS]);
internal void SDLStopGameControllers(SDL_GameController *(&Controllers)[MAX_CONTROLLERS]);

internal SDL_AudioDeviceID SDLInitializeAudio(int SamplesPerSecond, Uint16 BufferSize);
internal void SDLWriteToSoundBuffer(void* UserData, Uint8* AudioStream, int SampleLength);

int main()
{
#if 1
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

  Uint32 Subsystems =
      SDL_INIT_VIDEO           // graphics and window management
    | SDL_INIT_GAMECONTROLLER  // game controllers and joysticks
    | SDL_INIT_AUDIO;          // audio system

  // Initialize SDL
  if (SDL_InitSubSystem(Subsystems) != 0)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
    return(1);
  }
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "SDL initialized");

  // Shutdown SDL on exit
  atexit(SDL_Quit);

  // Setup audio system
  int AudioSampleRate = 48000; // samples per second
  Uint16 AudioBufferSize = 2 * AudioSampleRate * sizeof(Uint16); // 2 seconds' worth of samples
  SDL_AudioDeviceID AudioDevice = SDLInitializeAudio(AudioSampleRate, AudioBufferSize);
  SDL_PauseAudioDevice(AudioDevice, 0); // audio starts paused: a value of 0 here unpauses it

  // Setup initial Window
  Uint32 WindowFlags =
    SDL_WINDOW_RESIZABLE;
  SDL_Window *Window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        1280,                    // int         w,
                                        720,                     // int         h,
                                        WindowFlags);            // Uint32      flags
  // Check if the window creation was successful.
  if (Window == NULL)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);
  }
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  Uint32 RenderFlags = 0;
  SDL_Renderer *Renderer = SDL_CreateRenderer(Window, AUTODETECT_DRIVER, RenderFlags);
  if(Renderer == NULL)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create rendering context: %s", SDL_GetError());
    return(1);
  }

  sdl_window_dimension Dimension = SDLGetWindowDimension(Window);
  SDLResizeBuffer(&GlobalBackBuffer, Renderer, Dimension.Width, Dimension.Height);

  // Initialize any game controllers plugged in at the start of our game.
  // TODO: Enable haptics
  SDL_GameController *Controllers[MAX_CONTROLLERS];
  SDLStartGameControllers(Controllers);

  // Main event loop
  Running = true;
  int XOffset = 0;
  int YOffset = 0;
  while(Running)
  {
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
      if(Event.type == SDL_QUIT)
      {
        Running = false;
      }
      SDLHandleEvent(&Event, &XOffset, &YOffset);
    }

    // Input handling
    for (int ControllerIndex = 0;
         ControllerIndex < MAX_CONTROLLERS;
         ++ControllerIndex)
    {
      SDL_GameController *Controller = Controllers[ControllerIndex];
      if (SDL_GameControllerGetAttached(Controller))
      {
        bool Up = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        bool Down = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        bool Left = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        bool Right = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        bool Start = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_START);
        bool Back = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_BACK);
        bool LeftShoulder = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        bool RightShoulder = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        bool AButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_A);
        bool BButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_B);
        bool XButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_X);
        bool YButton = SDL_GameControllerGetButton(Controller, SDL_CONTROLLER_BUTTON_Y);

        Sint16 StickX = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 StickY = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTY);

        // Control side-scrolling with joystick.
        XOffset += StickX >> 8;
        XOffset += StickY >> 8;

        // TODO: Rumble support
      }
      else
      {
        // TODO: Controller not plugged in anymore; do something about it.
      }
    }

    // Screen drawing
    RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);
    SDLDisplayBufferInWindow(GlobalBackBuffer, Renderer);
    // ++XOffset;
  }

  // Clean up our game controllers
  SDLStopGameControllers(Controllers);
  // Close our audio output
  SDL_CloseAudioDevice(AudioDevice);

  return(0);
}

// ********

internal void
SDLHandleEvent(SDL_Event *Event, int* XOffset, int *YOffset)
{
  switch(Event->type)
  {
    case SDL_WINDOWEVENT:
    {
      switch(Event->window.event)
      {
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

    case SDL_KEYUP:
    case SDL_KEYDOWN:
    {
      SDL_Keycode KeyCode = Event->key.keysym.sym;
      bool WasDown = (Event->key.state == SDL_RELEASED) || Event->key.repeat;
      bool IsDown = Event->key.state == SDL_PRESSED;

      if (IsDown != WasDown)
      {
        switch(KeyCode)
        {
          case SDLK_w:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "w");
            *YOffset += 12;
          } break;

          case SDLK_a:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "a");
            *XOffset += 12;
          } break;

          case SDLK_s:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "s");
            *YOffset -= 12;
          } break;

          case SDLK_d:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "d");
            *XOffset -= 12;
          } break;

          case SDLK_q:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "q");
          } break;

          case SDLK_e:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "e");
          } break;

          case SDLK_UP:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "up");
          } break;

          case SDLK_DOWN:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "down");
          } break;

          case SDLK_LEFT:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "left");
          } break;

          case SDLK_RIGHT:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "right");
          } break;

          case SDLK_ESCAPE:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "escape");
          } break;

          case SDLK_SPACE:
          {
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "space");
          } break;

          default:
          {
          } break;
        }
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

internal void
SDLStartGameControllers(SDL_GameController *(&Controllers)[MAX_CONTROLLERS])
{
  int MaxJoysticks = SDL_NumJoysticks();
  int NumControllers = 0;
  for (int JoystickIndex = 0;
       JoystickIndex < MaxJoysticks;
       ++JoystickIndex)
  {
    if (NumControllers >= MAX_CONTROLLERS)
    {
      // Ignore any additional controllers
      break;
    }

    if (!SDL_IsGameController(JoystickIndex))
    {
      // Ignore any unsupported controllers
      continue;
    }

    // Initialize the controller and save a reference to it
    SDL_GameController *Controller = SDL_GameControllerOpen(JoystickIndex);
    Controllers[NumControllers++] = Controller;
  }
}

internal void
SDLStopGameControllers(SDL_GameController *(&Controllers)[MAX_CONTROLLERS])
{
  for (int ControllerIndex = 0;
       ControllerIndex < MAX_CONTROLLERS;
       ++ControllerIndex)
  {
    SDL_GameController *Controller = Controllers[ControllerIndex];
    SDL_GameControllerClose(Controller);
  }
}

internal SDL_AudioDeviceID
SDLInitializeAudio(int SamplesPerSecond, Uint16 BufferSize)
{
  SDL_AudioSpec DesiredSettings, ActualSettings;

  SDL_memset(&DesiredSettings, 0, sizeof(DesiredSettings)); // zero-out the memory
  DesiredSettings.freq = SamplesPerSecond;
  DesiredSettings.format = AUDIO_S16LSB; // signed 16-bit samples in little-endian byte order
  DesiredSettings.channels = 2;
  DesiredSettings.samples = BufferSize;
  DesiredSettings.callback = &SDLWriteToSoundBuffer;

  SDL_AudioDeviceID OpenedAudioDevice = SDL_OpenAudioDevice(0, 0, &DesiredSettings, &ActualSettings, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (!OpenedAudioDevice)
  {
    // TODO: We failed to open the audio device. Do something about it.
  }

  if (ActualSettings.format != DesiredSettings.format)
  {
    // TODO: We didn't get the settings we wanted. Figure out how to handle this.
  }

  return(OpenedAudioDevice);
}

internal void
SDLWriteToSoundBuffer(void* UserData, Uint8* AudioStream, int SampleLength)
{
  // Just write 'silence' for now.
  // TODO: Write actual audio.
  SDL_memset(AudioStream, 0, SampleLength);
}
