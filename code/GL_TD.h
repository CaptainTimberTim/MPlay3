#pragma once

#pragma comment (lib, "opengl32.lib")
#include <gl/gl.h>

/*
#define GL_X_GET_PROC_ADDRESS(name) void * name(const GLubyte * procName)
typedef GL_X_GET_PROC_ADDRESS(gl_x_get_proc_address);
GL_X_GET_PROC_ADDRESS(GLXGetProcAddressStub) { }
global_variable gl_x_get_proc_address *GLXGetProcAddress_ = GLXGetProcAddressStub;
#define glXGetProcAddress GLXGetProcAddress_

#define GL_GEN_BUFFERS(name) void name(GLsizei n, GLuint * buffers)
typedef GL_GEN_BUFFERS(gl_gen_buffers);
GL_GEN_BUFFERS(GLGenBuffersStub) { }
global_variable gl_gen_buffers *GLGenBuffers_ = GLGenBuffersStub;
#define glGenBuffers GLGenBuffers_

internal void
LoadGLFunctions()
{
    HMODULE GLLibrary = LoadLibraryA("opengl32.dll");
    
    if(GLLibrary)
    {
        glXGetProcAddress = (gl_x_get_proc_address *)GetProcAddress(GLLibrary, "glXGetProcAddress");
        if(glXGetProcAddress)
        {
            glGenBuffers = (gl_gen_buffers *)glXGetProcAddress((const GLubyte *)"glGenBuffers");
        }
    }
}*/