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

## Rendering

### General

- Load game: All entities, including the player and walls are visible and rendered
- Move player: The camera is centered on the player as the player moves. All rendered entities should be rendered accordingly, except for elements of the UI which are fixed to the screen.

## Entity Behaviour

Behaviours of various entity interactions, including collision, detection, and expiration.

### Walls

- Move the player to the outer walls: Player is limited to move within the map boundaries defined by the outer walls
- Move the player to inner walls, lure enemies to inner walls, fire at inner walls: Player, Enemies, Bullets cannot pass through the walls within the map

### Hazards & Power-ups

- Drive over hazards: Hazards will decrease player health
- Drive over power-up: Power-ups are printed in the terminal (M1)

### Projectiles

- Hit wall: Projectiles will bounce off walls in M1
- Expiry: Projectiles should de-spawn after 5 seconds in M1 or after hitting a target
- Hitting Self: Projectiles should not be able to hit the creator (E.g. Player bullets cannot hit player, will pass through)

### Enemies

- Player gets within detection distance: Enemy will begin shooting and chasing the player. Once it gets within a configurable range, it will stop approaching the player but will continue shooting.
- Exit detection distance: Enemy stops moving and shooting in M1

## Bug List

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
