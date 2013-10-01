#ifndef UI_GLUTIL_H
#define UI_GLUTIL_H

#include <vector>

#include <ui/SDL_opengl.h>


/**
 * GlUtil class.
 *
 * Static utility functions for GL calls and operations.
 *
 * @author SwarmingLogic (Roald Fernandez)
 */
class GlUtil
{
public:
  /**
   * Creates a vertex buffer, fills buffer w/data, returns name of buffer.
   */
  static GLuint prepareVertexBuffer(const std::vector<GLfloat>& data,
                                    GLenum target = GL_ARRAY_BUFFER,
                                    GLenum usage = GL_STATIC_DRAW);

  /**
   * Creates a vertex buffer, does NOT fill buffer w/data
   * Returns name of buffer.
   *
   * Use fillVertexBuffer to fill w/data.
   */
  static GLuint prepareVertexBuffer(GLsizeiptr size,
                                    GLenum target = GL_ARRAY_BUFFER,
                                    GLenum usage = GL_STATIC_DRAW);

  /**
   * Fills vertex buffer with data.
   * Make sure the buffer is bound to the target array prior to calling.
   */
  static void fillVertexBuffer(const std::vector<GLfloat>& data,
                               GLenum target = GL_ARRAY_BUFFER);
private:
  GlUtil();
 ~GlUtil();

  // NonCopyable
  GlUtil(const GlUtil& c);
  GlUtil& operator=(const GlUtil& c);
};

#endif