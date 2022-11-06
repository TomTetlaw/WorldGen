#include "Includes.h"

texture *LastTexture = 0;

texture *FindTexture(s8 *Path) {
    texture *Texture = LastTexture;
    while(Texture) {
        if(StringCompare(Path, Texture->Path)) return Texture;
        Texture = Texture->Prev;
    }
    
    return 0;
}

texture *LoadTexture(s8 *Path) {
    texture *Texture = FindTexture(Path);
    if(Texture) return Texture;
    
    GlobalPushMark();
    
    file File = PlatformLoadFile(Path, 0, &GlobalArena);
    
    s32 Width, Height;
    u8 *Data = stbi_load_from_memory((u8*)File.Data, File.Length, &Width, &Height, 0, 4);
    
    if(!File.Data) PlatformFatalError("Failed to load texture %s: %s", Path, stbi_failure_reason());
    
    u32 ID;
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    stbi_image_free(Data);
    
    GlobalPopMark();
    
    Texture = (texture *)ArenaAllocate(&TextureArena, sizeof(texture));
    Texture->Width = Width;
    Texture->Height = Height;
    Texture->ID = ID;
    Texture->Path = Path;
    Texture->Target = GL_TEXTURE_2D;
    
    Texture->Prev = LastTexture;
    LastTexture = Texture;
    
    return Texture;
}

texture *CreateBufferTexture(s8 *Name, void *Data, s32 Size) {
    u32 BufferID;
    glGenBuffers(1, &BufferID);
    glBindBuffer(GL_TEXTURE_BUFFER, BufferID);
    glBufferData(GL_TEXTURE_BUFFER, Size, Data, GL_DYNAMIC_DRAW);
    
    u32 TextureID;
    glGenTextures(1, &TextureID);
    
    texture *Texture = (texture *)ArenaAllocate(&TextureArena, sizeof(texture));
    Texture->ID = TextureID;
    Texture->BufferID = BufferID;
    Texture->Path = Name;
    Texture->Target = GL_TEXTURE_BUFFER;
    
    Texture->Prev = LastTexture;
    LastTexture = Texture;
    
    UpdateBufferTexture(Texture, Data, Size);
    
    return Texture;
}

void UpdateBufferTexture(texture *Texture, void *Data, s32 Size) {
    glBindBuffer(GL_TEXTURE_BUFFER, Texture->BufferID);
    glBufferSubData(GL_TEXTURE_BUFFER, 0, Size, Data);
    glBindTexture(GL_TEXTURE_BUFFER, Texture->ID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, Texture->BufferID);
}