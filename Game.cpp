#include "Includes.h"

struct world_map_shader_properties {
    f32 TileSize;
    s32 WidthInTiles;
    s32 HeightInTiles;
};

void WorldMapMaterialFunction(shader_instance *Shader) {
    world_map_shader_properties *Properties = (world_map_shader_properties *)Shader->UserData;
    
    ShaderSetInt(Shader, "WidthInTiles", Properties->WidthInTiles);
    ShaderSetInt(Shader, "HeightInTiles", Properties->HeightInTiles);
    ShaderSetFloat(Shader, "TileSize", Properties->TileSize);
}

struct game_state {
    arena FrameArena;
    
    texture *WorldMapTexture;
    u8 *WorldMapColours;
    
    texture *RegionMapTexture;
    u8 *RegionMapColours;
    
    u32 WorldMapShaderFile;
    shader_instance WorldMapShader;
    shader_instance RegionMapShader;
    
    world_data *World;
    vec2i CameraTilePosition;
};

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
    vec2i Position;
    u8 Colour;
    vec4 DebugColour;
};

struct region {
    vec2i Position;
    u8 Colour;
    vec4 DebugColour;
    
    biome Biome;
    s32 Width;
    s32 Height;
    tile *Tiles;
};

struct world_gen_data {
    s32 Width;
    s32 Height;
    
    s32 RegionWidth;
    s32 RegionHeight;
    
    f32 RegionRectSize;
    
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
    s32 RegionWidth;
    s32 RegionHeight;
    f32 RegionRectSize;
    region *Regions;
};

region *GetRegion(world_data *World, vec2i Position) {
    if(Position.x < 0 || Position.x >= World->Width) return 0;
    if(Position.y < 0 || Position.y >= World->Height) return 0;
    return World->Regions + (Position.x + Position.y*World->Width);
}

vec2 WorldTexCoord(world_gen_data *GenData, vec2i Position) {
    vec2 TotalSize = v2(GenData->Width, GenData->Height) * GenData->RegionRectSize;
    vec2 TexCoord = v2(Position.x, Position.y);
    TexCoord = TexCoord * v2(GenData->RegionRectSize);
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

void GenerateIceRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateTundraRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateForestRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateGrasslandRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateSaharaRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateDesertRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateWaterRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    for(s32 y = 0; y < Region->Height; y++) {
        for(s32 x = 0; x < Region->Width; x++) {
            tile *Tile = Region->Tiles + (x + y*Region->Width);
            Tile->Colour = Region->Colour;
        }
    }
}

void GenerateRegion(world_gen_data *GenData, world_data *World, region *Region, arena *Arena) {
    switch(Region->Biome) {
		case biome_ice: 
        GenerateIceRegion(GenData, World, Region, Arena);
        return;
        
		case biome_tundra: 
        GenerateTundraRegion(GenData, World, Region, Arena); 
        return;
        
		case biome_forest: 
        GenerateForestRegion(GenData, World, Region, Arena); 
        return;
        
		case biome_grassland: 
        GenerateGrasslandRegion(GenData, World, Region, Arena); 
        return;
        
		case biome_sahara: 
        GenerateSaharaRegion(GenData, World, Region, Arena); 
        return;
        
		case biome_desert: 
        GenerateDesertRegion(GenData, World, Region, Arena); 
        return;
        
		case biome_water: 
        GenerateWaterRegion(GenData, World, Region, Arena); 
        return;
	}
}

void GenerateWorld(world_gen_data *GenData, world_data *World, arena *Arena) {
    region *Regions = (region *)ArenaAllocate(Arena, sizeof(region) * GenData->Width * GenData->Height);
    
    for(s32 y = 0; y < GenData->Height; y++) {
        for(s32 x = 0; x < GenData->Width; x++) {
            region *Region = Regions + (x + y*GenData->Width);
            Region->Position = v2i(x, y);
            
            vec2 TexCoord = WorldTexCoord(GenData, Region->Position);
            
            f32 Value;
            biome Biome = WorldBiome(GenData, TexCoord, &Value);
            Region->Colour = PackedColourForBiome(Biome);
            Region->DebugColour = v4(v3(Value), 1.0f);
            Region->Biome = Biome;
            
            Region->Width = GenData->RegionWidth;
            Region->Height = GenData->RegionHeight;
            Region->Tiles = (tile *)ArenaAllocate(Arena, sizeof(tile)*GenData->RegionWidth*GenData->RegionHeight);
            GenerateRegion(GenData, World, Region, Arena);
        }
    }
    
    World->Regions = Regions;
    World->RegionRectSize = GenData->RegionRectSize;
    World->Width = GenData->Width;
    World->Height = GenData->Height;
    World->RegionWidth = GenData->RegionWidth;
    World->RegionHeight = GenData->RegionHeight;
}

void InitGame(platform *Platform) {
    InitMemory();
    InitRenderer();
    InitEntitySystem();
    
    Platform->UserData = ArenaAllocate(&GlobalArena, sizeof(game_state));
    game_state *GameState = (game_state *)Platform->UserData;
    
    MakeSubArena(&GameState->FrameArena, &GlobalArena, "FrameArena", MB(512));
    
    world_gen_data GenData;
    GenData.Width = 256;
    GenData.Height = 256;
    GenData.RegionWidth = 16;
    GenData.RegionHeight = 16;
    GenData.RegionRectSize = 8.0f;
    GenData.NoiseScale = 10.0f;
    GenData.Lacunarity = 2.0f;
    GenData.Gain = 0.5f;
    GenData.Octaves = 6;
    
    static world_data World;
    GenerateWorld(&GenData, &World, &GlobalArena);
    GameState->World = &World;
    
    s32 WorldMapSize = 64*64;
    s32 RegionMapSize = World.RegionWidth * World.RegionHeight;
    GameState->WorldMapColours = (u8 *)ArenaAllocate(&GlobalArena, WorldMapSize);
    GameState->RegionMapColours = (u8 *)ArenaAllocate(&GlobalArena, RegionMapSize);
    GameState->WorldMapTexture = CreateBufferTexture("WorldMap", GameState->WorldMapColours, WorldMapSize);
    GameState->RegionMapTexture = CreateBufferTexture("RegionMap", GameState->RegionMapColours, RegionMapSize);
    
    GameState->WorldMapShaderFile = LoadShader("Shaders/RegionMap.vs", "Shaders/RegionMap.fs");
    
    static world_map_shader_properties WorldMapShaderProperties;
    WorldMapShaderProperties.WidthInTiles = 64;
    WorldMapShaderProperties.HeightInTiles = 64;
    WorldMapShaderProperties.TileSize = 8.0f;
    CreateShaderInstance(&GameState->WorldMapShader, WorldMapMaterialFunction, &WorldMapShaderProperties, GameState->WorldMapShaderFile);
    
    static world_map_shader_properties RegionMapShaderProperties;
    RegionMapShaderProperties.WidthInTiles = GenData.RegionWidth;
    RegionMapShaderProperties.HeightInTiles = GenData.RegionHeight;
    RegionMapShaderProperties.TileSize = 8.0f;
    CreateShaderInstance(&GameState->RegionMapShader, WorldMapMaterialFunction, &RegionMapShaderProperties, GameState->WorldMapShaderFile);
}

void TickGame(platform *Platform) {
    static bool ShowDebug = false;
    if(Platform->KeyPressed[key_q]) ShowDebug = !ShowDebug;
    
    TracyBegin(PushQuads);
    
    game_state *GameState = (game_state *)Platform->UserData;
    world_data *World = GameState->World;
    
    s32 RenderSize = 64;
    
    for(s32 y = 0; y < RenderSize; y++) {
        for(s32 x = 0; x < RenderSize; x++) {
            region *Region = GetRegion(World, GameState->CameraTilePosition + v2i(x, y));
            if(!Region) continue;
            GameState->WorldMapColours[x + y*RenderSize] = Region->Colour;
        }
    }
    
    UpdateBufferTexture(GameState->WorldMapTexture, GameState->WorldMapColours, RenderSize*RenderSize);
    renderer_quad Quad;
    Quad.Position = v2(100.0f);
    Quad.Size = v2(World->RegionRectSize * RenderSize);
    Quad.Shader = &GameState->WorldMapShader;
    PushTexture(&Quad, GameState->WorldMapTexture);
    
    region *CurrentRegion = GetRegion(World, GameState->CameraTilePosition);
    if(CurrentRegion) {
        for(s32 y = 0; y < World->RegionHeight; y++) {
            for(s32 x = 0; x < World->RegionWidth; x++) {
                tile *Tile = CurrentRegion->Tiles + (x + y*World->RegionWidth);
                GameState->RegionMapColours[x + y*World->RegionWidth] = Tile->Colour;
            }
        }
    }
    
    s32 RegionMapSize = World->RegionWidth * World->RegionHeight;
    UpdateBufferTexture(GameState->RegionMapTexture, GameState->RegionMapColours, RegionMapSize);
    Quad.Position = v2(800.0f, 100.0f);
    Quad.Size = v2(World->RegionRectSize)*v2(World->RegionWidth,World->RegionHeight);
    Quad.Shader = &GameState->RegionMapShader;
    PushTexture(&Quad, GameState->RegionMapTexture);
    
    TracyEnd(PushQuads);
    
    if(Platform->KeyPressed[key_w]) GameState->CameraTilePosition.y -= 1;
    if(Platform->KeyPressed[key_a]) GameState->CameraTilePosition.x -= 1;
    if(Platform->KeyPressed[key_s]) GameState->CameraTilePosition.y += 1;
    if(Platform->KeyPressed[key_d]) GameState->CameraTilePosition.x += 1;
    if(GameState->CameraTilePosition.x < 0) 
        GameState->CameraTilePosition.x = World->Width - 1;
    if(GameState->CameraTilePosition.x >= World->Width) 
        GameState->CameraTilePosition.x = 0;
    if(GameState->CameraTilePosition.y < 0) 
        GameState->CameraTilePosition.y = World->Height - 1;
    if(GameState->CameraTilePosition.y >= World->Height) 
        GameState->CameraTilePosition.y = 0;
    
    UpdateAllEntities(Platform, World);
    
    RendererBeginFrame();
    RenderAllEntities(World);
    RendererEndFrame();
    
    GameState->FrameArena.Used = 0;
}