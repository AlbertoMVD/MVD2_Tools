#include "EditorSystem.h"
#include "../extern.h"
#include "../Game.h"
#include "../render/RenderToTexture.h"
#include "EditorGraphModule.h"
#include "ConsoleModule.h"
#include "EditorUtils.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
#include <iostream>
#include <fstream>

NodeFile node_project_;

EditorSystem::EditorSystem()
{

}

EditorSystem::~EditorSystem()
{

}

// Initialize all booleans, external modules and directories
void EditorSystem::Init()
{
    // Initialize editor systems.

    is_editor_mode = true;
    is_render_active = false;
    is_adding_component = false;
    is_saving_scene = false;
    graph_module_ = new EditorGraphModule();
    console_module_ = new ConsoleModule();

    SetStyles();
    node_project_ = ProcessDirectoryOredered("assets");

    //picking collider
    ent_picking_ray_ = ECS.createEntity("picking_ray");
    Collider& picking_ray = ECS.createComponentForEntity<Collider>(ent_picking_ray_);
    picking_ray.collider_type = ColliderTypeRay;
    picking_ray.direction = lm::vec3(0, 0, -1);
    picking_ray.max_distance = 0.001f;

}

// We can set our custom imgui styles, such as colors
void EditorSystem::SetStyles()
{
    // Set the necessary styles for ImGui
    ImGuiStyle &st = ImGui::GetStyle();
    st.FrameBorderSize = 1.0f;
    st.FramePadding = ImVec2(4.0f, 2.0f);
    st.ItemSpacing = ImVec2(4.0f, 2.0f);
    st.WindowBorderSize = 1.0f;
    st.FrameBorderSize = 1.0f;
    st.WindowRounding = 1.0f;
    st.ChildRounding = 1.0f;
    st.FrameRounding = 1.0f;
    st.ScrollbarRounding = 1.0f;
    st.GrabRounding = 1.0f;
    st.FrameRounding = 1.0f;
}

// Update the main engine window
// All subwindows will be rendered from here
void EditorSystem::update(float dt)
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(is_editor_mode)
    {
        UpdateFPS(dt);
        UpdateMenubar(dt);

        ImGui::SetNextWindowBgAlpha(0);
        ImGui::SetNextWindowPos(ImVec2(.0f, 18.f), ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(ImVec2(Game::get().game_instance->getWidth(), (Game::get().game_instance->getHeight() - 18.f)), ImGuiSetCond_Always);
        ImGui::Begin("MVD Engine", &is_editor_mode, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::SetNextWindowBgAlpha(1);
        {
            graph_module_->RenderGraph(&is_editor_mode);
            UpdateRender(dt);
            UpdateHierarchy(dt);
            UpdateInspector(dt);
            UpdateProject(dt);
            UpdateConsole(dt);
            UpdateComponentMenu(dt);
        }
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Main menu bar, used to save scene
// Add more extra functionality when needed

void EditorSystem::UpdateMenubar(float dt)
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save scene"))
            {
                is_saving_scene = true;
            }
            if (ImGui::MenuItem("Exit"))
            {
                exit(-1);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Test"))
            {

            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Windows"))
        {
            if (ImGui::MenuItem("Hierarchy.."))
            {

            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("About"))
            {

            }
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

    }

    // In case we save the scene, we need a new dialog window
    // Give a name and save into scenes folder.
    if (is_saving_scene) {
        ImGui::Begin("Save scene", &is_saving_scene);
        {
            static char elevenBytes[11] = {};
            ImGui::InputText("Name", elevenBytes, sizeof(elevenBytes));
            ImGui::AddSpace(0, 10);
            if (ImGui::Button("Save")) {
                is_saving_scene = false;
                string str(elevenBytes);
                SaveSceneToFile(str);
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                is_saving_scene = false;
            }
        }
        ImGui::End();
    }
}

// Render window, updating the image with the render to texture
// Set focus if this window is the active window
void EditorSystem::UpdateRender(float dt)
{
    ImGui::Begin("Render", &is_editor_mode);
    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImGui::GetWindowDrawList()->AddImage(
            (void *)(Game::get().main_buffer)->GetColorBuffer(), ImVec2(ImGui::GetCursorScreenPos()),
            ImGui::GetWindowSize(), ImVec2(0, 1), ImVec2(1, 0));
        render_size = lm::vec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

        if (ImGui::IsWindowFocused()) {
            is_render_active = true;
        }
        else {
            is_render_active = false;
        }
    }
    ImGui::End();
}

// Inspector update, loop through each entity and render it
void EditorSystem::UpdateInspector(float dt)
{
    ImGui::Begin("Inspector", &is_editor_mode);
    {
        int entity_id = ECS.getEntity(selected);

        if (entity_id != -1) {
            //ECS.renderEntity(entity_id);

            // Add a new component given by the user
            ImGui::PushItemWidth(-1);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 10));
            ImGui::Spacing();
            ImGui::PopStyleVar(1);
            ImGui::PushItemWidth(200);
            if (ImGui::Button("Add new component")) {
                is_adding_component = true;
            }
        }
    }
    ImGui::End();
}

// Renders the node in a hierarchy panel
// Nodes without children are text, other are treenodes.
void EditorSystem::RenderNode(TransformNode& trans) {

	auto& ent = ECS.entities[trans.entity_owner];

    //ImGui::Unindent(10);
    if (trans.children.size() > 0) {
        ImGui::Unindent(7);
        if (ImGui::TreeNode(ent.name.c_str())) {

            if (ImGui::IsItemClicked())
            {
                selected = ent.name;
            }

            for (auto& child : trans.children)
                RenderNode(child);

            ImGui::TreePop();
        }
        ImGui::Indent(7);
    }
    else {
        if (ImGui::Selectable(ent.name.c_str()))
        {
            selected = ent.name;
        }
    }
    //ImGui::Indent(10);
}

// Same as the hierarchy panel, but in this case
// we render folders and files from the project
void EditorSystem::RenderProject(NodeFile& trans) {

    if (trans.name == "") return;

    if (trans.children.size() > 0) {
        ImGui::Unindent(7);
        if (ImGui::TreeNode(trans.name.c_str())) {

            for (auto& child : trans.children)
                RenderProject(child);

            ImGui::TreePop();
        }
        ImGui::Indent(7);
    }
    else {
        if (ImGui::Selectable(trans.name.c_str()))
        {
            // Do Something
        }
    }
    //ImGui::Indent(10);
}

// Method used to update the hierarchy
// Loops through all nodes and orders them then it display.
void EditorSystem::UpdateHierarchy(float dt)
{
    ImGui::Begin("Hierarchy", &is_editor_mode);
    {
        // 1) create a temporary array with ALL transforms
        std::vector<TransformNode> transform_nodes;
        auto& all_transforms = ECS.getAllComponents<Transform>();
        for (size_t i = 0; i < all_transforms.size(); i++) {
            TransformNode tn;
            tn.trans_id = (int)i;
            tn.entity_owner = all_transforms[i].owner;
            tn.ent_name = ECS.entities[tn.entity_owner].name;
            if (all_transforms[i].parent == -1)
                tn.isTop = true;
            transform_nodes.push_back(tn);
        }

        // 2) traverse array to assign children to their parents
        for (size_t i = 0; i < transform_nodes.size(); i++) {
            int parent = all_transforms[i].parent;
            if (parent != -1) {
                transform_nodes[parent].children.push_back(transform_nodes[i]);
            }
        }

        // 3) create a new array with only top level nodes of transform tree
        std::vector<TransformNode> transform_topnodes;
        for (size_t i = 0; i < transform_nodes.size(); i++) {
            if (transform_nodes[i].isTop)
                transform_topnodes.push_back(transform_nodes[i]);
        }

        if (ImGui::TreeNode("Master Scene")) {

            for (auto& trans : transform_topnodes)
                RenderNode(trans);

            ImGui::TreePop();
        }
    }
    ImGui::End();
}

// Method used to update project files and display them
void EditorSystem::UpdateProject(float dt)
{
    ImGui::Begin("Project", &is_editor_mode);
    {
        RenderProject(node_project_);
    }
    ImGui::End();
}

// Call on console editor update
void EditorSystem::UpdateConsole(float dt)
{
    ImGui::Begin("Console", &is_editor_mode);
    {
        console_module_->update(dt);
    }
    ImGui::End();
}

// Used to draw current fps on screen
void EditorSystem::UpdateFPS(float dt)
{
    {
        //UI Window's Size
        ImGui::SetNextWindowSize(ImVec2((float)Game::get().getWidth(), (float)Game::get().getHeight()), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

        ImGui::Begin("UI", NULL,
            ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
        {
            ImGui::SetCursorPos(ImVec2(Game::get().getWidth() - Game::get().getWidth() * 0.05f, Game::get().getHeight() * 0.01f));
            ImGui::Text("FPS %d", (int)Game::get().fps);
        }

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
    }
}

// Method that loops through all entities
// Save all entities and components into json file
// Later this json is saved into the file with the name given by the user.
void EditorSystem::SaveSceneToFile(const std::string & scene_name)
{
    // TO-DO
    // Add scene saving.
}

// Method for raypicking objects
void EditorSystem::SetPickingRay(int mouse_x, int mouse_y, int screen_width, int screen_height) {

    //if we are not in debug mode (alt-0) do nothing!
    //if (!is_render_active) return;

    //convert mouse_x and mouse_y to NDC
    float ndc_x = (((float)mouse_x / (float)screen_width) * 2) - 1;
    float ndc_y = (((float)(screen_height - mouse_y) / (float)screen_height) * 2) - 1;

    //start point for ray is point on near plane of ndc
    lm::vec4 mouse_near_plane(ndc_x, ndc_y, -1.0, 1.0);

    //get view projection

    //get ray start point in world coordinates
    //don't forget this is in HOMOGENOUS coords :)
    //so we must normalize the vector

    //set the picking ray
    //the actual collision detection will be done next frame in the CollisionSystem

}

// Dialog window to add a component
void EditorSystem::UpdateComponentMenu(float dt)
{
    if(is_adding_component)
    {
        ImGui::Begin("Add component", &is_editor_mode);
        {
            // TO-DO
            // Add here the functionality 
        }
        ImGui::End();
    }
}

// Adds the given component to the object selected.
void EditorSystem::AddComponentSelected(int id)
{
    int entity_id = ECS.getEntity(selected);

    switch (id) {

    }
}

