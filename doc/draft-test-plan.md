# FishTank! Test Plan

**Last Updated:** Feb 8, 2026

## Table of Contents

- [FishTank! Test Plan](#fishtank-test-plan)
  - [Table of Contents](#table-of-contents)
  - [Player Experience](#player-experience)
    - [Movement](#movement)
    - [Shooting](#shooting)
  - [Rendering](#rendering)
    - [General](#general)
  - [Entity Behaviour](#entity-behaviour)
    - [Walls](#walls)
    - [Hazards \& Power-ups](#hazards--power-ups)
    - [Projectiles](#projectiles)
    - [Enemies](#enemies)
  - [Bug List](#bug-list)
    - [P5: Urchin enemy projectiles not appearing #52](#p5-urchin-enemy-projectiles-not-appearing-52)
    - [P4: New ID increment issue (Too many IDs used) #47](#p4-new-id-increment-issue-too-many-ids-used-47)
    - [P3: Linear Interpolated Camera causes tiles to cull too early #44](#p3-linear-interpolated-camera-causes-tiles-to-cull-too-early-44)
    - [P3: TileMap Texture Bleeding #39](#p3-tilemap-texture-bleeding-39)
    - [P2: Tileset tiles are renderering incorrect IDs #30](#p2-tileset-tiles-are-renderering-incorrect-ids-30)
    - [P4: ID increment issue (Too many IDs used) #27](#p4-id-increment-issue-too-many-ids-used-27)
    - [P3: Bullet Targeting is Incorrect #16](#p3-bullet-targeting-is-incorrect-16)

## Player Experience

Items relating directly to the player's view. (Entity interactions are in a separate section)

### Movement

- Player can use `W` key to add forward acceleration. Player speed is capped and the player will automatically decelerate when not holding `W` down to 0 velocity
- Player can use `D` key to turn left
- Player can use `A` key to turn right
- Player can use `S` will toggle quick turn. Players will not be able to `A` or `D` during the quick turn to prevent multiple conflicting actions.

### Shooting

- Player can use `Left-Click` on their mouse to shoot a projectile. The projectile should travel in the direction of the cursor on screen. Player can shoot independently of their movement with the keyboard.

### Menuing
- Player can press `T` at any time to restart tutorial.
- Player can press `Enter` to clear tutorial messages.
- Player can press `R` once they've won or lost to reset the game to after the tutorial.

## Rendering

### General

- Load game: All entities, including the player and walls are visible and rendered
- Move player: The camera is centered on the player as the player moves, offset slightly by cursor position to help the player aim. 
- All rendered entities should be rendered accordingly, except for elements of the UI which are fixed to the screen.
- All players and enemies have unique sprites.

### Animation

- Player, Enemy, Hazard, Power-Up, and bubble sprites all have animation

## Entity Behaviour

Behaviours of various entity interactions, including collision, detection, and expiration.

### Walls

- Move the player to the outer walls: Player is limited to move within the map boundaries defined by the outer walls
- Move the player to inner walls, lure enemies to inner walls, fire at inner walls: Player, Enemies, Bullets cannot pass through the walls within the map

### Hazards & Power-ups

- Drive over hazards: Hazards will decrease player health
- Drive over power-up: Drive over a medkit in order to increase player health.

### Projectiles

- Hit wall: Projectiles will bounce off walls
- Expiry: Projectiles should de-spawn after ten seconds for the player, or five seconds for the enemy, or after hitting a target
- Hitting Self: Projectiles should not be able to hit the creator (E.g. Player bullets cannot hit player, will pass through)
- Player can only fire projectiles if they have ammunition (shown in the bottom UI)

### Enemies

- Player gets within detection distance: Enemy will begin shooting and chasing the player. Once it gets within a configurable range, it will stop approaching the player but will continue shooting.
- Exit detection distance: Crab enemies will only detect the player when they are nearby, and will lose the player if they exit their detection distance. Fish enemies do not have a maximum detection distance, and always track and follow the player.
- Pathfinding: Enemies should pathfind correctly around walls towards the player.

### Mesh-Based Collision
- Player has mesh-based collider that follows the rough contour of the fish.

## Bug List

### [P4: Different coloured bullets for player vs enemy](https://github.students.cs.ubc.ca/CPSC427-2025W-T2/Team02/issues/62)

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
