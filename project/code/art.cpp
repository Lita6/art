#include "art.h"

struct Sphere
{
	v3 center;
	r32 radius;
	Color color;
	s32 specular;
	r32 reflective;
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

struct Reflection
{
	Color color;
	r32 reflective;
};

r32
ClosestIntersection
(Sphere *spheres, s32 SphereCount, v3 origin, v3 vector, r32 tMin, r32 tMax, Sphere **result)
{
	
	r32 closest = tMax;
	for(s32 i = 0; i < SphereCount; i++)
	{
		r32 r = spheres[i].radius;
		v3 CO = origin - spheres[i].center;
		
		r32 a = Dot(vector, vector);
		r32 b = 2 * Dot(CO, vector);
		r32 c = Dot(CO, CO) - r * r;
		
		r32 discriminant = b*b - 4*a*c;
		
		if(discriminant >= 0)
		{
			r32 t1 = (r32)(((b * -1.0) + sqrtr32(discriminant)) / (2 * a));
			r32 t2 = (r32)(((b * -1.0) - sqrtr32(discriminant)) / (2 * a));
			
			if(((t1 >= tMin) && (t1 < tMax)) || ((t2 >= tMin) && (t2 < tMax)))
			{
				if((t1 < closest) || (t2 < closest))
				{
					closest = (t1 < t2) ? t1 : t2;
					*result = &spheres[i];
				}
			}
		}
	}
	
	return(closest);
}

r32
ComputeLighting
(Light *lights, s32 LightCount, Sphere *spheres, s32 SphereCount, v3 Point, r32 MaxViewDistance, v3 NormalizedPoint, s32 specular, v3 vector)
{
	r32 result = 0;
	v3 LightVector = {};
	r32 tMax = 0.0;
	
	for(s32 i = 0; i < LightCount; i++)
	{
		
		tMax = MaxViewDistance;
		
		if(lights[i].type == Light_Type_Ambient)
		{
			result += lights[i].intensity;
		}
		else
		{
			if(lights[i].type == Light_Type_Point)
			{
				LightVector = lights[i].vector - Point;
				tMax = 1.0;
				
			}
			else
			{
				LightVector = lights[i].vector;
			}
		}
		
		Sphere *ClosestSphere = {};
		// NOTE: Shadow check
		ClosestIntersection(spheres, SphereCount, Point, LightVector, (r32)0.001, tMax, &ClosestSphere);
		
		if(ClosestSphere != 0)
		{
			continue;
		}
		ClosestSphere = 0;
		
		// NOTE: Diffuse lighting
		r32 nDotL = Dot(NormalizedPoint, LightVector);
		if(nDotL > 0.0)
		{
			result += lights[i].intensity * (nDotL / (v3Length(NormalizedPoint) * v3Length(LightVector)));
		}
		
		// NOTE: Shiny lighting
		if(specular >= 0)
		{
			v3 R = 2.0 * NormalizedPoint * Dot(NormalizedPoint, LightVector) - LightVector;
			v3 V = -1.0 * vector;
			r32 rDotV = Dot(R, V);
			if(rDotV > 0)
			{
#if 0
				// NOTE: Cartoony
				result += lights[i].intensity * (r32)((s32)(rDotV / (v3Length(R) * v3Length(V))) ^ specular);
#else
				result += lights[i].intensity * (Power((rDotV / (v3Length(R) * v3Length(V))), (u32)specular));
#endif
			}
		}
	}
	
	return(result);
}

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
	
	Color bkWhite = make_color(0, 255, 255, 255);
	(void)bkWhite;
	Color bkSkyBlue = make_color(0, 135, 205, 235);
	(void)bkSkyBlue;
	Color bkBlack = {};
	(void)bkBlack;
	
	Color background = bkBlack;
	
	// NOTE: Clear to black
	for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
	{
		for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
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
	spheres[0].reflective = (r32)0.2;
	
	spheres[1].center = V3(2.0, 0.0, 4.0);
	spheres[1].radius = 1.0;
	spheres[1].color = Blue;
	spheres[1].specular = 500;
	spheres[1].reflective = (r32)0.3;
	
	spheres[2].center = V3(-2.0, 0.0, 4.0);
	spheres[2].radius = 1.0;
	spheres[2].color = Green;
	spheres[2].specular = 10;
	spheres[2].reflective = (r32)0.4;
	
	spheres[3].center = V3(0.0, -5001.0, 0.0);
	spheres[3].radius = 5000.0;
	spheres[3].color = Yellow;
	spheres[3].specular = 1000;
	spheres[3].reflective = 0.5;
	
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
	
	// TODO: Specular reflections stopped working for some reason.
	
	r32 MaxViewDistance = 10000.0;
	for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
	{
		for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
		{
			// NOTE: This is not just its location. Because the camera is at (0,0,0), it's also the vector for the ray we want to trace.
			v3 viewport = {};
			viewport.x = (r32)x * (ViewWidth/(r32)Buffer->Width);
			viewport.y = (r32)y * (ViewHeight/(r32)Buffer->Height);
			viewport.z = ViewDistanceFromCamera;
			
			Color drawColor = background;
			s32 specular = -1;
			v3 SphereCenter = {};
			Sphere *ClosestSphere = 0;
			Reflection reflections[3] = {};
			s32 ReflectionCount = 0;
			v3 origin = camera;
			v3 direction = viewport;
			r32 tMin = 1.0;
			for(s32 i = 0; i < 3; i++)
			{
				r32 closest_point = ClosestIntersection(spheres, SphereCount, origin, direction, tMin, MaxViewDistance, &ClosestSphere);
				if(ClosestSphere != 0)
				{
					reflections[i].color = ClosestSphere->color;
					specular = ClosestSphere->specular;
					SphereCenter = ClosestSphere->center;
					reflections[i].reflective = ClosestSphere->reflective;
				}
				else
				{
					reflections[i].color = background;
					specular = -1;
					SphereCenter = {};
				}
				ReflectionCount++;
				
				v3 Point = origin + (closest_point * direction);
				v3 NormalizedPoint = Point - SphereCenter;
				NormalizedPoint = NormalizedPoint / v3Length(NormalizedPoint);
				v3 reverse = -1 * direction;
				
				reflections[i].color = reflections[i].color * ComputeLighting(lights, LightCount, spheres, SphereCount, Point, MaxViewDistance, NormalizedPoint, specular, reverse);
				
				if(reflections[i].reflective <= 0.0)
				{
					break;
				}
				
				origin = Point;
				direction = 2.0 * NormalizedPoint * Dot(NormalizedPoint, reverse) - reverse;
				tMin = (r32)0.001;
				
			}
			
			if(ReflectionCount > 1)
			{
				Color reflected_color = {};
				reflections[(ReflectionCount - 1)].reflective = 0.0;
				for(s32 i = (ReflectionCount - 1); i >= 0; i--)
				{
					reflected_color = reflections[i].color * ((r32)1.0 - reflections[i].reflective) + reflected_color * reflections[i].reflective;
				}
				
				drawColor = reflected_color;
			}
			else
			{
				drawColor = reflections[0].color;
			}
			
			PutPixel(Buffer, x, y, drawColor);
			
		}
	}
	
}
