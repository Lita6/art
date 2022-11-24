#include "art.h"

global s32 MinX;
global s32 MaxX;
global s32 MinY;
global s32 MaxY;

void
PutPixel
(game_offscreen_buffer *Buffer, s32 x, s32 y, u32 color)
{
	Assert((x > MinX) && (x < MaxX));
	Assert((y > MinY) && (y < MaxY));
	
	// NOTE: This puts 0, 0 at the center of the screen and flips y
	s32 CanvasX = MaxX + x;
	s32 CanvasY = MaxY - y;
	
	Assert((CanvasX > 0) && (CanvasX < Buffer->Width));
	Assert((CanvasY > 0) && (CanvasY < Buffer->Height));
	
	u32 *Pixel = (u32 *)((u8 *)Buffer->Memory + ((u32)CanvasX * (u32)Buffer->BytesPerPixel) + ((u32)CanvasY * (u32)Buffer->Pitch));
	*Pixel = color;
}

extern "C" 
void
GameUpdateAndRender
(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
	
	(void)Thread;
	(void)Input;
	
	MinX = (s32)((Buffer->Width / 2) * -1);
	MaxX = (s32)(Buffer->Width / 2);
	MinY = (s32)((Buffer->Height / 2) * -1);
	MaxY = (s32)(Buffer->Height / 2);
	
	memory_arena TransientMemory = {};
	InitializeArena(&TransientMemory, (memory_index)Memory->TransientStorageSize, (u8*)Memory->TransientStorage);
	
	// NOTE: Clear to black
	for(s32 x = (MinX + 1); x < MaxX; x++)
	{
		for(s32 y = (MinY + 1); y < MaxY; y++)
		{
			PutPixel(Buffer, x, y, (u32)0xff000000);
		}
	}
	
}
