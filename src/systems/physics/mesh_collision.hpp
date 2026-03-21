#pragma once
#include "tinyECS/components.hpp" // for Motion, vec2, MeshCollider
#include <vector>

// Narrow-phase tests
// legacy 4-arg overload (keeps backward compatibility)
bool circleIntersectsPolygon(const vec2 &circle_center, float radius, const std::vector<vec2> &poly,
                             const Motion &poly_motion);
// sprite_base_px: authoring pixel size for the polygon vertices (default 128)
bool circleIntersectsPolygon(const vec2 &circle_center, float radius, const std::vector<vec2> &poly,
                             const Motion &poly_motion, float sprite_base_px = 128.0f);
bool aabbIntersectsPolygon(const Motion &aabb_motion, const std::vector<vec2> &poly, const Motion &poly_motion);
bool polygonIntersectsPolygon(const std::vector<vec2> &polyA, const Motion &motionA, const std::vector<vec2> &polyB,
                              const Motion &motionB);

// utility
// sprite_base_px: the authoring pixel size used for vertex coordinates (default 128px)
std::vector<vec2> transformPolygon(const std::vector<vec2> &poly, const Motion &m, float sprite_base_px = 128.0f);
// Output-reference overload: reuses existing buffer to avoid heap allocation
void transformPolygon(const std::vector<vec2> &poly, const Motion &m, float sprite_base_px, std::vector<vec2> &out);

// Compute minimal translation vector to separate polyA from polyB
bool computePolygonMTV(const std::vector<vec2> &polyA_world, const std::vector<vec2> &polyB_world, vec2 &out_mtv);
// Pointer-based overload: avoids vector overhead for fixed-size polygons (e.g. wall AABBs)
bool computePolygonMTV(const vec2 *polyA, size_t countA, const vec2 *polyB, size_t countB, vec2 &out_mtv);
