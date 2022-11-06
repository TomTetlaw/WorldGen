#ifndef MEMORY_H
#define MEMORY_H

#define KB(x) ((x##ll)*1024ll)
#define MB(x) ((x##ll)*1024ll*1024ll)
#define GB(x) ((x##ll)*1024ll*1024ll*1024ll)

struct arena {
    s8 *Data;
    s64 Capacity;
    s64 Used;
    s64 Mark;
    s8 *Name;
};

extern arena GlobalArena;
extern arena StringArena;
extern arena RendererArena;
extern arena TextureArena;

void InitMemory();
void MakeArena(arena *Arena, s8 *Name, s64 Capacity);
void MakeSubArena(arena *Arena, arena *Parent, s8 *Name, s64 Capacity);
void PushArenaMark(arena *Arena);
void PopArenaMark(arena *Arena);

// this is just for stb_image
void *__ArenaRealloc(arena *Arena, void *OldMemory, s32 OldSize, s32 NewSize);

void GlobalPushMark();
void GlobalPopMark();

#ifdef PROFILE_MEMORY
#define ArenaAllocate(Arena, Size) _ArenaAllocateTracked(Arena, Size, __FILE__, __LINE__)
#else
#define ArenaAllocate(Arena, Size) _ArenaAllocate(Arena, Size)
#endif

void *ArenaAllocateNoZero(arena *Arena, s64 Size);
void *_ArenaAllocate(arena *Arena, s64 Size);
void *_ArenaAllocateTracked(arena *Arena, s64 Size, s8 *File, s32 Line);

#define ArenaAllocateStruct(Arena, T) _ArenaAllocateStruct<T>(Arena, sizeof(T))
template<typename T>
T *_ArenaAllocateStruct(arena *Arena, s64 Size);

#endif