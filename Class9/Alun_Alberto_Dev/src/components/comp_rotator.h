#pragma once
#include "../src/Components.h"

struct comp_rotator : Component
{
    // Data manipulation methods
    void Load(rapidjson::Value & entity, int ent_id);

    void update(float dt);
};