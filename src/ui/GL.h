#pragma once
#if defined(APP_GL_ES)
    #include <GLES3/gl3.h>
#elif defined(APP_GL_CORE)
    #include <glad/glad.h>
#else
    #error "Define APP_GL_ES or APP_GL_CORE (set GL_API in CMake)"
#endif