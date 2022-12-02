#include "art_platform.h"

#include <windows.h>
#include <stdio.h>

#include "win64_art.h"
#include "art.cpp"

global b32 GlobalRunning;
global b32 GlobalPause;
global win64_offscreen_buffer GlobalBackbuffer;
global WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

internal void
ToggleFullscreen
(HWND Window)
{
	// NOTE: Code from: blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
	
	LONG Style = GetWindowLongA(Window, GWL_STYLE);
	if(Style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
		if(GetWindowPlacement(Window, &GlobalWindowPosition) && GetMonitorInfoA(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
		{
			SetWindowLongA(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(Window, HWND_TOP, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top, MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
									 MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
	}
	else
	{
		SetWindowLongA(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(Window, &GlobalWindowPosition);
		SetWindowPos(Window, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}
}

void
Win64FreeFileMemory
(thread_context *Thread, void *Memory)
{
	(void)Thread;
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

debug_read_file_result
Win64ReadEntireFile
(thread_context *Thread, char *FileName)
{
	debug_read_file_result Result = {};
	
	HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = SafeTruncateU64((u64)FileSize.QuadPart);
			Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Contents)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
				{
					Result.ContentsSize = FileSize32;
					OutputDebugStringA("Read file properly.\n");
				}
				else
				{
					Win64FreeFileMemory(Thread, Result.Contents);
					Result.Contents = 0;
					OutputDebugStringA("Didn't read the file properly.\n");
				}
			}
			else
			{
				OutputDebugStringA("Failed to allocate memory for file read.\n");
			}
		}
		else
		{
			OutputDebugStringA("Failed to get file size after opening.\n");
		}
		
		CloseHandle(FileHandle);
	}
	else
	{
		OutputDebugStringA("Failed to open file.\n");
	}
	
	return(Result);
}

b32
Win54WriteEntireFile
(thread_context *Thread, char *Filename, u32 MemorySize, void *Memory)
{
	(void)Thread;
	
	b32 Result = false;
	
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
			OutputDebugStringA("Wrote file properly.\n");
		}
		else
		{
			OutputDebugStringA("Failed to write file.\n");
		}
		
		CloseHandle(FileHandle);
	}
	else
	{
		OutputDebugStringA("Failed to create file to write.\n");
	}
	
	return(Result);
}

win64_window_dimension
Win64GetWindowDimension
(HWND Window)
{
	win64_window_dimension Result;
	
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	
	return(Result);
}

internal void
Win64ResizeDIBSection(win64_offscreen_buffer *Buffer, int Width, int Height)
{
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}
	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;
	
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	
	int BitmapMemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, (size_t)BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * Buffer->BytesPerPixel;
}

internal void
Win64DisplayBufferInWindow(win64_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
	
	PatBlt(DeviceContext, 0, 0, WindowWidth, WindowHeight, BLACKNESS);
	
	s32 xOffset = (WindowWidth - Buffer->Width) / 2;
	s32 yOffset = (WindowHeight - Buffer->Height) / 2;;
	
	s32 StretchSuccess = StretchDIBits(DeviceContext,
																		 /* Dest */ xOffset, yOffset, Buffer->Width, Buffer->Height,
																		 /* Source */ 0, 0, Buffer->Width, Buffer->Height,
																		 Buffer->Memory, &Buffer->Info,
																		 DIB_RGB_COLORS, SRCCOPY);
	
	if((!StretchSuccess) || StretchSuccess == GDI_ERROR)
	{
		OutputDebugStringA("StretchDIBits failed.\n");
	}
}

internal LRESULT CALLBACK
Win64MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
	LRESULT Result = 0;
	
	switch(Message)
	{
		case WM_CLOSE:
		{
			GlobalRunning = false;
		}break;
		
		case WM_ACTIVATEAPP:
		{
#if 0
			if(WParam == TRUE)
			{
				SetLayeredWindowAttributes(Window, RGB(0,0,0), 255, LWA_ALPHA);
			}
			else
			{
				SetLayeredWindowAttributes(Window, RGB(0,0,0), 64, LWA_ALPHA);
			}
#endif
		}break;
		
		case WM_DESTROY:
		{
			GlobalRunning = false;
		}break;
		
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Assert(!"Keyboard input came in through a non-dispatch message!\n");
		}break;
		
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			win64_window_dimension Dimension = Win64GetWindowDimension(Window);
			Win64DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
			EndPaint(Window, &Paint);
		}break;
		
		default:
		{
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		}break;
	}
	
	return(Result);
}

internal void
Win64ProcessPendingMessages()
{
	MSG Message;
	while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		switch (Message.message)
		{
			case WM_QUIT:
			{
				GlobalRunning = false;
			}break;
			
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 VKCode = (u32)Message.wParam;
				b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
				b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
				if(WasDown != IsDown)
				{
					if(VKCode == 'W')
					{
						
					}
					else if(VKCode == 'A')
					{
						
					}
					else if(VKCode == 'S')
					{
						
					}
					else if(VKCode == 'D')
					{
						
					}
					else if(VKCode == 'Q')
					{
						
					}
					else if(VKCode == 'E')
					{
						
					}
					else if(VKCode == VK_UP)
					{
						
					}
					else if(VKCode == VK_LEFT)
					{
						
					}
					else if(VKCode == VK_DOWN)
					{
						
					}
					else if(VKCode == VK_RIGHT)
					{
						
					}
					else if(VKCode == VK_ESCAPE)
					{
						if(IsDown)
						{
							GlobalRunning = false;
						}
					}
					else if(VKCode == VK_SPACE)
					{
						
					}
					
					if(IsDown)
					{                    
						b32 AltKeyWasDown = (Message.lParam & (1 << 29));
						if((VKCode == VK_F4) && AltKeyWasDown)
						{
							GlobalRunning = false;
						}
						if((VKCode == VK_RETURN) && AltKeyWasDown)
						{
							ToggleFullscreen(Message.hwnd);
						}
					}
					
				}
			}break;
			
			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}break;
		}
	}
}

int __stdcall
WinMainCRTStartup
(void)
{
	HINSTANCE Instance = (HINSTANCE)GetModuleHandle(0);
	Assert(Instance != 0);
	
	win64_state Win64State = {};
	
	Win64ResizeDIBSection(&GlobalBackbuffer, /*Width*/1000, /*Height*/1000);
	
	WNDCLASSA WindowClass = {};
	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win64MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "ArtProgramWindowClass";
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	
	if(RegisterClassA(&WindowClass))
	{
		HWND Window = 
			CreateWindowExA(
											0,
											WindowClass.lpszClassName,
											"Art Program",
											WS_OVERLAPPEDWINDOW | WS_VISIBLE,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
											0,
											0,
											Instance,
											0);
		
		if(Window)
		{
			
			GlobalRunning = true;
			
			LPVOID BaseAddress = 0;
			
			game_memory GameMemory = {};
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.TransientStorageSize = Gigabytes(1);
			
			Win64State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
			Win64State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win64State.TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			GameMemory.PermanentStorage = Win64State.GameMemoryBlock;
			GameMemory.TransientStorage = (u8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize;
			
			if(GameMemory.PermanentStorage && GameMemory.TransientStorage)
			{
				ToggleFullscreen(Window);
				while(GlobalRunning)
				{
					
					Win64ProcessPendingMessages();
					if(!GlobalPause)
					{
						//POINT MouseP;
						//GetCursorPos(&MouseP);
						//ScreenToClient(Window, &MouseP);
						
						game_offscreen_buffer Buffer = {};
						Buffer.Memory = GlobalBackbuffer.Memory;
						Buffer.Width = GlobalBackbuffer.Width;
						Buffer.Height = GlobalBackbuffer.Height;
						Buffer.Pitch = GlobalBackbuffer.Pitch;
						Buffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;
						Buffer.EndOfBuffer = (u8 *)Buffer.Memory + (Buffer.Pitch * Buffer.Height);
						
						GameUpdateAndRender(&GameMemory, &Buffer);
						
						win64_window_dimension Dimension = Win64GetWindowDimension(Window);
						
						HDC DeviceContext = GetDC(Window);
						Win64DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext, Dimension.Width, Dimension.Height);
						ReleaseDC(Window, DeviceContext);
						
					}
				}
			}
			else
			{
				OutputDebugStringA("Failed to allocate game memory.\n");
			}
		}
		else
		{
			OutputDebugStringA("Failed to get a window handle.\n");
		}
	}
	else
	{
		OutputDebugStringA("Failed to register window class.\n");
	}
	
	return(0);
}