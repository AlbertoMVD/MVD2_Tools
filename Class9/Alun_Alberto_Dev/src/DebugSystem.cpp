#include "DebugSystem.h"
#include "extern.h"
#include "Parsers.h"

DebugSystem::~DebugSystem() {

    delete grid_shader_;
    delete icon_shader_;
}

void DebugSystem::init() {

    //init booleans
    draw_grid_ = false;
    draw_icons_ = false;
    draw_frustra_ = false;
    
    //compile debug shaders from strings in header file
    grid_shader_ = new Shader();
    grid_shader_->compileFromStrings(line_vertex_shader_, line_fragment_shader_);
    icon_shader_ = new Shader();
    icon_shader_->compileFromStrings(icon_vertex_shader_, icon_fragment_shader_);
    
    //create geometries
    createGrid_();
    createIcon_();
    createCube_();
    
    //create texture for light icon
	icon_light_texture_ = Parsers::parseTexture("data/assets/textures/icon_light.tga");
	icon_camera_texture_ = Parsers::parseTexture("data/assets/textures/icon_camera.tga");
}

//draws debug information or not
void DebugSystem::setActive(bool a) {

    draw_grid_ = a;
    draw_icons_ = a;
    draw_frustra_ = a;
}

//called once per frame
void DebugSystem::update(float dt) {
    
    //get the camera view projection matrix
    lm::mat4 vp = ECS.getComponentInArray<Camera>(ECS.main_camera).view_projection;
    
    //line drawing first
    if (draw_grid_ || draw_frustra_) {
    
        //use line shader to draw all lines and boxes
        glUseProgram(grid_shader_->program);
        GLint u_mvp = glGetUniformLocation(grid_shader_->program, "u_mvp");
        GLint u_color = glGetUniformLocation(grid_shader_->program, "u_color");
        GLint u_color_mod = glGetUniformLocation(grid_shader_->program, "u_color_mod");
        GLint u_size_scale = glGetUniformLocation(grid_shader_->program, "u_size_scale");
        GLint u_center_mod = glGetUniformLocation(grid_shader_->program, "u_center_mod");
        

    
        if (draw_grid_) {
            //set uniforms and draw grid
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, vp.m);
            glUniform3fv(u_color, 4, grid_colors);
            glUniform3f(u_size_scale, 1.0, 1.0, 1.0);
            glUniform3f(u_center_mod, 0.0, 0.0, 0.0);
            glUniform1i(u_color_mod, 0);
            glBindVertexArray(grid_vao_); //GRID
            glDrawElements(GL_LINES, grid_num_indices, GL_UNSIGNED_INT, 0);
        }
        
        if (draw_frustra_) {
            //draw frustra for all cameras
            auto& cameras = ECS.getAllComponents<Camera>();
            for (auto& cc : cameras) {
                lm::mat4 cam_iv = cc.view_matrix;
                cam_iv.inverse();
                lm::mat4 cam_ip = cc.projection_matrix;
                cam_ip.inverse();
                lm::mat4 cam_ivp = cc.view_projection;
                cam_ivp.inverse();
                lm::mat4 mvp =  vp * cam_ivp;
                
                //set uniforms and draw cube
                glUniformMatrix4fv(u_mvp, 1, GL_FALSE, mvp.m);
                glUniform1i(u_color_mod, 1); //set color to index 1 (red)
                glBindVertexArray(cube_vao_); //CUBE
                glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
            }
        }    
    }
    
    if (draw_icons_) {
        //switch to icon shader
        glUseProgram(icon_shader_->program);
        
        //get uniforms
        GLint u_mvp = glGetUniformLocation(icon_shader_->program, "u_mvp");
        GLint u_icon = glGetUniformLocation(icon_shader_->program, "u_icon");
        glUniform1i(u_icon, 0);
        
        
        //for each light - bind light texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, icon_light_texture_);
    
        auto& lights = ECS.getAllComponents<Light>();
        for (auto& curr_light : lights) {
            Transform& curr_light_transform = ECS.getComponentFromEntity<Transform>(curr_light.owner);

            lm::mat4 mvp_matrix = vp * curr_light_transform.getGlobalMatrix(ECS.getAllComponents<Transform>());;
            //BILLBOARDS
            //the mvp for the light contains rotation information. We want it to look at the camera always.
            //So we zero out first three columns of matrix, which contain the rotation information
            //this is an extremely simple billboard
            lm::mat4 bill_matrix;
            for (int i = 12; i < 16; i++) bill_matrix.m[i] = mvp_matrix.m[i];
            
            //send this new matrix as the MVP
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, bill_matrix.m);
            glBindVertexArray(icon_vao_);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        
        //bind camera texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, icon_camera_texture_);
        
        //for each camera, exactly the same but with camera texture
        auto& cameras = ECS.getAllComponents<Camera>();
        for (auto& curr_camera : cameras) {
            Transform& curr_cam_transform = ECS.getComponentFromEntity<Transform>(curr_camera.owner);
            lm::mat4 mvp_matrix = vp * curr_cam_transform.getGlobalMatrix(ECS.getAllComponents<Transform>());
            
            // billboard as above
            lm::mat4 bill_matrix;
            for (int i = 12; i < 16; i++) bill_matrix.m[i] = mvp_matrix.m[i];
            glUniformMatrix4fv(u_mvp, 1, GL_FALSE, bill_matrix.m);
            glBindVertexArray(icon_vao_);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
        }
    }
    glBindVertexArray(0);
}

///////////////////////////////////////////////
// **** Functions to create geometry ********//
///////////////////////////////////////////////

//creates a simple quad to store a light texture
void DebugSystem::createIcon_() {
    float is = 0.5f;
    GLfloat icon_vertices[12]{-is, -is, 0, is, -is, 0, is, is, 0, -is, is, 0};
    GLfloat icon_uvs[8]{ 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f };
    GLuint icon_indices[6]{ 0, 1, 2, 0, 2, 3 };
    glGenVertexArrays(1, &icon_vao_);
    glBindVertexArray(icon_vao_);
    GLuint vbo;
    //positions
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,  sizeof(icon_vertices), icon_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    //uvs
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(icon_uvs), icon_uvs, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    //indices
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(icon_indices), icon_indices, GL_STATIC_DRAW);
    //unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void DebugSystem::createCube_() {
    
    //4th component is color!
    const GLfloat quad_vertex_buffer_data[] = {
        -1.0f,  -1.0f,  -1.0f,  0.0f,  //near bottom left
        1.0f,   -1.0f,  -1.0f,  0.0f,   //near bottom right
        1.0f,   1.0f,   -1.0f,  0.0f,    //near top right
        -1.0f,  1.0f,   -1.0f,  0.0f,   //near top left
        -1.0f,  -1.0f,  1.0f,   0.0f,   //far bottom left
        1.0f,   -1.0f,  1.0f,   0.0f,    //far bottom right
        1.0f,   1.0f,   1.0f,   0.0f,     //far top right
        -1.0f,  1.0f,   1.0f,   0.0f,    //far top left
    };
    
    const GLuint quad_index_buffer_data[] = {
        0,1, 1,2, 2,3, 3,0, //top
        4,5, 5,6, 6,7, 7,4, // bottom
        4,0, 7,3, //left
        5,1, 6,2, //right
    };
    
    glGenVertexArrays(1, &cube_vao_);
    glBindVertexArray(cube_vao_);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertex_buffer_data), quad_vertex_buffer_data, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_index_buffer_data), quad_index_buffer_data, GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

//creates the debug grid for our scene
void DebugSystem::createGrid_() {
    
    std::vector<float> grid_vertices;
    const float size = 100.0f; //outer width and height
    const int div = 100; // how many divisions
    const int halfdiv = div / 2;
    const float step = size / div; // gap between divisions
    const float half = size / 2; // middle of grid
    
    float p; //temporary variable to store position
    for (int i = 0; i <= div; i++) {
        
        //lines along z-axis, need to vary x position
        p = -half + (i*step);
        //one end of line
        grid_vertices.push_back(p);
        grid_vertices.push_back(0);
        grid_vertices.push_back(half);
        if (i == halfdiv) grid_vertices.push_back(1); //color
        else grid_vertices.push_back(0);
        
        //other end of line
        grid_vertices.push_back(p);
        grid_vertices.push_back(0);
        grid_vertices.push_back(-half);
        if (i == halfdiv) grid_vertices.push_back(1); //color
        else grid_vertices.push_back(0);
        
        //lines along x-axis, need to vary z positions
        p = half - (i * step);
        //one end of line
        grid_vertices.push_back(-half);
        grid_vertices.push_back(0);
        grid_vertices.push_back(p);
        if (i == halfdiv) grid_vertices.push_back(3); //color
        else grid_vertices.push_back(0);
        
        //other end of line
        grid_vertices.push_back(half);
        grid_vertices.push_back(0);
        grid_vertices.push_back(p);
        if (i == halfdiv) grid_vertices.push_back(3); //color
        else grid_vertices.push_back(0);
    }
    
    //indices
    const int num_indices = (div + 1) * 4;
    GLuint grid_line_indices[num_indices];
    for (int i = 0; i < num_indices; i++)
        grid_line_indices[i] = i;
    
    grid_num_indices = num_indices;
    
    //gl buffers
    glGenVertexArrays(1, &grid_vao_);
    glBindVertexArray(grid_vao_);
    GLuint vbo;
    //positions
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(float), &(grid_vertices[0]), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    //indices
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grid_line_indices), grid_line_indices, GL_STATIC_DRAW);
    
    //unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

