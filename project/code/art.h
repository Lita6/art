/* date = Novembr 6th 2022 9:02 pm */

#ifndef ART_H

#include "art_vector.h"

global s32 ScreenMinX;
global s32 ScreenMaxX;
global s32 ScreenMinY;
global s32 ScreenMaxY;

struct Color
{
	u8 alpha;
	u8 red;
	u8 green;
	u8 blue;
};

Color
make_color
(u8 alpha, u8 red, u8 green, u8 blue)
{
	Color result = {};
	result.alpha = alpha;
	result.red = red;
	result.green = green;
	result.blue = blue;
	return(result);
}

Color
operator*
(r32 intensity, Color color)
{
	
	Color result = {};
	result.red = ClampS32ToU8((s32)((r32)color.red * intensity));
	result.green = ClampS32ToU8((s32)((r32)color.green * intensity));
	result.blue = ClampS32ToU8((s32)((r32)color.blue * intensity));
	result.alpha = color.alpha;
	return(result);
}

Color
operator*
(Color color, r32 intensity)
{
	Color result = intensity*color;
	return(result);
}

Color
operator+
(Color a, Color b)
{
	
	Color result = {};
	result.alpha = (u8)(((s32)a.alpha + (s32)b.alpha) / 2);
	result.red = ClampS32ToU8(((s32)a.red + (s32)b.red));
	result.green = ClampS32ToU8(((s32)a.green + (s32)b.green));
	result.blue = ClampS32ToU8(((s32)a.blue + (s32)b.blue));
	return(result);
}

void
PutPixel
(game_offscreen_buffer *Buffer, s32 x, s32 y, Color output)
{
	u32 color = (u32)(output.alpha << 24) | (u32)(output.red << 16) | (u32)(output.green << 8) | (u32)(output.blue << 0);
	
	Assert((x >= ScreenMinX) && (x < ScreenMaxX));
	Assert((y >= ScreenMinY) && (y < ScreenMaxY));
	
	// NOTE: This puts 0, 0 at the center of the screen
	s32 CanvasX = ScreenMaxX + x;
	s32 CanvasY = ScreenMaxY + y;
	
	Assert((CanvasX >= 0) && (CanvasX < Buffer->Width));
	Assert((CanvasY >= 0) && (CanvasY < Buffer->Height));
	
	u32 *Pixel = (u32 *)((u8 *)Buffer->Memory + ((u32)CanvasX * (u32)Buffer->BytesPerPixel) + ((u32)CanvasY * (u32)Buffer->Pitch));
	*Pixel = color;
}

struct Border
{
	u32 thickness;
	Color color;
};

struct Rect
{
	s32 x;
	s32 y;
	s32 width;
	s32 height;
	b32 filled;
	Color color;
	Border outline;
};

Rect
rect
(s32 x, s32 y, s32 width, s32 height, b32 filled, Color color, u32 outlineThickness, Color outlineColor)
{
	Rect result = {};
	result.x = x;
	result.y = y;
	result.width = width;
	result.height = height;
	result.filled = filled;
	result.color = color;
	result.outline.thickness = outlineThickness;
	result.outline.color = outlineColor;
	return(result);
}

void
DrawRect
(game_offscreen_buffer *Buffer, Rect rectangle)
{
	s32 secX = rectangle.x + rectangle.width;
	s32 secY = rectangle.y + rectangle.height;
	
	s32 minX = (rectangle.x < secX) ? rectangle.x : secX;
	s32 maxX = (rectangle.x > secX) ? rectangle.x : secX;
	s32 minY = (rectangle.y < secY) ? rectangle.y : secY;
	s32 maxY = (rectangle.y > secY) ? rectangle.y : secY;
	
	if(minX < ScreenMinX)
	{
		minX = ScreenMinX;
	}
	
	if(minY < ScreenMinY)
	{
		minY = ScreenMinY;
	}
	
	if(maxX > ScreenMaxX)
	{
		maxX = ScreenMaxX;
	}
	
	if(maxY > ScreenMaxY)
	{
		maxY = ScreenMaxY;
	}
	
	if(rectangle.filled == TRUE)
	{
		for(s32 y = minY; y < maxY; y++)
		{
			for(s32 x = minX; x < maxX; x++)
			{
				PutPixel(Buffer, x, y, rectangle.color);
			}
		}
	}
	
	if(rectangle.outline.thickness > 0)
	{
		
		Rect bottom = rect(minX, minY, maxX, (minY + (s32)rectangle.outline.thickness), 1, rectangle.outline.color, 0, make_color(0,0,0,0));
		DrawRect(Buffer, bottom);
		
		Rect left = rect(minX, minY, (minX + (s32)rectangle.outline.thickness), maxY, 1, rectangle.outline.color, 0, make_color(0,0,0,0));
		DrawRect(Buffer, left);
		
		Rect top = rect(minX, maxY, maxX, (maxY - (s32)rectangle.outline.thickness), 1, rectangle.outline.color, 0, make_color(0,0,0,0));
		DrawRect(Buffer, top);
		
		Rect right = rect(maxX, maxY, (maxX - (s32)rectangle.outline.thickness), minY, 1, rectangle.outline.color, 0, make_color(0,0,0,0));
		DrawRect(Buffer, right);
		
	}
}

struct Line
{
	v2 start;
	v2 end;
	u32 color;
};

struct memory_arena
{
	memory_index Size;
	u8 *Base;
	memory_index Used;
};

internal void
InitializeArena
(memory_arena *Arena, memory_index Size, u8 *Base)
{
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count * sizeof(type)))
void *
PushSize_
(memory_arena *Arena, memory_index Size)
{
	Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;
	
	return(Result);
}

#define ART_H
#endif //ART_H
