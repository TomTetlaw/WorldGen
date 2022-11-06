#include "Includes.h"

s8 *GetWindowsErrorMessage() {
    s32 ErrorCode = GetLastError();
    
    s8 *Dest = 0;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   0, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&Dest, 0, 0);
    
    return Dest;
}

void *PlatformAllocate(s64 Size) {
    void *Memory = VirtualAlloc(0, Size, MEM_COMMIT, PAGE_READWRITE);
    if(!Memory) {
        PlatformFatalError("Failed to allocate %d bytes", Size);
        return 0;
    }
    
    return Memory;
}

void PlatformFree(void *Pointer) {
    if(!VirtualFree(Pointer, 0, MEM_DECOMMIT | MEM_RELEASE)) {
        PlatformFatalError("Failed to free memory at %x", Pointer);
    }
}

file PlatformLoadFile(s8 *Path, bool ZeroTerminate, arena *Arena) {
    file File = { 0, 0 };
    
    HANDLE Handle = 0;
    Handle = CreateFile(Path, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(Handle == INVALID_HANDLE_VALUE) {
        PlatformFatalError("Failed to open file %s", Path);
        return File;
    }
    
    s32 FileSize = GetFileSize(Handle, 0);
    if(ZeroTerminate) FileSize += 1;
    
    void *Data = ArenaAllocate(Arena, FileSize);
    Zero(Data, FileSize);
    
    ReadFile(Handle, Data, FileSize, 0, 0);
    CloseHandle(Handle);
    
    File.Data = (s8 *)Data;
    File.Length = FileSize;
    
    return File;
}

void PlatformDeleteFile(s8 *Path) {
    DeleteFile(Path);
}

void PlatformWriteFile(s8 *Path, void *Data, s64 Size) {
    HANDLE Handle = 0;
    
    Handle = CreateFile(Path, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(Handle == INVALID_HANDLE_VALUE) {
        PlatformFatalError("Failed to open file for write %s", Path);
        return;
    }
    
    WriteFile(Handle, Data, Size, 0, 0);
    CloseHandle(Handle);
}

void PlatformAppendFile(s8 *Path, void *Data, s64 Size) {
    HANDLE Handle = 0;
    
    Handle = CreateFile(Path, FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(Handle == INVALID_HANDLE_VALUE) {
        PlatformFatalError("Failed to open file for append %s", Path);
        return;
    }
    
    WriteFile(Handle, Data, Size, 0, 0);
    CloseHandle(Handle);
}

void PlatformFatalError(s8 *Text, ...) {
    s8 Buffer[1024] = {0};
    va_list Args;
    
    va_start(Args, Text);
    stb_vsnprintf(Buffer, 1024, Text, Args);
    va_end(Args);
    
    MessageBox(0, Buffer, "Fatal Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
    
#ifdef DEBUG_BUILD
#if 0
    DebugBreak();
#endif
#endif
    ExitProcess(0);
}

f32 PlatformTime() {
    static bool First = true;
    static LARGE_INTEGER Frequency;
    static LARGE_INTEGER StartTime;
    
    if(First) {
        First = false;
        QueryPerformanceFrequency(&Frequency);
        QueryPerformanceCounter(&StartTime);
    }
    
    LARGE_INTEGER Counter;
    QueryPerformanceCounter(&Counter);
    
    LARGE_INTEGER Elapsed;
    Elapsed.QuadPart = Counter.QuadPart - StartTime.QuadPart;
    
    return Elapsed.QuadPart / (f32)Frequency.QuadPart;
}