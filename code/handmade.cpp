#include "handmade.h"

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
  // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "painting pixels");
  uint8 *Row = (uint8 *)Buffer->Memory;
  for(int Y = 0;
      Y < Buffer->Height;
      ++Y)
    {
      uint32 *Pixel = (uint32 *)Row;
      for(int X = 0;
          X < Buffer->Width;
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
      Row += Buffer->Pitch;
    }
}

internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
  RenderWeirdGradient(Buffer, BlueOffset, GreenOffset);
}
