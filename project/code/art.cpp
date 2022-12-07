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
	v3 vector_reflection;
	v3 intersection;
};

v3
ReflectRay
(v3 a, v3 b)
{
	v3 result = (2.0 * b * Dot(b, a)) - a;
	return(result);
}

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
(Light *lights, s32 LightCount, Sphere *spheres, s32 SphereCount, v3 Point, v3 SurfaceNormal, v3 vector, s32 specular, r32 MaxViewDistance)
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
		r32 nDotL = Dot(SurfaceNormal, LightVector);
		if(nDotL > 0.0)
		{
			result += lights[i].intensity * (nDotL / (v3Length(SurfaceNormal) * v3Length(LightVector)));
		}
		
		// NOTE: Shiny lighting
		if(specular != -1)
		{
			v3 R = ReflectRay(LightVector, SurfaceNormal);
			r32 rDotV = Dot(R, vector);
			if(rDotV > 0)
			{
				
				result += lights[i].intensity * (Power((rDotV / (v3Length(R) * v3Length(vector))), (u32)specular));
				
			}
		}
	}
	
	return(result);
}

b32
IsReflective
(r32 a)
{
	b32 result = (a < (r32)0.001) ? TRUE : FALSE;
	return(result);
}

Reflection
TraceRay
(Sphere *spheres, s32 SphereCount, Light *lights, s32 LightCount, v3 origin, v3 direction, r32 tMin, Sphere sky)
{
	r32 tMax = sky.radius;
	Reflection result = {};
	
	Sphere *ClosestSphere = 0;
	r32 direction_factor = ClosestIntersection(spheres, SphereCount, origin, direction, tMin, tMax, &ClosestSphere);
	
	if(ClosestSphere == 0)
	{
		//just return the background color
		result.color = sky.color;
		result.reflective = sky.reflective;
	}
	else
	{
		
		result.intersection = origin + (direction_factor * direction);
		v3 surface_normal = result.intersection - ClosestSphere->center;
		surface_normal = surface_normal / v3Length(surface_normal);
		v3 reverse_direction = -1.0 * direction;
		
		result.color = ClosestSphere->color * 
			ComputeLighting(lights, LightCount, spheres, SphereCount, result.intersection, surface_normal, reverse_direction, ClosestSphere->specular, tMax);
		result.reflective = ClosestSphere->reflective;
		
		if(IsReflective(result.reflective))
		{
			result.vector_reflection = ReflectRay(reverse_direction, surface_normal);
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
	
	// Opaque Colors
	Color Black = make_color(255, 0, 0, 0);
	Color Red = make_color(255, 255, 0, 0);
	Color Green = make_color(255, 0, 255, 0);
	Color Blue = make_color(255, 0, 0, 255);
	Color Yellow = make_color(255, 255, 255, 0);
	
	// Transparent Colors meant for the background
	Color bkWhite = make_color(0, 255, 255, 255);
	(void)bkWhite;
	Color bkSkyBlue = make_color(0, 135, 205, 235);
	(void)bkSkyBlue;
	Color bkBlack = {};
	(void)bkBlack;
	
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
	
	Sphere sky = {};
	sky.radius = 10000.0;
	sky.color = bkWhite;
	sky.specular = -1;
	sky.reflective = 0.0;
	for(s32 y = ScreenMinY; y < ScreenMaxY; y++)
	{
		for(s32 x = ScreenMinX; x < ScreenMaxX; x++)
		{
			// NOTE: This is not just its location. Because the camera is at (0,0,0), it's also the vector for the ray we want to trace.
			v3 viewport = {};
			viewport.x = (r32)x * (ViewWidth/(r32)Buffer->Width);
			viewport.y = (r32)y * (ViewHeight/(r32)Buffer->Height);
			viewport.z = ViewDistanceFromCamera;
			
			Color drawColor = sky.color;
			v3 SphereCenter = sky.center;
			Reflection reflections[3] = {};
			s32 ReflectionCount = 0;
			v3 origin = camera;
			v3 direction = viewport;
			r32 tMin = viewport.z;
			
			for(s32 i = 0; i < 2; i++)
			{
				reflections[i] = TraceRay(spheres, SphereCount, lights, LightCount, origin, direction, tMin, sky);
				
				if(!IsReflective(reflections[i].reflective))
				{
					break;
				}
				
				origin = reflections[i].intersection;
				direction = reflections[i].vector_reflection;
				tMin = (r32)0.001;
			}
			
			if(ReflectionCount > 1)
			{
				Color reflected_color = {};
				reflections[(ReflectionCount - 1)].reflective = 0.0;
				for(s32 i = (ReflectionCount - 1); i >= 0; i--)
				{
					if(IsReflective(reflections[i].reflective))
					{
						reflected_color = reflections[i].color * ((r32)1.0 - reflections[i].reflective) + reflected_color * reflections[i].reflective;
					}
					else
					{
						reflected_color = reflections[i].color;
					}
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
