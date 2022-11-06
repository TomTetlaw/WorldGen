#include "Includes.h"

struct entity_list {
    arena Arena;
    entity *LastEntity;
    s32 Count;
    s32 NextParity;
};

entity_list List;

void InitEntitySystem() {
    Zero(&List, sizeof(entity_list));
    
    MakeSubArena(&List.Arena, &GlobalArena, "EntityArena", MB(128));
}

entity *CreateEntity() {
    entity *Entity = List.LastEntity;
    while(Entity) {
        if(Entity->Free) {
            Entity->Free = false;
            break;
        }
        
        Entity = Entity->Prev;
    }
    
    if(!Entity) {
        Entity = (entity *)ArenaAllocate(&List.Arena, sizeof(entity));
        Entity->Prev = List.LastEntity;
        List.LastEntity = Entity;
    }
    
    Entity->Active = true;
    Entity->Texture = 0;
    
    Entity->Parity = List.NextParity++;
    List.Count++;
    
    return Entity;
}

void RemoveEntity(entity *Entity) {
    Entity->Free = true;
    List.Count--;
}

void RemoveAllEntities() {
    entity *Entity = List.LastEntity;
    while(Entity) {
        Entity->Free = true;
        Entity = Entity->Prev;
    }
}

void UpdateAllEntities(platform *Platform, world_data *World) {
    entity *Entity = List.LastEntity;
    while(Entity) {
        if(Entity->Free) {
            Entity = Entity->Prev;
            continue;
        }
        if(!Entity->Active) {
            Entity = Entity->Prev;
            continue;
        }
        
        Entity = Entity->Prev;
    }
}

void RenderAllEntities(world_data *World) {
    entity *Entity = List.LastEntity;
    while(Entity) {
        if(Entity->Free) {
            Entity = Entity->Prev;
            continue;
        }
        if(!Entity->Active) {
            Entity = Entity->Prev;
            continue;
        }
        
        renderer_quad Quad;
        Quad.Position = v2(Entity->TilePosition * v2i(World->RegionRectSize));
        Quad.Size = v2(World->RegionRectSize);
        Quad.Colour = Entity->Colour;
        Quad.Shader = 0;
        
        if(Entity->Texture) PushTexture(&Quad, Entity->Texture);
        else PushQuad(&Quad);
        Entity = Entity->Prev;
    }
}