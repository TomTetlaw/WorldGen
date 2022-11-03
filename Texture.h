#ifndef _TEXTURE_H
#define _TEXTURE_H

struct texture {
    s8 *Path;
    s32 Width;
    s32 Height;
    u32 ID;
    texture *Prev;
};

texture *LoadTexture(s8 *Path);

#endif //_TEXTURE_H
