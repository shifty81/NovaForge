#pragma once
#include <cstdint>
#include <vector>
#include <cmath>

namespace atlas::physics {

struct Vec3 {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float Length() const { return std::sqrt(x * x + y * y + z * z); }
};

using BodyID = uint32_t;

struct RigidBody {
    BodyID id = 0;
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    float mass = 1.0f;
    float restitution = 0.5f;
    bool isStatic = false;
    bool active = true;
};

struct AABB {
    Vec3 min;
    Vec3 max;
};

struct CollisionPair {
    BodyID a = 0;
    BodyID b = 0;
};

class PhysicsWorld {
public:
    void Init();
    void Shutdown();

    BodyID CreateBody(float mass, bool isStatic = false);
    void DestroyBody(BodyID id);
    RigidBody* GetBody(BodyID id);
    const RigidBody* GetBody(BodyID id) const;
    size_t BodyCount() const;

    void SetPosition(BodyID id, float x, float y, float z);
    void SetVelocity(BodyID id, float vx, float vy, float vz);
    void ApplyForce(BodyID id, float fx, float fy, float fz);

    void SetGravity(float x, float y, float z);
    Vec3 GetGravity() const;

    void Step(float dt);

    const std::vector<CollisionPair>& GetCollisions() const;

private:
    std::vector<RigidBody> m_bodies;
    std::vector<CollisionPair> m_collisions;
    Vec3 m_gravity = {0.0f, -9.81f, 0.0f};
    BodyID m_nextId = 1;
    bool m_initialized = false;
};

}
