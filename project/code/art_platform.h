/* date = November 6th 2022 9:01 pm */

#ifndef ART_PLATFORM_H
#define ART_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif
	
	/*
	*
	* NOTE: COMPILERS
	*
	*/
	
#ifdef _MSC_VER
#define COMPILER_MSVC 1
#else
#define COMPILER_LLVM 1
#endif
	
	/*
*
* NOTE: TYPES
*
*/
	
#include <stdint.h>
#include <stddef.h>
	
	// NOTE: This is needed for the linker to use floats
	int _fltused;
	
	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
	typedef int64_t s64;
	
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	
#define MAX_U8 0xFF
#define MAX_U16 0XFFFF
#define MAX_U32 0xFFFFFFFF
#define MAX_U64 0XFFFFFFFFFFFFFFFF
	
	typedef s8 b8;
	typedef s16 b16;
	typedef s32 b32;
	
#define TRUE 1
#define FALSE 0
	
	typedef float r32;
	typedef double r64;
	
	typedef size_t memory_index;
	
	typedef struct
	{
		int Placeholder;
	}thread_context;
	
#define internal static
#define local_persist static
#define global static
	
#define Pi32 3.141159265359f
	
#define FAIL 0
	
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
	
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)
	
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
	
	inline u32
		SafeTruncateU64
	(u64 Value)
	{
		Assert(Value <= 0xFFFFFFFF);
		u32 Result = (u32)Value;
		return(Result);
	}
	
	inline u8
		ClampS32ToU8
	(s32 value)
	{
		s32 result = value;
		
		if(value > MAX_U8)
		{
			result = MAX_U8;
		}
		else if(value < 0)
		{
			result = 0;
		}
		
		return((u8)result);
	}
	
	typedef struct
	{
		u32 ContentsSize;
		void *Contents;
	}debug_read_file_result;
	
	typedef struct
	{
		void *Memory;
		int Width;
		int Height;
		int Pitch;
		int BytesPerPixel;
		u8 *EndOfBuffer;
	}game_offscreen_buffer;
	
	typedef struct
	{
		b32 IsInitialized;
		
		u64 PermanentStorageSize;
		void *PermanentStorage;
		
		u64 TransientStorageSize;
		void *TransientStorage;
		
	}game_memory;
	
#if 0	
	struct Float
	{
		u32 base : 23;
		u32 exponent : 8;
		u32 sign : 1;
	};
	
	union uFloat
	{
		Float spliced;
		r32 f;
	};
#endif
	
	r32
		Power
	(r32 base, u32 exponent)
	{
		r32 result = 1.0;
		
		for(u32 i = 0; i < exponent; i++)
		{
			result *= base;
		}
		
		return(result);
	}
	
#ifdef __cplusplus
}
#endif

#endif //ART_PLATFORM_H
