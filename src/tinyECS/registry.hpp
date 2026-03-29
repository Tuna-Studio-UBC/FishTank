#pragma once
#include <vector>

#include "components.hpp"
#include "states/state_machine.hpp"
#include "tiny_ecs.hpp"

class ECSRegistry {
    // callbacks to remove a particular or all entities in the system
    std::vector<ContainerInterface *> registry_list;

  public:
    // Entity
    ComponentContainer<Player> players;
    ComponentContainer<Camera> cameras;
    ComponentContainer<Boss> bosses;
    ComponentContainer<Enemy> enemies;
    ComponentContainer<Turret> turrets;
    ComponentContainer<Wall> walls;

    // Hazards
    ComponentContainer<Hazard> hazards;
    ComponentContainer<Bullet> bullets;
    ComponentContainer<Mine> mines;
    ComponentContainer<Anemone> anemones;
    ComponentContainer<Bell> bells;

    // Behaviour
    ComponentContainer<Health> healths;
    ComponentContainer<Dead> deads;
    ComponentContainer<Damage> damages;
    ComponentContainer<FireControl> fireControls;
    ComponentContainer<SpeedControl> speedControls;
    ComponentContainer<PlayerQuickTurn> playerQuickTurns;
    ComponentContainer<Expiration> expirations;
    ComponentContainer<OverTimeEffect> overTimeEffects;
    ComponentContainer<Target> targets;
    ComponentContainer<Wallet> wallets;
    ComponentContainer<DropsCrackers> dropsCrackers;
    ComponentContainer<Pursuit> pursuits;
    ComponentContainer<Bouncy> bouncies;
    ComponentContainer<Explosive> explosives;
    ComponentContainer<DeployedMine> deployedMines;
    ComponentContainer<WeaponState> weaponStates;
    ComponentContainer<BulletRequest> bullet_requests;
    ComponentContainer<Distracted> distracteds;
    ComponentContainer<Sense> senses;
    ComponentContainer<AI> ais;
    ComponentContainer<Bezier> beziers;
    ComponentContainer<Boid> boids;
    ComponentContainer<Upgrades> upgrades;
    ComponentContainer<Leader> leaders;

    // Pickups
    ComponentContainer<PickUp> pickUps;

    ComponentContainer<PowerUp> powerUps;
    ComponentContainer<Heal> heals;
    ComponentContainer<SpeedMultiplier> speedMultipliers;
    ComponentContainer<DamageMultiplier> damageMultipliers;
    ComponentContainer<Bubbler> bubblers;

    // Physics
    ComponentContainer<Motion> motions;
    ComponentContainer<Growth> growths;
    ComponentContainer<Collision> collisions;
    ComponentContainer<ParticleEmitter> particleEmitters;
    ComponentContainer<MeshCollider> meshColliders;

    // Game State
    ComponentContainer<ScreenState> screenStates;
    ComponentContainer<KeyboardInputState> keyboardInputs;
    ComponentContainer<MouseInputState> mouseInputs;
    ComponentContainer<Persistence> persistences;
    ComponentContainer<Progress> progresses;
    ComponentContainer<Scoring> scoring;
    ComponentContainer<TutorialTrigger> tutorialTriggers;
    ComponentContainer<StateMachine> stateMachines;
    ComponentContainer<StateTransitionRequest> stateTransitionRequests;

    // Sound
    ComponentContainer<SoundRequest> sound_requests;

    // Rendering
    ComponentContainer<Mesh *> meshPtrs;
    ComponentContainer<RenderRequest> renderRequests;
    ComponentContainer<Colour> colours;
    ComponentContainer<Tint> tints;
    ComponentContainer<Animation> animations;
    ComponentContainer<SpawnRequest> spawnRequests;

    // Map
    ComponentContainer<TileSet> tilesets;
    ComponentContainer<TileMap> tilemaps;

    // UI
    ComponentContainer<Text> texts;
    ComponentContainer<Overlay> overlays;

    // Debug
    ComponentContainer<Debug> debugComponents;

    // constructor that adds all containers for looping over them
    ECSRegistry() {
        // Entity
        registry_list.push_back(&players);
        registry_list.push_back(&bosses);
        registry_list.push_back(&enemies);
        registry_list.push_back(&turrets);
        registry_list.push_back(&walls);

        // Hazards
        registry_list.push_back(&hazards);
        registry_list.push_back(&bullets);
        registry_list.push_back(&mines);
        registry_list.push_back(&cameras);
        registry_list.push_back(&anemones);
        registry_list.push_back(&bells);

        // Behaviour
        registry_list.push_back(&healths);
        registry_list.push_back(&deads);
        registry_list.push_back(&damages);
        registry_list.push_back(&expirations);
        registry_list.push_back(&overTimeEffects);
        registry_list.push_back(&fireControls);
        registry_list.push_back(&speedControls);
        registry_list.push_back(&playerQuickTurns);
        registry_list.push_back(&targets);
        registry_list.push_back(&wallets);
        registry_list.push_back(&dropsCrackers);
        registry_list.push_back(&pursuits);
        registry_list.push_back(&bouncies);
        registry_list.push_back(&explosives);
        registry_list.push_back(&deployedMines);
        registry_list.push_back(&weaponStates);
        registry_list.push_back(&bullet_requests);
        registry_list.push_back(&distracteds);
        registry_list.push_back(&senses);
        registry_list.push_back(&ais);
        registry_list.push_back(&beziers);
        registry_list.push_back(&boids);
        registry_list.push_back(&upgrades);
        registry_list.push_back(&leaders);

        // Pickups
        registry_list.push_back(&pickUps);

        registry_list.push_back(&powerUps);
        registry_list.push_back(&heals);
        registry_list.push_back(&speedMultipliers);
        registry_list.push_back(&damageMultipliers);
        registry_list.push_back(&bubblers);

        // Physics
        registry_list.push_back(&motions);
        registry_list.push_back(&growths);
        registry_list.push_back(&collisions);
        registry_list.push_back(&particleEmitters);

        // Game State
        registry_list.push_back(&screenStates);
        registry_list.push_back(&keyboardInputs);
        registry_list.push_back(&mouseInputs);
        registry_list.push_back(&persistences);
        registry_list.push_back(&progresses);
        registry_list.push_back(&scoring);
        registry_list.push_back(&tutorialTriggers);
        registry_list.push_back(&stateMachines);
        registry_list.push_back(&stateTransitionRequests);

        // Sound
        registry_list.push_back(&sound_requests);

        // Rendering
        registry_list.push_back(&meshPtrs);
        registry_list.push_back(&renderRequests);
        registry_list.push_back(&colours);
        registry_list.push_back(&tints);
        registry_list.push_back(&animations);
        registry_list.push_back(&spawnRequests);

        // Map
        registry_list.push_back(&tilesets);
        registry_list.push_back(&tilemaps);

        // UI
        registry_list.push_back(&texts);
        registry_list.push_back(&overlays);

        // Debug
        registry_list.push_back(&debugComponents);
    }

    void clear_all_components() {
        for (ContainerInterface *reg : registry_list) {
            // A bit hacky, but a special case for persistence
            if (reg == &persistences) {
                continue;
            } else {
                reg->clear();
            }
        }
    }

    void list_all_components() {
        printf("Debug info on all registry entries:\n");
        int total_components = 0;
        for (ContainerInterface *reg : registry_list)
            if (reg->size() > 0) {
                printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
                total_components += (int)reg->size();
            }
        printf("======== TOTAL COMPONENTS: %4d ========\n", total_components);
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
    template <> ComponentContainer<Damage> &get<Damage>() { return damages; }
    template <> ComponentContainer<Heal> &get<Heal>() { return heals; }
    template <> ComponentContainer<SpeedMultiplier> &get<SpeedMultiplier>() { return speedMultipliers; }
    template <> ComponentContainer<DamageMultiplier> &get<DamageMultiplier>() { return damageMultipliers; }
};

extern ECSRegistry registry;
