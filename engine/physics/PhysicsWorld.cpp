#include "PhysicsWorld.h"
#include <algorithm>

namespace atlas::physics {

void PhysicsWorld::Init() {
    m_bodies.clear();
    m_collisions.clear();
    m_nextId = 1;
    m_initialized = true;
}

void PhysicsWorld::Shutdown() {
    m_bodies.clear();
    m_collisions.clear();
    m_initialized = false;
}

BodyID PhysicsWorld::CreateBody(float mass, bool isStatic) {
    RigidBody body;
    body.id = m_nextId++;
    body.mass = mass > 0.0f ? mass : 1.0f;
    body.isStatic = isStatic;
    m_bodies.push_back(body);
    return body.id;
}

void PhysicsWorld::DestroyBody(BodyID id) {
    m_bodies.erase(
        std::remove_if(m_bodies.begin(), m_bodies.end(),
            [id](const RigidBody& b) { return b.id == id; }),
        m_bodies.end());
}

RigidBody* PhysicsWorld::GetBody(BodyID id) {
    for (auto& body : m_bodies) {
        if (body.id == id) return &body;
    }
    return nullptr;
}

const RigidBody* PhysicsWorld::GetBody(BodyID id) const {
    for (auto& body : m_bodies) {
        if (body.id == id) return &body;
    }
    return nullptr;
}

size_t PhysicsWorld::BodyCount() const {
    return m_bodies.size();
}

void PhysicsWorld::SetPosition(BodyID id, float x, float y, float z) {
    auto* body = GetBody(id);
    if (body) {
        body->position = {x, y, z};
    }
}

void PhysicsWorld::SetVelocity(BodyID id, float vx, float vy, float vz) {
    auto* body = GetBody(id);
    if (body) {
        body->velocity = {vx, vy, vz};
    }
}

void PhysicsWorld::ApplyForce(BodyID id, float fx, float fy, float fz) {
    auto* body = GetBody(id);
    if (body && !body->isStatic && body->mass > 0.0f) {
        body->acceleration = body->acceleration + Vec3{fx / body->mass, fy / body->mass, fz / body->mass};
    }
}

void PhysicsWorld::SetGravity(float x, float y, float z) {
    m_gravity = {x, y, z};
}

Vec3 PhysicsWorld::GetGravity() const {
    return m_gravity;
}

void PhysicsWorld::Step(float dt) {
    // Integrate velocities and positions
    for (auto& body : m_bodies) {
        if (body.isStatic || !body.active) continue;

        // Apply gravity
        body.velocity = body.velocity + m_gravity * dt;

        // Apply accumulated forces
        body.velocity = body.velocity + body.acceleration * dt;

        // Integrate position
        body.position = body.position + body.velocity * dt;

        // Clear per-frame acceleration
        body.acceleration = {0, 0, 0};
    }

    // Simple AABB collision detection
    m_collisions.clear();
    float bodyRadius = 0.5f;
    for (size_t i = 0; i < m_bodies.size(); i++) {
        for (size_t j = i + 1; j < m_bodies.size(); j++) {
            if (!m_bodies[i].active || !m_bodies[j].active) continue;

            Vec3 diff = m_bodies[i].position - m_bodies[j].position;
            float dist = diff.Length();
            if (dist < bodyRadius * 2.0f) {
                m_collisions.push_back({m_bodies[i].id, m_bodies[j].id});
            }
        }
    }
}

const std::vector<CollisionPair>& PhysicsWorld::GetCollisions() const {
    return m_collisions;
}

}
