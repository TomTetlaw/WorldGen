#ifndef _TEXTURE_H
#define _TEXTURE_H

struct texture {
    s8 *Path;
    s32 Width;
    s32 Height;
    u32 ID;
    u32 BufferID; // for buffer textures
    u32 Target;
    texture *Prev;
};

texture *LoadTexture(s8 *Path);
texture *CreateBufferTexture(s8 *Name, void *Data, s32 Size);
void UpdateBufferTexture(texture *Texture, void *Data, s32 Size);

#endif //_TEXTURE_H
