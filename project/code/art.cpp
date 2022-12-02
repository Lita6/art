#include "art.h"

struct Sphere
{
	v3 center;
	r32 radius;
	Color color;
};

extern "C" 
void
GameUpdateAndRender
(game_memory *Memory, game_offscreen_buffer *Buffer)
{
	
	ScreenMinX = (s32)((Buffer->Width / 2) * -1);
	ScreenMaxX = (s32)(Buffer->Width / 2);
	ScreenMinY = (s32)((Buffer->Height / 2) * -1);
	ScreenMaxY = (s32)(Buffer->Height / 2);
	
	memory_arena TransientMemory = {};
	InitializeArena(&TransientMemory, (memory_index)Memory->TransientStorageSize, (u8*)Memory->TransientStorage);
	
	Color Black = make_color(255, 0, 0, 0);
	Color Red = make_color(255, 255, 0, 0);
	Color Green = make_color(255, 0, 255, 0);
	Color Blue = make_color(255, 0, 0, 255);
	
	// NOTE: Clear to black
	for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
	{
		for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
		{
			PutPixel(Buffer, x, y, Blue);
		}
	}
	
	// NOTE: Camera x, y, and z really are 0.
	v3 camera = {};
	
	r32 ViewWidth = 1.0; //((r32)Buffer->Width/(r32)Buffer->Height);
	r32 ViewHeight = 1.0;
	r32 ViewDistanceFromCamera = 1.0;
	
#define SphereCount 3
	Sphere spheres[SphereCount] = {};
	
	spheres[0].center.x = 0.0;
	spheres[0].center.y = -1.0;
	spheres[0].center.z = 3.0;
	spheres[0].radius = 1.0;
	spheres[0].color = Red;
	
	spheres[1].center.x = 2.0;
	spheres[1].center.y = 0.0;
	spheres[1].center.z = 4.0;
	spheres[1].radius = 1.0;
	spheres[1].color = Blue;
	
	spheres[2].center.x = -2.0;
	spheres[2].center.y = 0.0;
	spheres[2].center.z = 4.0;
	spheres[2].radius = 1.0;
	spheres[2].color = Green;
	
	Color background = make_color(255, 255, 255, 255);
	
	r32 MaxViewDistance = 10.0;
	for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
	{
		for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
		{
			// NOTE: This is not just its location. Because the camera is at (0,0,0), it's also the vector for the ray we want to trace. (Do I want to normalize it? Is it already normalized?)
			v3 viewport = {};
			viewport.x = (r32)x * (ViewWidth/(r32)Buffer->Width);
			viewport.y = (r32)y * (ViewHeight/(r32)Buffer->Height);
			viewport.z = ViewDistanceFromCamera;
			
			Color drawColor = background;
			r32 closest = MaxViewDistance;
			for(s32 i = 0; i < SphereCount; i++)
			{
				r32 r = spheres[i].radius;
				v3 CO = camera - spheres[i].center;
				
				r32 a = DotProduct(viewport, viewport);
				r32 b = 2 * DotProduct(CO, viewport);
				r32 c = DotProduct(CO, CO) - r * r;
				
				r32 discriminant = b*b - 4*a*c;
				
				if(discriminant >= 0)
				{
					r32 t1 = (r32)(((b * -1.0) + sqrtr32(discriminant)) / (2 * a));
					r32 t2 = (r32)(((b * -1.0) - sqrtr32(discriminant)) / (2 * a));
					
					if(((t1 >= 1.0) && (t1 < MaxViewDistance)) || ((t2 >= 1.0) && (t2 < MaxViewDistance)))
					{
						if((t1 < closest) || (t2 < closest))
						{
							closest = (t1 < t2) ? t1 : t2;
							drawColor = spheres[i].color;
						}
					}
				}
			}
			
			PutPixel(Buffer, x, y, drawColor);
			
		}
	}
	
}
