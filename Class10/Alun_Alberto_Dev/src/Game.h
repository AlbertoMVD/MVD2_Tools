//
//  Game.h
//  02-GameLoop
//
//  Copyright � 2018 Alun Evans. All rights reserved.
//
#pragma once
#include "includes.h"
#include "GraphicsSystem.h"
#include "ControlSystem.h"
#include "DebugSystem.h"
#include "CollisionSystem.h"
#include "tools/EditorSystem.h"

class RenderToTexture;

class Game
{
public:

    unsigned int fps;
    RenderToTexture * main_buffer;

	Game();
    void init(int window_width, int window_height);
	void update(float dt);

    static Game* game_instance;

    static Game& get() {
        assert(game_instance);
        return *game_instance;
    }

    int getWidth() {
        return window_width_;
    }

    int getHeight() {
        return window_height_;
    }

    GraphicsSystem & getGraphicsSystem() {
        return graphics_system_;
    }

	//pass input straight to input system
	void updateMousePosition(int new_x, int new_y) { 
		control_system_.updateMousePosition(new_x, new_y);
	}
	void key_callback(int key, int scancode, int action, int mods) {
		control_system_.key_mouse_callback(key, action, mods);
	}
	void mouse_button_callback(int button, int action, int mods) {
		control_system_.key_mouse_callback(button, action, mods);
	}
	void update_viewports(int window_width, int window_height);

private:
	GraphicsSystem graphics_system_;
	ControlSystem control_system_;
    DebugSystem debug_system_;
    CollisionSystem collision_system_;
    EditorSystem editor_system_;

	int window_width_;
	int window_height_;
    int mouse_x_;
    int mouse_y_;
};
