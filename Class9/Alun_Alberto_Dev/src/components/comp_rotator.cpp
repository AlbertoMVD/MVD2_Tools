#include "comp_rotator.h"

void comp_rotator::Load(rapidjson::Value & entity, int ent_id)
{

}

void comp_rotator::update(float dt)
{

}

void comp_rotator::debugRender()
{
    std::cout << "updating render ui" << std::endl;

    lm::vec3 axis = this->axis;
    float axis_array[3] = { axis.x, axis.y, axis.z };

    ImGui::AddSpace(0, 5);
    {
        if (ImGui::TreeNode("Transform")) {
            ImGui::AddSpace(0, 5);
            if (ImGui::DragFloat3("Axis", axis_array)) {
                //this->axis = (axis_array[0], axis_array[1], axis_array[2]);
            }
            ImGui::TreePop();
        }
    }
}
