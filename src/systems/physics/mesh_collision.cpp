#include "mesh_collision.hpp"
#include <algorithm>
#include <cfloat>
#include <cmath>

static float projectMin(const std::vector<float> &projs) { return *std::min_element(projs.begin(), projs.end()); }
static float projectMax(const std::vector<float> &projs) { return *std::max_element(projs.begin(), projs.end()); }

// Output-reference overload: reuses existing buffer to avoid heap allocation on every call.
void transformPolygon(const std::vector<vec2> &poly, const Motion &m, float sprite_base_px, std::vector<vec2> &out) {
    out.resize(poly.size());
    float ca = cos(m.angle);
    float sa = sin(m.angle);

    // Convert motion.scale (which is in pixels) to a model-space scale factor using provided base
    vec2 modelScale = {m.scale.x / sprite_base_px, m.scale.y / sprite_base_px};
    for (size_t i = 0; i < poly.size(); ++i) {
        vec2 s = {poly[i].x * modelScale.x, poly[i].y * modelScale.y};
        vec2 r = {ca * s.x - sa * s.y, sa * s.x + ca * s.y};
        out[i] = r + m.position;
    }
}

// value-returning overload (convenience; delegates to the buffer version).
std::vector<vec2> transformPolygon(const std::vector<vec2> &poly, const Motion &m, float sprite_base_px) {
    std::vector<vec2> out;
    transformPolygon(poly, m, sprite_base_px, out);
    return out;
}

static void polygonAxes(const std::vector<vec2> &poly, std::vector<vec2> &axes) {
    // produce edge normals (perpendicular) for SAT
    size_t n = poly.size();
    for (size_t i = 0; i < n; ++i) {
        vec2 a = poly[i];
        vec2 b = poly[(i + 1) % n];
        vec2 edge = b - a;
        vec2 axis = normalize(vec2(-edge.y, edge.x)); // outward normal (unit)
        axes.push_back(axis);
    }
}

static void projectOntoAxis(const std::vector<vec2> &poly, const vec2 &axis, std::vector<float> &out) {
    out.clear();
    out.reserve(poly.size());
    for (const vec2 &p : poly)
        out.push_back(dot(p, axis));
}

static bool overlapOnAxis(const std::vector<vec2> &polyA, const std::vector<vec2> &polyB, const vec2 &axis) {
    std::vector<float> pa, pb;
    projectOntoAxis(polyA, axis, pa);
    projectOntoAxis(polyB, axis, pb);
    float minA = projectMin(pa), maxA = projectMax(pa);
    float minB = projectMin(pb), maxB = projectMax(pb);
    return !(maxA < minB || maxB < maxA);
}

bool polygonIntersectsPolygon(const std::vector<vec2> &polyA_local, const Motion &motionA,
                              const std::vector<vec2> &polyB_local, const Motion &motionB) {
    auto A = transformPolygon(polyA_local, motionA);
    auto B = transformPolygon(polyB_local, motionB);

    std::vector<vec2> axes;
    polygonAxes(A, axes);
    polygonAxes(B, axes);

    for (const vec2 &axis : axes) {
        if (!overlapOnAxis(A, B, axis))
            return false;
    }
    return true;
}

// closest point on segment AB to point P
static vec2 closestPointSegment(const vec2 &A, const vec2 &B, const vec2 &P) {
    vec2 AB = B - A;
    float denom = dot(AB, AB);
    if (denom <= 0.f)
        return A;
    float t = dot(P - A, AB) / denom;
    t = std::clamp(t, 0.0f, 1.0f);
    return A + AB * t;
}

bool circleIntersectsPolygon(const vec2 &circle_center, float radius, const std::vector<vec2> &poly_local,
                             const Motion &poly_motion, float sprite_base_px) {
    auto poly = transformPolygon(poly_local, poly_motion, sprite_base_px);
    // SAT axes: polygon edge normals
    std::vector<vec2> axes;
    polygonAxes(poly, axes);

    // Also include axis from circle center to closest point on polygon (handles vertex-edge cases)
    float minDist2 = FLT_MAX;
    vec2 closest = {0, 0};
    for (size_t i = 0; i < poly.size(); ++i) {
        vec2 A = poly[i];
        vec2 B = poly[(i + 1) % poly.size()];
        vec2 cp = closestPointSegment(A, B, circle_center);
        float d2 = dot(cp - circle_center, cp - circle_center);
        if (d2 < minDist2) {
            minDist2 = d2;
            closest = cp;
        }
    }
    vec2 centerDir = closest - circle_center;
    float centerLen = length(centerDir);
    const float EPS = 1e-6f;
    if (centerLen > EPS)
        axes.push_back(centerDir / centerLen);

    // Project polygon and circle onto each axis
    std::vector<float> projPoly;
    for (const vec2 &axis : axes) {
        projectOntoAxis(poly, axis, projPoly);
        float minP = projectMin(projPoly), maxP = projectMax(projPoly);
        float centerProj = dot(circle_center, axis);
        float minC = centerProj - radius, maxC = centerProj + radius;
        if (maxP < minC || maxC < minP)
            return false;
    }
    return true;
}

// legacy 4-arg wrapper kept for compatibility with older call sites
bool circleIntersectsPolygon(const vec2 &circle_center, float radius, const std::vector<vec2> &poly_local,
                             const Motion &poly_motion) {
    return circleIntersectsPolygon(circle_center, radius, poly_local, poly_motion, 128.0f);
}

bool aabbIntersectsPolygon(const Motion &aabb_motion, const std::vector<vec2> &poly_local, const Motion &poly_motion) {
    // Convert AABB to polygon (world-space)
    vec2 half = aabb_motion.scale / 2.f;
    std::vector<vec2> aabbPoly = {
        aabb_motion.position + vec2{-half.x, -half.y}, aabb_motion.position + vec2{half.x, -half.y},
        aabb_motion.position + vec2{half.x, half.y}, aabb_motion.position + vec2{-half.x, half.y}};

    // Transform the local polygon into world-space (this respects motion.scale and angle)
    auto polyWorld = transformPolygon(poly_local, poly_motion);

    // Collect SAT axes from both polygons
    std::vector<vec2> axes;
    polygonAxes(aabbPoly, axes);
    polygonAxes(polyWorld, axes);

    // SAT test on all axes
    for (const vec2 &axis : axes) {
        if (!overlapOnAxis(aabbPoly, polyWorld, axis))
            return false; // separating axis found
    }
    return true; // no separating axis -> intersection
}
// zero-allocation SAT MTV: works directly on pointer+count, no heap vectors allocated.
bool computePolygonMTV(const vec2 *polyA, size_t countA, const vec2 *polyB, size_t countB, vec2 &out_mtv) {
    float minOverlap = FLT_MAX;
    vec2 smallestAxis = {0.f, 0.f};

    // Compute polygon centers for direction disambiguation
    vec2 centerA = {0.f, 0.f};
    for (size_t i = 0; i < countA; ++i)
        centerA += polyA[i];
    centerA /= (float)countA;
    vec2 centerB = {0.f, 0.f};
    for (size_t i = 0; i < countB; ++i)
        centerB += polyB[i];
    centerB /= (float)countB;

    // Lambda: project both polygons onto axis, check overlap, track minimum.
    // returns false if a separating axis is found.
    auto testAxis = [&](const vec2 &axis) -> bool {
        float minA = FLT_MAX, maxA = -FLT_MAX;
        for (size_t i = 0; i < countA; ++i) {
            float d = dot(polyA[i], axis);
            if (d < minA)
                minA = d;
            if (d > maxA)
                maxA = d;
        }
        float minB = FLT_MAX, maxB = -FLT_MAX;
        for (size_t i = 0; i < countB; ++i) {
            float d = dot(polyB[i], axis);
            if (d < minB)
                minB = d;
            if (d > maxB)
                maxB = d;
        }
        // check for gap
        if (maxA < minB || maxB < minA) {
            // no intersection on this axis -> separating axis found
            return false;
        }
        // compute overlap on axis
        float overlap = std::min(maxA, maxB) - std::max(minA, minB);
        if (overlap < minOverlap) {
            minOverlap = overlap;
            smallestAxis = axis;
        }
        return true;
    };

    // Test edge normals from polyA
    for (size_t i = 0; i < countA; ++i) {
        vec2 edge = polyA[(i + 1) % countA] - polyA[i];
        vec2 normal = {-edge.y, edge.x};
        float len = length(normal);
        if (len < 1e-10f)
            continue;
        if (!testAxis(normal / len))
            return false;
    }
    // Test edge normals from polyB
    for (size_t i = 0; i < countB; ++i) {
        vec2 edge = polyB[(i + 1) % countB] - polyB[i];
        vec2 normal = {-edge.y, edge.x};
        float len = length(normal);
        if (len < 1e-10f)
            continue;
        if (!testAxis(normal / len))
            return false;
    }

    if (minOverlap == FLT_MAX)
        return false; // shouldn't happen

    // Ensure axis points from A toward outside of B (so moving A by +mtv resolves intersection)
    vec2 dir = centerB - centerA;
    if (dot(dir, smallestAxis) < 0.f)
        smallestAxis = -smallestAxis;

    out_mtv = smallestAxis * minOverlap;
    return true;
}

// vector overload delegates to the pointer-based version (no extra allocations).
bool computePolygonMTV(const std::vector<vec2> &polyA_world, const std::vector<vec2> &polyB_world, vec2 &out_mtv) {
    return computePolygonMTV(polyA_world.data(), polyA_world.size(), polyB_world.data(), polyB_world.size(), out_mtv);
}
