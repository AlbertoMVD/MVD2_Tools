#pragma once
#include "Components.h"
#include "Game.h"
#include "extern.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

/**** COMPONENTS ****/

void Transform::Save(rapidjson::Document& json, rapidjson::Value & entity)
{

}

void Transform::Load(rapidjson::Value & entity, int ent_id) {

    auto jt = entity["transform"]["translation"].GetArray();
    auto jr = entity["transform"]["rotation"].GetArray();
    auto js = entity["transform"]["scale"].GetArray();

    lm::vec3 rotate; rotate.x = jr[0].GetFloat(); rotate.y = jr[1].GetFloat(); rotate.z = jr[2].GetFloat();
    lm::quat qR(rotate.x*DEG2RAD, rotate.y*DEG2RAD, rotate.z*DEG2RAD);
    lm::mat4 R;
    R.rotateLocal(rotate.x*DEG2RAD, lm::vec3(1, 0, 0));
    R.rotateLocal(rotate.y*DEG2RAD, lm::vec3(0, 1, 0));
    R.rotateLocal(rotate.z*DEG2RAD, lm::vec3(0, 0, 1));
    this->set(*this * R);
    this->scaleLocal(js[0].GetFloat(), js[1].GetFloat(), js[2].GetFloat());
    this->translate(jt[0].GetFloat(), jt[1].GetFloat(), jt[2].GetFloat());
}

void Transform::debugRender() {


}


void Mesh::Save(rapidjson::Document& json, rapidjson::Value & entity)
{

}

void Mesh::Load(rapidjson::Value & entity, int ent_id) {
 
}

void Mesh::debugRender() {

}

//Light Component
// - id of transform in ECS array
// - color of light
//Later will be developed extensively


void Light::Save(rapidjson::Document& json, rapidjson::Value & entity)
{

}

void Light::Load(rapidjson::Value & entity, int ent_id) {

    auto json_lc = entity["light"]["color"].GetArray();
    this->color = lm::vec3(json_lc[0].GetFloat(), json_lc[1].GetFloat(), json_lc[2].GetFloat());
}

void Light::debugRender() {


}

//ColliderComponent. Only specifies size - collider location is given by any
//associated TransformComponent
// - collider_type is the type according to enum above
// - local_center is offset from transform
// - local_halfwidth is used for box,
// - direction is used for ray
// - max_distance is used to convert ray to segment


void Collider::Save(rapidjson::Document& json, rapidjson::Value & entity)
{

}

void Collider::Load(rapidjson::Value & entity, int ent_id) {

    std::string coll_type = entity["collider"]["type"].GetString();
    if (coll_type == "box") {
        this->collider_type = ColliderTypeBox;

        auto json_col_center = entity["collider"]["center"].GetArray();
        this->local_center.x = json_col_center[0].GetFloat();
        this->local_center.y = json_col_center[1].GetFloat();
        this->local_center.z = json_col_center[2].GetFloat();

        auto json_col_halfwidth = entity["collider"]["halfwidth"].GetArray();
        this->local_halfwidth.x = json_col_halfwidth[0].GetFloat();
        this->local_halfwidth.y = json_col_halfwidth[1].GetFloat();
        this->local_halfwidth.z = json_col_halfwidth[2].GetFloat();
    }
}

void Collider::debugRender() {

}

void Entity::Save(rapidjson::Document& json, rapidjson::Value & entity) {


}