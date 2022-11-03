#ifndef _PLATFORM_H
#define _PLATFORM_H

enum key_code {
    key_none = -1,
    key_0 = 0, key_1, key_2, key_3, key_4, 
    key_5, key_6,key_7, key_8, key_9,
    key_a, key_b, key_c, key_d, key_e, key_f,
    key_g, key_h, key_i, key_j, key_k, key_l,
    key_m, key_n, key_o, key_p, key_q, key_r,
    key_s, key_t, key_u, key_v, key_w, key_x,
    key_y, key_z
};

struct platform {
    vec2 MousePos;
    u8 KeyStates[256];
    u8 KeyPressed[256];
    bool ShiftDown, CtrlDown, AltDown;
    
    f32 RealTime;
    f32 GameTime;
    f32 DeltaTime;
    
    void *UserData;
};

void *PlatformAllocate(s64 Size);

struct file {
    s8 *Data;
    s64 Length;
};

file PlatformLoadFile(s8 *Path, bool ZeroTerminate, arena *Arena);
void PlatformDeleteFile(s8 *Path);
void PlatformWriteFile(s8 *Path, void *Data, s64 Size);
void PlatformAppendFile(s8 *Path, void *Data, s64 Size);

void PlatformFatalError(s8 *Text, ...);

f32 PlatformTime();

#endif //_PLATFORM_H
