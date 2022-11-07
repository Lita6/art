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
	
	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
	typedef int64_t s64;
	
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;
	
	typedef s32 b32;
	typedef size_t memory_index;
	
	typedef struct
	{
		int Placeholder;
	}thread_context;
	
#define internal static
#define local_persist static
#define global_variable static
	
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
		s32 HalfTransitionCount;
		b32 EndedDown;
	}game_button_state;
	
	typedef struct
	{
		b32 IsConnected;
		
		union
		{
			game_button_state Buttons[12];
			struct
			{
				game_button_state MoveUp;
				game_button_state MoveDown;
				game_button_state MoveLeft;
				game_button_state MoveRight;
				
				game_button_state ActionUp;
				game_button_state ActionDown;
				game_button_state ActionLeft;
				game_button_state ActionRight;
				
				game_button_state LeftShoulder;
				game_button_state RightShoulder;
				
				game_button_state Back;
				game_button_state Start;
				
				// NOTE: All buttons must be added above this line
				
				game_button_state Terminator;
			};
		};
	}game_controller_input;
	
	typedef struct
	{
		game_button_state MouseButtons[5];
		s32 MouseX, MouseY, MouseZ;
		game_controller_input Controller;
	}game_input;
	
	typedef struct
	{
		b32 IsInitialized;
		
		u64 PermanentStorageSize;
		void *PermanentStorage;
		
		u64 TransientStorageSize;
		void *TransientStorage;
		
	}game_memory;
	
#ifdef __cplusplus
}
#endif

#endif //ART_PLATFORM_H
