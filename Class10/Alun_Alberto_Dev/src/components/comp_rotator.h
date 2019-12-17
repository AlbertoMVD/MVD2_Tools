#pragma once
#include "../src/Components.h"

struct comp_rotator : Component
{
    lm::vec3 axis;
    // Data manipulation methods
    void Load(rapidjson::Value & entity, int ent_id);

    void update(float dt);
    void debugRender();
};