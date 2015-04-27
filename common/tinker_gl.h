#pragma once

#ifdef _WIN32
#include <GL3/gl3w.h>
#else
#include <OpenGL/gl3.h>
#endif

#define CheckGLReturn() \
do { \
	GLenum e = glGetError(); \
	TAssert(e == GL_NO_ERROR); \
	PRAGMA_WARNING_PUSH \
	PRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
PRAGMA_WARNING_POP \
