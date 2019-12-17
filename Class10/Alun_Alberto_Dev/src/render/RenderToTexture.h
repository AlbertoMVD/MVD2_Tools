#pragma once
#include "../includes.h"


class RenderToTexture
{

protected:

    GLuint frambuffer_name_ = 0;
    GLuint colorbuffer_;
    GLuint depthbuffer_;

public:
    RenderToTexture();
    RenderToTexture(const char* name, int xres, int yres);
    ~RenderToTexture();

    bool Init();
    void Activate();
    void Deactivate();
    void Destroy();

    GLuint GetFrameBufferName() {
        return frambuffer_name_;
    }

    GLuint GetColorBuffer() {
        return colorbuffer_;
    }

    GLuint GetDepthBuffer() {
        return depthbuffer_;
    }

private:

    int xres_ = 0;
    int yres_ = 0;
    const char* name_;
};
