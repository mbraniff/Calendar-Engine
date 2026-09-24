#pragma once
#include "app/GL.h"

inline void setWireframe(bool on) {
#ifndef APP_GL_ES
    glPolygonMode(GL_FRONT_AND_BACK, on ? GL_LINE : GL_FILL);
#else
    (void)on;
#endif
}

#ifdef APP_GL_ES
    #define APP_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
#else
    #define APP_CLAMP_TO_BORDER GL_CLAMP_TO_BORDER
#endif