#include <SDL.h>
#include <stdlib.h>
#include <math.h>

/* Macros and type aliases */

// The many faces of 'static'
#define internal static
#define local_persist static
#define global_variable static

// The maximum number of game controllers we'll allow to be used at once
#define MAX_CONTROLLERS 4

// Device mappings for Nintendo Switch "Joy-Con" controllers
#define JOY_CON_L_MAPPING "030000007e0500000620000001000000,Joy-Con (L),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b13,leftshoulder:b4,leftstick:b10,rightshoulder:b5,start:b8,x:b2,y:b3"
// #define JOY_CON_R_MAPPING "030000007e0500000720000001000000,Joy-Con (R),+leftx:h0.2,+lefty:h0.4,-leftx:h0.8,-lefty:h0.1,a:b0,b:b1,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b2,y:b3"

// Alternate mapping: physical letter buttons rotated 1 space counter-clockwise, to form:
//      x
//    y   a
//      b
// And (non?)inverted joystick: up goes up, down goes down, left goes left, right goes right.
#define JOY_CON_R_MAPPING "030000007e0500000720000001000000,Joy-Con (R),+leftx:h0.8,+lefty:h0.1,-leftx:h0.2,-lefty:h0.4,a:b1,b:b0,back:b12,leftshoulder:b4,leftstick:b11,rightshoulder:b5,start:b9,x:b3,y:b2"

#define Pi32 3.14159265359

typedef Sint8 int8;
typedef Sint16 int16;
typedef Sint32 int32;
typedef Sint64 int64;
typedef int32 bool32;

typedef Uint8 uint8;
typedef Uint16 uint16;
typedef Uint32 uint32;
typedef Uint64 uint64;

typedef float real32;
typedef double real64;

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

struct sdl_sound_output
{
  int SamplesPerSecond; // sample rate
  int SampleCount;
  int ToneHz;
  int16 ToneVolume;
  uint32 RunningSampleIndex;
  int WavePeriod;
  int BytesPerSample;
  int BufferSize;
  real32 tSine; // position in the sine wave
  int LatencySampleCount; // how many samples we'll write at once
};

global_variable bool Running;
global_variable sdl_offscreen_buffer GlobalBackBuffer;

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
RenderWeirdGradient(sdl_offscreen_buffer Buffer, int XOffset, int YOffset)
{
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "painting pixels");
  uint8 *Row = (uint8 *)Buffer.Memory;
  for(int Y = 0;
      Y < Buffer.Height;
      ++Y)
  {
    uint32 *Pixel = (uint32 *)Row;
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
      uint8 Blue = (X + XOffset);
      uint8 Green = (Y + YOffset);
      uint8 Red = 0;
      // uint8 Padding = 0;

      // Write the pixel to our buffer.
      *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
    }

    // Move to the next row.
    Row += Buffer.Pitch;
  }
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

// The `Foo *(&X)[BAR]` syntax in a type signature means "X is a reference to an array of pointers to Foo and has a size of BAR".
internal void
SDLStartGameControllers(SDL_GameController *(&Controllers)[MAX_CONTROLLERS])
{
  if (SDL_GameControllerAddMapping(JOY_CON_L_MAPPING) == -1)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to add mapping for Joy-Con (L) controller: %s", SDL_GetError());
  }
  if (SDL_GameControllerAddMapping(JOY_CON_R_MAPPING) == -1)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to add mapping for Joy-Con (R) controller: %s", SDL_GetError());
  }

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

// The `Foo *(&X)[BAR]` syntax in a type signature means "X is a reference to an array of pointers to Foo and has a size of BAR".
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
SDLInitializeAudio(int SamplesPerSecond, uint16 BufferSize)
{
  SDL_AudioSpec DesiredSettings, ActualSettings;

  SDL_memset(&DesiredSettings, 0, sizeof(DesiredSettings)); // zero-out the memory
  DesiredSettings.freq = SamplesPerSecond;
  DesiredSettings.format = AUDIO_S16LSB; // signed 16-bit samples in little-endian byte order
  DesiredSettings.channels = 2;
  DesiredSettings.samples = BufferSize;
  DesiredSettings.callback = NULL;

  SDL_AudioDeviceID OpenedAudioDevice = SDL_OpenAudioDevice(0, 0, &DesiredSettings, &ActualSettings, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (!OpenedAudioDevice)
  {
    // TODO: We failed to open the audio device. Do something about it.
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize audio: %s", SDL_GetError());
  }

  if (ActualSettings.format != DesiredSettings.format)
  {
    // TODO: We didn't get the settings we wanted. Figure out how to handle this.
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize audio with desired format: %s", SDL_GetError());
  }

  return(OpenedAudioDevice);
}

internal void
SDLFillSoundBuffer(SDL_AudioDeviceID AudioDevice, sdl_sound_output* SoundOutput, int BytesToWrite)
{
  if (!BytesToWrite) return;

  void *SoundBuffer = malloc(BytesToWrite);
  int16 *AudioSampleOutput = (int16 *)SoundBuffer;
  for (int AudioSampleIndex = 0;
       AudioSampleIndex < SoundOutput->SampleCount;
       AudioSampleIndex++)
  {
    real32 SineValue = sinf(SoundOutput->tSine);
    int16 AudioSampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
    // Write to both audio channels (i.e. left and right in a stereo setting).
    *AudioSampleOutput++ = AudioSampleValue;
    *AudioSampleOutput++ = AudioSampleValue;

    SoundOutput->tSine += 2.0f * Pi32 * 1.0f/(real32)SoundOutput->WavePeriod;
    ++SoundOutput->RunningSampleIndex;
  }
  SDL_QueueAudio(AudioDevice, SoundBuffer, BytesToWrite);
  free(SoundBuffer);
}

// ********

int main()
{
  uint64 PerfCountFrequency = SDL_GetPerformanceFrequency();

#if 1
  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif

  uint32 Subsystems =
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
  sdl_sound_output SoundOutput = {};
  SoundOutput.SamplesPerSecond = 48000; // sample rate
  SoundOutput.BytesPerSample = sizeof(int16) * 2;
  SoundOutput.SampleCount = SoundOutput.SamplesPerSecond / 60;
  SoundOutput.BufferSize = SoundOutput.SampleCount * SoundOutput.BytesPerSample;
  SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15; // sound latency: 1/15 of a second, aka 4 frames @ 60FPS
  SoundOutput.ToneHz = 256; // close to a middle C note
  SoundOutput.ToneVolume = 7000;
  SoundOutput.RunningSampleIndex = 0;
  SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
  int TargetQueueBytes = SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample;

  SDL_AudioDeviceID AudioDevice = SDLInitializeAudio(SoundOutput.SamplesPerSecond, SoundOutput.SampleCount);
  SDL_PauseAudioDevice(AudioDevice, 0); // audio starts paused: a value of 0 here unpauses it

  // Setup initial Window
  uint32 WindowFlags =
    SDL_WINDOW_RESIZABLE;
  SDL_Window *Window = SDL_CreateWindow("Handmade Hero",         // const char* title,
                                        SDL_WINDOWPOS_UNDEFINED, // int         x,
                                        SDL_WINDOWPOS_UNDEFINED, // int         y,
                                        1280,                    // int         w,
                                        720,                     // int         h,
                                        WindowFlags);            // uint32      flags
  // Check if the window creation was successful.
  if (Window == NULL)
  {
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create main application window: %s", SDL_GetError());
    return(1);
  }
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Application window created");

  // Setup a rendering context.
  int AUTODETECT_DRIVER = -1;
  uint32 RenderFlags = 0;
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

  uint64 LastCounter = SDL_GetPerformanceCounter();
  uint64 LastCycleCount = __rdtsc();

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

        int16 StickX = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTX);
        int16 StickY = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTY);

        // Control side-scrolling with joystick.
        XOffset += StickX / ( 4096 * 2 );
        YOffset += StickY / ( 4096 * 2 );

        // Control pitch with joystick
        SoundOutput.ToneHz = 512 + (int)(256.0f*((real32)StickY / 30000.0f));
        SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;

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

    // Audio generation
    // TODO Why does this introduce sound artifacts?
    // int BytesToWrite = TargetQueueBytes - SDL_GetQueuedAudioSize(AudioDevice);
    int BytesToWrite = SoundOutput.BufferSize;
    SDLFillSoundBuffer(AudioDevice, &SoundOutput, BytesToWrite);

    // Performance measurement / Dimensional analysis
    uint64 EndCycleCount = __rdtsc();

    uint64 EndCounter = SDL_GetPerformanceCounter();
    uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
    uint64 CounterElapsed = EndCounter - LastCounter;
    real32 MSPerFrame = ((1000.0f*(real32)CounterElapsed) / (real32)PerfCountFrequency);
    real32 FPS = ((real32)PerfCountFrequency / (real32)CounterElapsed);
    real32 MCPF = ((real32)CyclesElapsed / (1000.0f*1000.0f));

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%fms/f, %ff/s, %fmc/f", MSPerFrame, FPS, MCPF);

    LastCounter = EndCounter;
    LastCycleCount = EndCycleCount;
  }

  // Clean up our game controllers
  SDLStopGameControllers(Controllers);
  // Close our audio output
  SDL_CloseAudioDevice(AudioDevice);

  return(0);
}
