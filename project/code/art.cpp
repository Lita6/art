#include "art.h"

void
PutPixel
(game_offscreen_buffer *Buffer, s32 x, s32 y, u32 color)
{
	Assert((x > -960) && (x < 960));
	Assert((y > -540) && (y < 540));
	
	s32 CanvasX = (Buffer->Width / 2) + x;
	s32 CanvasY = (Buffer->Height / 2) - y;
	
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
	
	// TODO: So I PatBlt blackness to clear the screen and now the screen has a weird flicker. But if I deal with timing, then I know the weird flicker goes away. I stopped dealing with time cause the linker was having a hard time with using the float type for some reason and all the timing was in float. I decided I wasn't going to bother and I got rid of it. Apparently, too soon.
	
	(void)Thread;
	(void)Input;
	
	memory_arena TransientMemory = {};
	InitializeArena(&TransientMemory, (memory_index)Memory->TransientStorageSize, (u8*)Memory->TransientStorage);
	s32 MinX = (s32)(((Buffer->Width / 2) - 1) * -1);
	s32 MinY = (s32)(((Buffer->Height / 2) - 1) * -1);
	s32 MaxX = (s32)(Buffer->Width / 2);
	s32 MaxY = (s32)(Buffer->Height / 2);
	
	for(s32 x = MinX; x < MaxX; x++)
	{
		for(s32 y = MinY; y < MaxY; y++)
		{
			PutPixel(Buffer, x, y, (u32)0xffff0000);
		}
	}
	
	for(s32 x = -10; x < 10; x++)
	{
		for(s32 y = -10; y < 10; y++)
		{
			PutPixel(Buffer, x, y, (u32)0xff0000ff);
		}
	}
}
