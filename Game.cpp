#include "Includes.h"

arena FrameArena;

enum biome {
    biome_ice,
    biome_tundra,
    biome_forest,
    biome_grassland,
    biome_sahara,
    biome_desert,
    biome_water,
};

vec4 UnpackColour(u8 Colour) {
    switch(Colour) {
		case 1: return rgba(255, 255, 255, 255);
        case 2: return rgba(9, 150, 181, 255);
        case 3: return rgba(24, 110, 0, 255);
		case 4: return rgba(159, 250, 2, 255);
		case 5: return rgba(225, 250, 2, 255);
		case 6: return rgba(250, 167, 2, 255);
		case 7: return rgba(0, 0, 255, 255);
    }
	
	return v4(0.0f);
}

u8 PackedColourForBiome(biome Biome) {
	switch(Biome) {
		case biome_ice: return 1;
		case biome_tundra: return 2;
		case biome_forest: return 3;
		case biome_grassland: return 4;
		case biome_sahara: return 5;
		case biome_desert: return 6;
		case biome_water: return 7;
	}
	
	return 0;
}

struct tile {
    vec2i TilePosition;
    u8 Colour;
    bool Blocking;
    vec4 DebugColour;
};

#define building_ladder (building_ladder_left | building_ladder_right)
#define building_ladder_left (1 << 0)
#define building_ladder_right (1 << 1)

u8 PackTile(tile *Tile) {
    u8 Out = ((Tile->Colour << 1) & 0x1e) | ((u8)Tile->Blocking ? (u8)1 : (u8)0) & 0x1;
    return Out;
}

void UnpackTile(u8 Packed, tile *Tile) {
    Tile->Colour = (Packed & 0x1e) >> 1;
    Tile->Blocking = (Packed & 0x1);
}

struct world_gen_data {
    s32 Width;
    s32 Height;
    f32 TileSize;
    
    // NOTE(TomTetlaw): Noise paramaters.
    u8 Seed;
    f32 NoiseScale;
    f32 Lacunarity;
    f32 Gain;
    f32 Octaves;
};

struct world_data {
    s32 Width;
    s32 Height;
    f32 TileSize;
    tile *Tiles;
};

tile *GetTile(world_data *World, vec2i TilePosition) {
    if(TilePosition.x < 0 || TilePosition.x >= World->Width) return 0;
    if(TilePosition.y < 0 || TilePosition.y >= World->Height) return 0;
    return World->Tiles + (TilePosition.x + TilePosition.y*World->Width);
}

vec2 WorldTexCoord(world_gen_data *GenData, vec2i TilePosition) {
    vec2 TotalSize = v2(GenData->Width, GenData->Height) * GenData->TileSize;
    vec2 TexCoord = v2(TilePosition.x, TilePosition.y);
    TexCoord = TexCoord * v2(GenData->TileSize);
    TexCoord = TexCoord * v2(1.0f/TotalSize.x, 1.0f/TotalSize.y);
    return TexCoord;
}

f32 WorldRadialGradient(world_gen_data *GenData, vec2 TexCoord) {
    vec2 Dist = TexCoord - v2(0.5f);
    return 1.0f - Length(Dist);
}

f32 WorldTemperatureGradient(world_gen_data *GenData, vec2 TexCoord) {
    vec2 Dist = TexCoord - v2(0.5f);
    return 1.0f - Abs(Dist.y);
}

f32 WorldNoise(world_gen_data *GenData, vec2 TexCoord) {
    TexCoord *= GenData->NoiseScale;
    f32 Noise = stb_perlin_noise3(TexCoord.x, TexCoord.y, 0.0f, 0, 0, 0);
    return Noise*0.5f + 0.5f;
}

biome WorldBiome(world_gen_data *GenData, vec2 TexCoord, f32 *OutValue) {
    f32 RadialGradient = WorldRadialGradient(GenData, TexCoord);
    f32 TempGradient = WorldTemperatureGradient(GenData, TexCoord) + 0.1f;
    f32 WorldMask = TempGradient*RadialGradient;
    f32 Noise = WorldNoise(GenData, TexCoord);
    f32 Value = Noise * WorldMask;
    if(OutValue) *OutValue = Value;
    
    f32 e = 0.1f;
    if(Value - 0.0f <= e) return biome_water;
    if(Value - 0.1f <= e) return biome_desert;
    if(Value - 0.2f <= e) return biome_sahara;
    if(Value - 0.3f <= e) return biome_grassland;
    if(Value - 0.5f <= e) return biome_forest;
    if(Value - 0.6f <= e) return biome_tundra;
    return biome_ice;
}

void GenerateWorld(world_gen_data *GenData, world_data *World, arena *Arena) {
    tile *Tiles = (tile *)ArenaAllocate(Arena, sizeof(tile) * GenData->Width * GenData->Height);
    
    for(s32 y = 0; y < GenData->Height; y++) {
        for(s32 x = 0; x < GenData->Width; x++) {
            tile *Tile = Tiles + (x + y*GenData->Width);
            Tile->TilePosition = v2i(x, y);
            
            vec2 TexCoord = WorldTexCoord(GenData, Tile->TilePosition);
            
            f32 Value;
            biome Biome = WorldBiome(GenData, TexCoord, &Value);
            Tile->Colour = PackedColourForBiome(Biome);
            Tile->DebugColour = v4(v3(Value), 1.0f);
        }
    }
    
    World->Tiles = Tiles;
    World->TileSize = GenData->TileSize;
    World->Width = GenData->Width;
    World->Height = GenData->Height;
}

struct world_file_header {
    s32 Width;
    s32 Height;
    f32 TileSize;
};

void WriteWorld(world_data *World, s8 *Path, arena *Arena) {
    PushArenaMark(Arena);
    
    s32 Size = sizeof(world_file_header) + World->Width*World->Height;
    u8 *Bytes = (u8 *)ArenaAllocate(Arena, Size);
    
    world_file_header *Header = (world_file_header *)Bytes;
    Header->Width = World->Width;
    Header->Height = World->Height;
    Header->TileSize = World->TileSize;
    
    for(s32 y = 0; y < World->Height; y++) {
        for(s32 x = 0; x < World->Width; x++) {
            u8 *Byte = Bytes + (x + y*World->Width) + sizeof(world_file_header);
            *Byte = PackTile(GetTile(World, v2i(x, y)));
        }
    }
    
    PlatformWriteFile(Path, Bytes, Size);
    
    PopArenaMark(Arena);
}

void ReadWorld(world_data *World, s8 *Path, arena *Arena) {
    file File = PlatformLoadFile(Path, false, Arena);
    if(!File.Data) return;
    
    u8 *Bytes = (u8 *)File.Data;
    world_file_header *Header = (world_file_header *)Bytes;
    
    tile *Tiles = (tile *)ArenaAllocate(Arena, sizeof(tile) * Header->Width * Header->Height);
    
    for(s32 y = 0; y < Header->Height; y++) {
        for(s32 x = 0; x < Header->Width; x++) {
            u8 *Byte = Bytes + (x + y*Header->Width) + sizeof(world_file_header);
            tile *Tile = Tiles + (x + y*Header->Width);
            UnpackTile(*Byte, Tile);
            
            Tile->TilePosition = v2i(x, y);
        }
    }
    
    World->TileSize = Header->TileSize;
    World->Width = Header->Width;
    World->Height = Header->Height;
    World->Tiles = Tiles;
}

void InitGame(platform *Platform) {
    InitMemory();
    InitRenderer();
    InitEntitySystem();
    
    MakeSubArena(&FrameArena, &GlobalArena, "FrameArena", MB(128));
    
    world_gen_data GenData;
    GenData.Width = 256;
    GenData.Height = 256;
    GenData.TileSize = 8.0f;
    GenData.NoiseScale = 10.0f;
    GenData.Lacunarity = 2.0f;
    GenData.Gain = 0.5f;
    GenData.Octaves = 6;
    
    static world_data World;
    GenerateWorld(&GenData, &World, &GlobalArena);
    
    WriteWorld(&World, "Worlds/Test.world", &GlobalArena);
    
    Platform->UserData = ArenaAllocate(&GlobalArena, sizeof(world_data));
    ReadWorld((world_data *)Platform->UserData, "Worlds/Test.world", &GlobalArena);
    
    Platform->UserData = (void *)&World;
}

void TickGame(platform *Platform) {
    static bool ShowDebug = false;
    if(Platform->KeyPressed[key_q]) ShowDebug = !ShowDebug;
    
    TracyBegin(PushQuads);
    
    world_data *World = (world_data *)Platform->UserData;
    renderer_quad *Quads = (renderer_quad *)ArenaAllocate(&FrameArena, sizeof(renderer_quad) * World->Width * World->Height);
    
    for(s32 y = 0; y < World->Height; y++) {
        for(s32 x = 0; x < World->Width; x++) {
            tile *Tile = GetTile(World, v2i(x, y));
            
            renderer_quad *Quad = Quads + (x + y*World->Width);
            Quad->Position = v2(100.0f) + v2(x, y)*World->TileSize*0.25f;
            Quad->Size = v2(World->TileSize)*0.25f;
            
            if(ShowDebug) Quad->Colour = Tile->DebugColour;
            else Quad->Colour = UnpackColour(Tile->Colour);
        }
    }
    
    PushQuads(Quads, World->Width * World->Height);
    
    TracyEnd(PushQuads);
    
    UpdateAllEntities(Platform, World);
    
    RendererBeginFrame();
    RenderAllEntities(World);
    RendererEndFrame();
    
    FrameArena.Used = 0;
}