#pragma once

#ifdef _WIN32
#include <GL3/gl3w.h>
#else
#include <OpenGL/gl.h>
#endif

#ifdef __APPLE__
#define glBindVertexArray glBindVertexArrayAPPLE
#define glGenVertexArrays glGenVertexArraysAPPLE

#endif