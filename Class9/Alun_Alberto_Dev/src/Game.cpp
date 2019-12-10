//
//  Game.cpp
//
//  Copyright � 2018 Alun Evans. All rights reserved.
//

#include "Game.h"
#include "Shader.h"
#include "extern.h"
#include "Parsers.h"
#include "render/RenderToTexture.h"

Game* Game::game_instance = nullptr;

Game::Game() {

    assert(game_instance == nullptr);
    game_instance = this;
}

int createFree(float aspect, ControlSystem& sys) {

	int ent_player = ECS.createEntity("PlayerFree");
	Camera& player_cam = ECS.createComponentForEntity<Camera>(ent_player);
	lm::vec3 the_position(10.0f, 6.0f, -5.0f);
	ECS.getComponentFromEntity<Transform>(ent_player).translate(the_position);
	player_cam.position = the_position;
	player_cam.forward = lm::vec3(-1.0f, -0.5f, 0.0f);
	player_cam.setPerspective(60.0f*DEG2RAD, aspect, 0.1f, 10000.0f);

	ECS.main_camera = ECS.getComponentID<Camera>(ent_player);

	sys.control_type = ControlTypeFree;

	return ent_player;
}

//Nothing here yet
void Game::init(int window_width, int window_height) {

	//******* INIT SYSTEMS *******

	//init systems except debug, which needs info about scene
	control_system_.init();
	graphics_system_.init(window_width_, window_height_);
    editor_system_.Init();

    //******** AUTOMATIC LOADING **********//
    
    // load a shader into the graphics system
    // Be very careful, on loading extra shaders,needs more support.
    graphics_system_.loadShader("phong", "data/shaders/phong.vert", "data/shaders/phong.frag");

	//Use this method to load an scene file we previously exported to our project
    Parsers::parseScene("data/assets/scenes/scene_test.scene", graphics_system_);

	//******** MANUAL LOADING **********//

    //geometries
    int teapot_geom_id = graphics_system_.createGeometryFromFile("data/assets/meshes/teapot_small.obj");
    int plane_geom_id = graphics_system_.createPlaneGeometry();

    //materials and textures
    int default_mat_id = graphics_system_.createMaterial();
    graphics_system_.getMaterial(default_mat_id).diffuse_map = Parsers::parseTexture("data/assets/textures/test.tga");
    graphics_system_.getMaterial(default_mat_id).shader_id = graphics_system_.getShaderProgram("phong");

    //******* CREATE OTHER ENTITIES AND ADD COMPONENTS *******//
    int ent_teapot = ECS.createEntity("Teapot");
    Mesh& tmc = ECS.createComponentForEntity<Mesh>(ent_teapot);
    tmc.geometry = teapot_geom_id;
    tmc.material = default_mat_id;
    ECS.getComponentFromEntity<Transform>(ent_teapot).translate(0.0, -0.5, 0.0);

    int ent_light1 = ECS.createEntity("Light 1");
    ECS.createComponentForEntity<Light>(ent_light1);
    ECS.getComponentFromEntity<Transform>(ent_light1).translate(10.0f, 5.0f, 10.0f);
    ECS.getComponentFromEntity<Light>(ent_light1).color = lm::vec3(1.0f, 1.0f, 1.0f);

    int ent_light2 = ECS.createEntity("Light 2");
    ECS.createComponentForEntity<Light>(ent_light2);
    ECS.getComponentFromEntity<Transform>(ent_light2).translate(-10.0f, -10.0f, 10.0f);
    ECS.getComponentFromEntity<Light>(ent_light2).color = lm::vec3(0.5f, 0.5f, 1.0f);

	createFree((float)window_width_ / (float)window_height_, control_system_);

    //******* INIT DEBUG SYSTEM *******
    debug_system_.init();
    debug_system_.setActive(true);

    main_buffer = new RenderToTexture("main_buffer", window_width, window_height);
}

//Entry point for game update code
void Game::update(float dt) {

	//update each system in turn

	//update input
	control_system_.update(dt);

    //collision
    collision_system_.update(dt);

	//render
	graphics_system_.update(dt);
    
    //debug
    debug_system_.update(dt);

    // Rendering modules
    {
        if (editor_system_.GetEditorStatus()) {
            main_buffer->Activate();
            graphics_system_.update(dt);
            debug_system_.update(dt);
            main_buffer->Deactivate();
        }
        else {
            graphics_system_.update(dt);
            debug_system_.update(dt);
        }
    }

    // Components
    ECS.update(dt);

    // Editor window
    editor_system_.update(dt);
   
}
//update game viewports
void Game::update_viewports(int window_width, int window_height) {

	window_width_ = window_width;
	window_height_ = window_height;

	auto& cameras = ECS.getAllComponents<Camera>();
	for (auto& cam : cameras)
		cam.setPerspective(60.0f*DEG2RAD, (float)window_width_ / (float) window_height_, 0.01f, 10000.0f);

	graphics_system_.updateMainViewport(window_width_, window_height_);
}


