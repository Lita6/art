#include "art.h"

struct Sphere
{
	v3 center;
	r32 radius;
	Color color;
	s32 specular;
};

enum Light_Type
{
	Light_Type_Ambient,
	Light_Type_Point,
	Light_Type_Directional,
};

struct Light
{
	Light_Type type;
	r32 intensity;
	v3 vector;
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
	Color Yellow = make_color(255, 255, 255, 0);
	
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
	
#define SphereCount 4
	Sphere spheres[SphereCount] = {};
	
	spheres[0].center = V3(0.0, -1.0, 3.0);
	spheres[0].radius = 1.0;
	spheres[0].color = Red;
	spheres[0].specular = 500;
	
	spheres[1].center = V3(2.0, 0.0, 4.0);
	spheres[1].radius = 1.0;
	spheres[1].color = Blue;
	spheres[1].specular = 500;
	
	spheres[2].center = V3(-2.0, 0.0, 4.0);
	spheres[2].radius = 1.0;
	spheres[2].color = Green;
	spheres[2].specular = 10;
	
	spheres[3].center = V3(0.0, -5001.0, 0.0);
	spheres[3].radius = 5000.0;
	spheres[3].color = Yellow;
	spheres[3].specular = 1000;
	
#define LightCount 3
	Light lights[LightCount] = {};
	
	lights[0].type = Light_Type_Ambient;
	lights[0].intensity = (r32)0.2;
	
	lights[1].type = Light_Type_Point;
	lights[1].intensity = (r32)0.6;
	lights[1].vector = V3(2.0, 1.0, 0.0);
	
	lights[2].type = Light_Type_Directional;
	lights[2].intensity = (r32)0.2;
	lights[2].vector = V3(1.0, 4.0, 4.0);
	
	Color background = make_color(255, 255, 255, 255);
	
	r32 MaxViewDistance = 100.0;
	for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
	{
		for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
		{
			// NOTE: This is not just its location. Because the camera is at (0,0,0), it's also the vector for the ray we want to trace.
			v3 viewport = {};
			viewport.x = (r32)x * (ViewWidth/(r32)Buffer->Width);
			viewport.y = (r32)y * (ViewHeight/(r32)Buffer->Height);
			viewport.z = ViewDistanceFromCamera;
			
			Color drawColor = background;
			r32 closest = MaxViewDistance;
			v3 SphereCenter = {};
			s32 specular = -1;
			for(s32 i = 0; i < SphereCount; i++)
			{
				r32 r = spheres[i].radius;
				v3 CO = camera - spheres[i].center;
				
				r32 a = Dot(viewport, viewport);
				r32 b = 2 * Dot(CO, viewport);
				r32 c = Dot(CO, CO) - r * r;
				
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
							SphereCenter = spheres[i].center;
							specular = spheres[i].specular;
						}
					}
				}
			}
			
			v3 P = camera + (closest * viewport);
			v3 N = P - SphereCenter;
			N = N / v3Length(N);
			r32 intensity = 0.0;
			v3 L = {};
			for(s32 i = 0; i < LightCount; i++)
			{
				if(lights[i].type == Light_Type_Ambient)
				{
					intensity += lights[i].intensity;
				}
				else
				{
					if(lights[i].type == Light_Type_Point)
					{
						L = lights[i].vector - P;
					}
					else
					{
						L = lights[i].vector;
					}
				}
				
				// NOTE: Diffuse lighting
				r32 nDotL = Dot(N, L);
				if(nDotL > 0.0)
				{
					intensity += lights[i].intensity * (nDotL / (v3Length(N) * v3Length(L)));
				}
				
				// NOTE: Shiny lighting
				if(specular >= 0)
				{
					v3 R = 2.0 * N * Dot(N, L) - L;
					v3 V = -1.0 * viewport;
					r32 rDotV = Dot(R, V);
					if(rDotV > 0)
					{
#if 0
						// NOTE: Cartoony
						intensity += lights[i].intensity * (r32)((s32)(rDotV / (v3Length(R) * v3Length(V))) ^ specular);
#else
						intensity += lights[i].intensity * (Power((rDotV / (v3Length(R) * v3Length(V))), (u32)specular));
#endif
					}
				}
			}
			
			drawColor = drawColor * intensity;
			PutPixel(Buffer, x, y, drawColor);
			
		}
	}
	
}
