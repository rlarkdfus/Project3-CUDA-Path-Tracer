#pragma once

#include "sceneStructs.h"

#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>

#include <cfloat>


/**
 * Handy-dandy hash function that provides seeds for random number generation.
 */
__host__ __device__ inline unsigned int utilhash(unsigned int a)
{
    a = (a + 0x7ed55d16) + (a << 12);
    a = (a ^ 0xc761c23c) ^ (a >> 19);
    a = (a + 0x165667b1) + (a << 5);
    a = (a + 0xd3a2646c) ^ (a << 9);
    a = (a + 0xfd7046c5) + (a << 3);
    a = (a ^ 0xb55a4f09) ^ (a >> 16);
    return a;
}

// CHECKITOUT
/**
 * Compute a point at parameter value `t` on ray `r`.
 * Falls slightly short so that it doesn't intersect the object it's hitting.
 */
__host__ __device__ inline glm::vec3 getPointOnRay(Ray r, float t)
{
    return r.origin + (t - .0001f) * glm::normalize(r.direction);
}

/**
 * Multiplies a mat4 and a vec4 and returns a vec3 clipped from the vec4.
 */
__host__ __device__ inline glm::vec3 multiplyMV(glm::mat4 m, glm::vec4 v)
{
    return glm::vec3(m * v);
}

// CHECKITOUT
/**
 * Test intersection between a ray and a transformed cube. Untransformed,
 * the cube ranges from -0.5 to 0.5 in each axis and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float boxIntersectionTest(
    Geom box,
    Ray r,
    glm::vec3& intersectionPoint,
    glm::vec3& normal,
    bool& outside);

// CHECKITOUT
/**
 * Test intersection between a ray and a transformed sphere. Untransformed,
 * the sphere always has radius 0.5 and is centered at the origin.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float sphereIntersectionTest(
    Geom sphere,
    Ray r,
    glm::vec3& intersectionPoint,
    glm::vec3& normal,
    bool& outside);

/**
 * Slab test against an axis-aligned box: the box is the intersection of three
 * pairs of parallel planes, so intersecting the ray's entry/exit interval for
 * each axis and checking the result is non-empty answers the whole query in
 * ~6 divisions, with no cross products or square roots.
 *
 * Conservative by construction -- the box contains every triangle, so a ray
 * that misses the box provably misses them all. A ray clipping an empty corner
 * is a harmless false positive that falls through to the triangle loop.
 */
__host__ __device__ inline bool aabbIntersectionTest(
    const glm::vec3& bboxMin,
    const glm::vec3& bboxMax,
    const Ray& r)
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    for (int xyz = 0; xyz < 3; ++xyz)
    {
        // A zero component gives +/-inf here, which the min/max below handle
        // correctly for a ray running parallel to this pair of planes.
        float inv = 1.0f / r.direction[xyz];
        float t1 = (bboxMin[xyz] - r.origin[xyz]) * inv;
        float t2 = (bboxMax[xyz] - r.origin[xyz]) * inv;
        if (t1 > t2)
        {
            float tmp = t1;
            t1 = t2;
            t2 = tmp;
        }
        tmin = glm::max(tmin, t1);   // latest entry wins
        tmax = glm::min(tmax, t2);   // earliest exit wins
        if (tmax < tmin)
        {
            return false;
        }
    }

    return tmax > 0.0f;              // box entirely behind the ray origin
}

/*
 * Test intersection between a ray and a transformed triangle mesh, by
 * transforming the ray into the mesh's object space and testing every triangle
 * in the mesh's slice of the scene-wide triangle buffer.
 *
 * @param intersectionPoint  Output parameter for point of intersection.
 * @param normal             Output parameter for surface normal.
 * @param outside            Output param for whether the ray came from outside.
 * @return                   Ray parameter `t` value. -1 if no intersection.
 */
__host__ __device__ float meshIntersectionTest(
    Geom mesh,
    const Triangle* triangles,
    Ray r,
    glm::vec3& intersectionPoint,
    glm::vec3& normal,
    bool& outside);
