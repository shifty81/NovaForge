#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace atlas {

struct Bone {
    std::string name;
    int parentIndex{-1};
    glm::vec3 localPosition{0.0f};
    glm::quat localRotation{1.0f, 0.0f, 0.0f, 0.0f};
};

struct Skeleton {
    std::vector<Bone> bones;

    inline void initializeBaseSkeleton() {
        bones.clear();
        bones.reserve(20);

        //                              name            parent  localPosition
        bones.push_back({"Root",         -1, {0.0f, 0.0f, 0.0f},   {1,0,0,0}});
        bones.push_back({"Pelvis",        0, {0.0f, 0.95f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Spine1",        1, {0.0f, 0.15f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Spine2",        2, {0.0f, 0.15f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Neck",          3, {0.0f, 0.15f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Head",          4, {0.0f, 0.10f, 0.0f},  {1,0,0,0}});

        // Left arm chain
        bones.push_back({"Shoulder_L",    3, {-0.18f, 0.12f, 0.0f}, {1,0,0,0}});
        bones.push_back({"UpperArm_L",    6, {-0.12f, 0.0f, 0.0f},  {1,0,0,0}});
        bones.push_back({"LowerArm_L",    7, {-0.25f, 0.0f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Hand_L",        8, {-0.22f, 0.0f, 0.0f},  {1,0,0,0}});

        // Right arm chain
        bones.push_back({"Shoulder_R",    3, {0.18f, 0.12f, 0.0f},  {1,0,0,0}});
        bones.push_back({"UpperArm_R",   10, {0.12f, 0.0f, 0.0f},   {1,0,0,0}});
        bones.push_back({"LowerArm_R",   11, {0.25f, 0.0f, 0.0f},   {1,0,0,0}});
        bones.push_back({"Hand_R",       12, {0.22f, 0.0f, 0.0f},   {1,0,0,0}});

        // Left leg chain
        bones.push_back({"UpperLeg_L",    1, {-0.10f, 0.0f, 0.0f},  {1,0,0,0}});
        bones.push_back({"LowerLeg_L",   14, {0.0f, -0.42f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Foot_L",       15, {0.0f, -0.40f, 0.05f}, {1,0,0,0}});

        // Right leg chain
        bones.push_back({"UpperLeg_R",    1, {0.10f, 0.0f, 0.0f},   {1,0,0,0}});
        bones.push_back({"LowerLeg_R",   17, {0.0f, -0.42f, 0.0f},  {1,0,0,0}});
        bones.push_back({"Foot_R",       18, {0.0f, -0.40f, 0.05f}, {1,0,0,0}});
    }
};

} // namespace atlas
