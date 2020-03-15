#if !defined(HANDMADE_H)

/*
  TODO: Services that the platform layer provides to the game.
*/

/*
  NOTE: Servces that the game provides to the platform layer.
*/

struct game_offscreen_buffer {
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

#define HANDMADE_H
#endif
