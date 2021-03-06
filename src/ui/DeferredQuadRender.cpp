#include <ui/DeferredQuadRender.h>

#include <math/MathUtil.h>
#include <math/Random.h>
#include <ui/GlState.h>
#include <ui/GlUtil.h>
#include <util/Asset.h>


DeferredQuadRender::DeferredQuadRender()
  :  log_("DeferredQuadRender"),
     program_(),
     viewport_(0, 0),
     textures_(),
     vertexBuffer_(0),
     vertices_(),
     timeParamId_(-1),
     projectionMatID_(-1),
     modelViewMatID_(-1),
     modelViewMat_(0.0f),
     lightPosID_(-1),
     lightPos_(0.0f),
     viewportID_(-1),
     timeID_(-1),
     ssaoDataBuffer_(0)
{
  for (auto& surface : textures_) {
    surface.setIsMaxFiltering(false);
  }

  for (size_t i = 0 ; i < 64 ; ++i) {
    ssaoData_.pos[i][0] = Random::get(-1.0f, 1.0f);
    ssaoData_.pos[i][1] = Random::get(-1.0f, 1.0f);
    ssaoData_.pos[i][2] = Random::get(-1.0f, 1.0f);
    ssaoData_.pos[i][3] = 0.0f;
    ssaoData_.pos[i] = glm::normalize(ssaoData_.pos[i]);

    float scale = static_cast<float>(i) / 64.0f;
    scale = MathUtil::lerp(0.1f, 1.0f, scale * scale);
    ssaoData_.pos[i] *= scale;
  }
  for (size_t i = 0 ; i < 64 ; ++i) {
    ssaoData_.vec[i][0] = Random::get(-1.0f, 1.0f);
    ssaoData_.vec[i][1] = Random::get(-1.0f, 1.0f);
    ssaoData_.vec[i][2] = Random::get(-1.0f, 1.0f);
    ssaoData_.vec[i][3] = Random::get(-1.0f, 1.0f);
  }


  glGenBuffers(1, &ssaoDataBuffer_);
  glBindBuffer(GL_UNIFORM_BUFFER, ssaoDataBuffer_);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(SsaoData),
               &ssaoData_, GL_STATIC_DRAW);
}


DeferredQuadRender::~DeferredQuadRender()
{
}


void DeferredQuadRender::initialize()
{
  log_.d("Initializing");
  program_.setShader(ShaderProgram::VERTEX,
                     AssetShader("deferredquad.vert"));
  program_.setShader(ShaderProgram::FRAGMENT,
                     AssetShader("deferredquad.frag"));
  updateShader();

  float s = 1.0f;
  vertices_ = {
    -s, -s, 0.0f,
     s, -s, 0.0f,
    -s,  s, 0.0f,
     s,  s, 0.0f
  };
  vertexBuffer_  = GlUtil::prepareVertexBuffer(vertices_);
}



void DeferredQuadRender::render(float time) const
{
  GlState::enable(GlState::BLEND);
  GlState::enable(GlState::DEPTH_TEST);

  GlState::useProgram(program_.get());

  // Uniform PositionData
  if (uniformTextureIds_[GBUFFERTARGET_COLOR] >= 0) {
    GlState::activeTexture(GL_TEXTURE2);
    textures_[GBUFFERTARGET_COLOR].glBind();
    glUniform1i(uniformTextureIds_[GBUFFERTARGET_COLOR], 2);
  }

  if (uniformTextureIds_[GBUFFERTARGET_NORMAL] >= 0) {
    GlState::activeTexture(GL_TEXTURE1);
    textures_[GBUFFERTARGET_NORMAL].glBind();
    glUniform1i(uniformTextureIds_[GBUFFERTARGET_NORMAL], 1);
  }

  if (uniformTextureIds_[GBUFFERTARGET_POSITION] >= 0) {
    GlState::activeTexture(GL_TEXTURE0);
    textures_[GBUFFERTARGET_POSITION].glBind();
    glUniform1i(uniformTextureIds_[GBUFFERTARGET_POSITION], 0);
  }

  if (modelViewMatID_ >= 0)
    glUniformMatrix4fv(modelViewMatID_, 1, GL_FALSE, &modelViewMat_[0][0]);

  if (projectionMatID_ >= 0)
    glUniformMatrix4fv(projectionMatID_, 1, GL_FALSE, &projectionMat_[0][0]);

  // Uniform Time
  if (timeParamId_ >= 0)
    glUniform1f(timeParamId_, time);

  // Light position
  if (lightPosID_ >= 0)
    glUniform4fv(lightPosID_, 1, &lightPos_[0]);

  if (viewportID_ >= 0)
    glUniform2iv(viewportID_, 1, viewport_.getData());

  if (timeID_ >= 0)
    glUniform1f(timeID_, time);

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ssaoDataBuffer_);

  // vec3 vpos
  glEnableVertexAttribArray(0);
  GlState::bindBuffer(GlState::ARRAY_BUFFER, vertexBuffer_);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glDisableVertexAttribArray(0);
}

void DeferredQuadRender::refresh()
{
  if(program_.isModified()) updateShader();
}



void DeferredQuadRender::handleResize(int width, int height)
{
  if (viewport_.w() == width && viewport_.h() == height)
    return;

  viewport_ = Size(width, height);
}


void DeferredQuadRender::setGlTextureId(GBufferTarget target, GLuint textureId)
{
  assert(target != N_GBUFFERTARGETS);
  textures_[target].setId(textureId);
}


void DeferredQuadRender::setModelViewMat(const glm::mat4 modelViewMat)
{
  modelViewMat_ = modelViewMat;
  lightPos_ = modelViewMat_ * glm::vec4(-8,10, 8, 1);
}

void DeferredQuadRender::setProjectionMat(const glm::mat4 projectionMat)
{
  projectionMat_ = projectionMat;
}


void DeferredQuadRender::updateShader()
{
  program_.compile();

  // Get uniform id's matching compiled shader.
  for (size_t i = 0 ; i < N_GBUFFERTARGETS ; ++i) {
    uniformTextureIds_[i] =
      glGetUniformLocation(program_.get(),
                           getUniformName(static_cast<GBufferTarget>(i)).c_str());

    if (uniformTextureIds_[i] < 0 )
      log_.w() << getUniformName(static_cast<GBufferTarget>(i))
               << " uniform not found" << Log::end;
  }

  modelViewMatID_  = glGetUniformLocation(program_.get(), "ModelViewMat");
  projectionMatID_ = glGetUniformLocation(program_.get(), "ProjectMat");
  lightPosID_      = glGetUniformLocation(program_.get(), "lightPos");
  viewportID_      = glGetUniformLocation(program_.get(), "viewport");
  timeID_          = glGetUniformLocation(program_.get(), "time");

  // if(modelViewMatID_  < 0) log_.w("ModelViewMat uniform not found");
  // if(projectionMatID_ < 0) log_.e("ProjectMat uniform not found");
  if(lightPosID_      < 0) log_.w("lightPos uniform not found");
  if(viewportID_      < 0) log_.w("viewport uniform not found");
  if(timeID_          < 0) log_.w("time uniform not found");
}

std::string DeferredQuadRender::getUniformName(GBufferTarget target)
{
  switch (target) {
  case GBUFFERTARGET_POSITION:
    return "PositionData";
  case GBUFFERTARGET_NORMAL:
    return "NormalData";
  case GBUFFERTARGET_COLOR:
    return "ColorData";
  case N_GBUFFERTARGETS:
  default:
    return "";
  }
}
