#pragma once
#include <vector>

#include "components.hpp"
#include "tiny_ecs.hpp"

class ECSRegistry {
    // callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface *> registry_list;

  public:
    // Entity
    ComponentContainer<Player> players;
    ComponentContainer<Enemy> enemies;
    ComponentContainer<Turret> turrets;
    ComponentContainer<Wall> wall;

    // Hazards
    ComponentContainer<Hazard> hazard;
    ComponentContainer<Bullet> bullet;
    ComponentContainer<Mine> mine;
    ComponentContainer<Camera> camera;
    ComponentContainer<Anemone> anemone;
    ComponentContainer<Bell> bells;

    // Behaviour
    ComponentContainer<Health> health;
    ComponentContainer<Dead> dead;
    ComponentContainer<Damage> damage;
    ComponentContainer<Expiration> expiration;
    ComponentContainer<OverTimeEffect> overTimeEffect;
    ComponentContainer<FireControl> fireControl;
    ComponentContainer<SpeedControl> speedControl;
    ComponentContainer<PlayerQuickTurn> playerQuickTurn;
    ComponentContainer<Target> target;
    ComponentContainer<Wallet> wallet;
    ComponentContainer<DropsCrackers> dropsCrackers;
    ComponentContainer<Patrolling> patrolling;
    ComponentContainer<Pursuit> pursuit;
    ComponentContainer<Bouncy> bouncy;
    ComponentContainer<BulletRequest> bullet_requests;
    ComponentContainer<Distracted> distracted;

    // Pickups
    ComponentContainer<PickUp> pickUp;

    ComponentContainer<PowerUp> powerUp;
    ComponentContainer<Heal> heal;
    ComponentContainer<SpeedMultiplier> speedMultiplier;
    ComponentContainer<DamageMultiplier> damageMultiplier;
    ComponentContainer<Bubbler> bubblers;

    // Physics
    ComponentContainer<Motion> motions;
    ComponentContainer<Collision> collisions;
    ComponentContainer<ParticleEmitter> particleEmitters;
    ComponentContainer<MeshCollider> meshColliders;
    // Game State
    ComponentContainer<ScreenState> screenStates;
    ComponentContainer<KeyboardInputState> keyboardInput;
    ComponentContainer<MouseInputState> mouseInput;
    ComponentContainer<Progress> progress;

    // Rendering
    ComponentContainer<Mesh *> meshPtrs;
    ComponentContainer<RenderRequest> renderRequests;
    ComponentContainer<vec3> colors;
    ComponentContainer<Tint> tints;
    ComponentContainer<Animation> animations;

    // Map
    ComponentContainer<TileSet> tilesets;
    ComponentContainer<TileMap> tilemaps;

    // UI
    ComponentContainer<Text> text;
    ComponentContainer<Overlay> overlays;

    // Debug
    ComponentContainer<Debug> debugComponents;

    // constructor that adds all containers for looping over them
    ECSRegistry() {
        // Entity
        registry_list.push_back(&players);
        registry_list.push_back(&enemies);
        registry_list.push_back(&turrets);
        registry_list.push_back(&wall);

        // Hazards
        registry_list.push_back(&hazard);
        registry_list.push_back(&bullet);
        registry_list.push_back(&mine);
        registry_list.push_back(&camera);
        registry_list.push_back(&anemone);
        registry_list.push_back(&bells);

        // Behaviour
        registry_list.push_back(&health);
        registry_list.push_back(&dead);
        registry_list.push_back(&damage);
        registry_list.push_back(&expiration);
        registry_list.push_back(&overTimeEffect);
        registry_list.push_back(&fireControl);
        registry_list.push_back(&speedControl);
        registry_list.push_back(&playerQuickTurn);
        registry_list.push_back(&target);
        registry_list.push_back(&wallet);
        registry_list.push_back(&dropsCrackers);
        registry_list.push_back(&patrolling);
        registry_list.push_back(&pursuit);
        registry_list.push_back(&bouncy);
        registry_list.push_back(&bullet_requests);
        registry_list.push_back(&distracted);

        // Pickups
        registry_list.push_back(&pickUp);

        registry_list.push_back(&powerUp);
        registry_list.push_back(&heal);
        registry_list.push_back(&speedMultiplier);
        registry_list.push_back(&damageMultiplier);
        registry_list.push_back(&bubblers);

        // Physics
        registry_list.push_back(&motions);
        registry_list.push_back(&collisions);
        registry_list.push_back(&particleEmitters);

        // Game State
        registry_list.push_back(&screenStates);
        registry_list.push_back(&keyboardInput);
        registry_list.push_back(&mouseInput);
        registry_list.push_back(&progress);

        // Rendering
        registry_list.push_back(&meshPtrs);
        registry_list.push_back(&renderRequests);
        registry_list.push_back(&colors);
        registry_list.push_back(&tints);
        registry_list.push_back(&animations);

        // Map
        registry_list.push_back(&tilesets);
        registry_list.push_back(&tilemaps);

        // UI
        registry_list.push_back(&text);
        registry_list.push_back(&overlays);

        // Debug
        registry_list.push_back(&debugComponents);
    }

    void clear_all_components() {
        for (ContainerInterface *reg : registry_list)
            reg->clear();
    }

    void list_all_components() {
        printf("Debug info on all registry entries:\n");
        for (ContainerInterface *reg : registry_list)
            if (reg->size() > 0)
                printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
    }

    void list_all_components_of(Entity e) {
        printf("Debug info on components of entity %u:\n", (unsigned int)e);
        for (ContainerInterface *reg : registry_list)
            if (reg->has(e))
                printf("type %s\n", typeid(*reg).name());
    }

    void remove_all_components_of(Entity e) {
        for (ContainerInterface *reg : registry_list)
            reg->remove(e);
    }

    template <typename Component> ComponentContainer<Component> &get();
    template <> ComponentContainer<Damage> &get<Damage>() { return damage; }
    template <> ComponentContainer<Heal> &get<Heal>() { return heal; }
    template <> ComponentContainer<SpeedMultiplier> &get<SpeedMultiplier>() { return speedMultiplier; }
    template <> ComponentContainer<DamageMultiplier> &get<DamageMultiplier>() { return damageMultiplier; }
};

extern ECSRegistry registry;
