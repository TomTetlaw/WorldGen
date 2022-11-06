#include "Platform.h"

arena GlobalArena;
arena StringArena;
arena RendererArena;
arena TextureArena;

void InitMemory() {
    MakeArena(&GlobalArena, "GlobalArena", GB(2));
    MakeSubArena(&StringArena, &GlobalArena, "StringArena", MB(128));
    MakeSubArena(&RendererArena, &GlobalArena, "RendererArena", MB(128));
    MakeSubArena(&TextureArena, &GlobalArena, "TextureArena", MB(128));
    
    PlatformDeleteFile("Memory.log");
}

void MakeArena(arena *Arena, s8 *Name, s64 Capacity) {
    Zero(Arena, sizeof(arena));
    
    Arena->Data = (s8 *)PlatformAllocate(Capacity);
    Arena->Capacity = Capacity;
    Arena->Name = Name;
    Arena->Mark = -1;
}

void MakeSubArena(arena *Arena, arena *Parent, s8 *Name, s64 Capacity) {
    Zero(Arena, sizeof(arena));
    
    Arena->Data = (s8 *)ArenaAllocate(Parent, Capacity);
    Arena->Capacity = Capacity;
    Arena->Name = Name;
    Arena->Mark = -1;
}

void PushArenaMark(arena *Arena) {
    if(Arena->Mark != -1) {
        PlatformFatalError("ArenaPushMark(%s): Already pushed", Arena->Name);
    }
    
    Arena->Mark = Arena->Used;
}

void PopArenaMark(arena *Arena) {
    if(Arena->Mark == -1) {
        PlatformFatalError("ArenaPopMark(%s): Not pushed", Arena->Name);
        return;
    }
    
    Arena->Used = Arena->Mark;
    Arena->Mark = -1;
}

void GlobalPushMark() { PushArenaMark(&GlobalArena); }
void GlobalPopMark() { PopArenaMark(&GlobalArena); }

void *ArenaAllocateNoZero(arena *Arena, s64 Size) {
    if(Arena->Used + Size >= Arena->Capacity) {
        PlatformFatalError("ArenaAllocate(%s, %d): Not enough space", Arena->Name, Size);
        return 0;
    }
    
    void *Data = (void *)(Arena->Data + Arena->Used);
    Arena->Used += Size;
    
    return Data;
}

void *_ArenaAllocate(arena *Arena, s64 Size) {
    if(Arena->Used + Size >= Arena->Capacity) {
        PlatformFatalError("ArenaAllocate(%s, %d): Not enough space", Arena->Name, Size);
        return 0;
    }
    
    void *Data = (void *)(Arena->Data + Arena->Used);
    Zero(Data, Size);
    Arena->Used += Size;
    
    return Data;
}

void *__ArenaRealloc(arena *Arena, void *OldMemory, s32 OldSize, s32 NewSize) {
    void *NewMemory = ArenaAllocate(Arena, NewSize);
    Copy(NewMemory, OldMemory, OldSize);
    return NewMemory;
}

void *_ArenaAllocateTracked(arena *Arena, s64 Size, s8 *File, s32 Line) {
    s8 Buffer[1024];
    s32 Length = stb_sprintf(Buffer, "%s, %d, %s, %d\n", Arena->Name, Size, File, Line);
    PlatformAppendFile("Memory.log", Buffer, Length);
    return _ArenaAllocate(Arena, Size);
}

template<typename T>
T *_ArenaAllocateStruct(arena *Arena, s64 Size) {
    T *Struct = (T *)ArenaAllocate(Arena, Size);
    *Struct = T();
    return Struct;
}