#include <ui/GlUtil.h>

#include <cassert>


GLuint GlUtil::prepareVertexBuffer(GLsizeiptr size,
                                   GLenum target,
                                   GLenum usage)
{
  GLuint buffername = 0;
  glGenBuffers(1, &buffername);
  glBindBuffer(target, buffername);
  glBufferData(GL_ARRAY_BUFFER, size, nullptr, usage);
  return buffername;
}

