# FishTank! Test Plan

**Last Updated:** Mar 01, 2026

## Player Experience

Items relating directly to the player's view. (Entity interactions are in a separate section)

### Movement

- Player can use `W` key to add forward acceleration. Player speed is capped and the player will automatically decelerate when not holding `W` down to 0 velocity
- Player can use `D` key to turn left
- Player can use `A` key to turn right
- Player can use `S` will toggle quick turn. Players will not be able to `A` or `D` during the quick turn to prevent multiple conflicting actions.

### Shooting

- Player can use `Left-Click` on their mouse to shoot a projectile. The projectile should travel in the direction of the cursor on screen. Player can shoot independently of their movement with the keyboard.
- **M3:** Player can use `Right-Click` on their mouse to shoot the special projectile.
- **M3:** Player can use the number `1` and `2` to swap between the basic and explosive weapons. Pressing `1` when you are already on the basic weapon does nothing and vice versa.

### Menu
- **M2:** Player can press `T` at any time to restart tutorial.
- **M2:** Player can press `Enter` to clear tutorial messages.
- **M2:** Player can press `R` once they've won or lost to reset the game to after the tutorial.
- **M3:** Player can press `ESC` to pause their game

## Rendering

### General

- Load game: All entities, including the player and walls are visible and rendered
- **M2:** Move player: The camera is centered on the player as the player moves, offset slightly by cursor position to help the player aim. 
- **M2:** All rendered entities should be rendered accordingly, except for elements of the UI which are fixed to the screen.
- **M2:** All players and enemies have unique sprites.

### Animation

- **M2:** Player, Enemy, Hazard, Power-Up, and bubble sprites all have animation

## Entity Behaviour

Behaviours of various entity interactions, including collision, detection, and expiration.

### Walls

- Move the player to the outer walls: Player is limited to move within the map boundaries defined by the outer walls
- Move the player to inner walls, lure enemies to inner walls, fire at inner walls: Player, Enemies, Bullets cannot pass through the walls within the map

### Hazards & Power-ups

- Drive over hazards: Hazards will decrease player health
- **M2:** Drive over power-up: Drive over a medkit in order to increase player health.

### Projectiles

- Hit wall: Projectiles will bounce off walls
- **M2:** Expiry: Projectiles should de-spawn after ten seconds for the player, or five seconds for the enemy, or after hitting a target
- **M2:** Hitting Self: Projectiles should not be able to hit the creator (E.g. Player bullets cannot hit player, will pass through)
- Player can only fire projectiles if they have ammunition (shown in the bottom UI)
- **M3:** Friendly mines do not damage player and explode on enemy contact, damaging in a small area.
- **M3:** Explosive shots explode on contact with another object instead of bouncing. When colliding with a mine, it results in a large explosion.

### Enemies

- Player gets within detection distance: Enemy will begin shooting and chasing the player. Once it gets within a configurable range, it will stop approaching the player but will continue shooting.
- **M2:** Exit detection distance: Crab enemies will only detect the player when they are nearby, and will lose the player if they exit their detection distance. Fish enemies do not have a maximum detection distance, and always track and follow the player.
- **M2:** Pathfinding: Enemies should pathfind correctly around walls towards the player.
- **M3:** When the "Mother" enemy is killed, any remaining children should switch from following the mother (Which is gone) to swarming the player. When the children of a mother are killed, the mother should switch to a basic crab attack pattern.

### Mesh-Based Collision
- **M2:** Player has mesh-based collider that follows the rough contour of the fish.

## Bug List

### [P1: Completing tutorial leads to win screen #123](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/123)

Level transitions broke during refactoring, completing the tutorial leads directly into the win screen. Fixed, can be tested by completing the tutorial.

Steps to reproduce:
1. Load the game up for the first time, you are in the tutorial
2. Defeat the 3 enemy crabs and enter the upgrade screen
3. After exiting via `ESC`, you will be greeted by the win screen

### [P2: Health bars render on top of game over message #117](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/117)

Health bar render layer is on top of game over message.

Steps to reproduce:
1. Position an enemy in the middle of your screen
2. Lose to that enemy
3. See that the health bar is on top

This has to do with how we've structured our rendering pipeline, not fixed as of M3 since this requires some restructuring in the rendering flow.

### [P2: Lag Spike for Mesh-Based Collisions #96](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/96)

Significant performance hit for mesh-based collisions. Will have to optimize the detections in the collisions/physics systems, and perform some pruning for far enough entities.

Steps to Replicate:

1. Toggle the FPS counter on
2. Move the player against any collider (best to just rub against a wall)
3. Observe drop in framerate

~~Unresolved as of M2~~

**M3:** Resolved via refactoring the collision and physics systems. Observe that the frames remain relatively stable (Some minor variation allowed) when colliding with walls.

### [P1: Game Crash on Reset #68](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/62)

I experienced a crash caused by an out of bounds vector lookup on [this line](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/blob/fceee345e95b222a5be10b11f4ecce6adc97295c/src/systems/player_input_system.cpp#L102) when resetting the game after winning. I think it's caused by the player being deleted on reset, so they don't exist here, and it crashes.

Steps to reproduce:

1. Play the game till you win (or lose)
2. Press "R" to reset (spam the key right before game over)
3. Crash occurs



### [P3: Bullets self-collide when fired](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/53)

When moving with a speed boost, the player's bullets sometimes hit themselves, and despawn.

Steps to replicate:

1. Pick up a speed boost item
2. Go in a straight line
3. Fire continuously in the direction you're going


### [P5: Urchin enemy projectiles not appearing #52](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/52)

The urchin attack created projectiles as expected, but the projectiles never made it to render. This caused the Urchin to appear as if stationary and non-hostile. This was caused by the stacked projectiles being mistaken for a collision and removed.

To test, move the player closer to the Urchin enemy, which should trigger the attack pattern. There should be 8 projectiles fired from the enemy in all directions.

### [P4: New ID increment issue (Too many IDs used) #47](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/47)

Once again, entity IDs were incrementing far too quickly. The cause this time were the wall entities being initialized at every pass through the physics system.

To test, fire a bullet and ensure each bullet only uses 1 new ID. The easiest way is to fire a volley of bullets and wait for them to expire. If the first bullet expiry is `Entity x has expired`, the next bullet should print `Entity x+1 has expired`.

### [P2: Projectile Bounce Bug #45](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/45)

If a projectile hits multiple walls simultaneously, sometimes it will reverse direction, rather than bounce.

Steps to Replicate: Aim and shoot at a wall, right between two tiles in the tilemap. The aiming is a bit finnicky, so it may take a few tries.

**Unresolved as of M2**

### [P3: Linear Interpolated Camera causes tiles to cull too early #44](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/44)

When moving horizontally quickly (with LERP camera), you can see tiles on the right side of the screen culling themselves, even when visible.

To test, move player character quickly horizontally from right to left. All tiles on screen should remain visible.

### [P3: TileMap Texture Bleeding #39](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/39)

Texture bleeding occurs sometimes when moving quickly along the tile map. Will occur when moving around as the player, seemingly somewhat random.

To test, move around as the player and set the player max speed to be high (300 px/s is more than sufficient). Moving around the map on each side of the wall, there should be no bleeding that occurs on any of the walls.

### [P2: Tileset tiles are renderering incorrect IDs #30](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/30)

After implementing tile rendering improvement in grey box branch, tiles sometimes render the incorrect tile ID. Issue was related to passing in texture offset as an integer, rather than a float.

To test, ensure that the loaded map contains the right tiles at the correct positions.

### [P4: ID increment issue (Too many IDs used) #27](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/27)

ID Incremented around 50 times whenever the player fired a bullet. Root cause was that the bullet would create collision events with the player upon creation, which would be ignored as expected. However, each collision event was discarding 2 IDs due to implementation.

To test, fire a bullet and ensure each bullet only uses 1 new ID. The easiest way is to fire a volley of bullets and wait for them to expire. If the first bullet expiry is `Entity x has expired`, the next bullet should print `Entity x+1 has expired`.

### [P3: Bullet Targeting is Incorrect #16](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/16)

Player projectile creation was using screen coordinates. Firing would work normally at spawn, but as soon as the player moves, the screen coordinate is inaccurate because the world coordinate no longer equals the screen center. This causes some projectiles to not fire in the direction of the cursor on the screen.

To test, move the player around the map and make sure that the bullet fires where your cursor is pointed on the screen no matter where the player is positioned.