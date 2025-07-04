#pragma once

class ResourceBuilder {
  public:
    inline void build() {
        buildVertices();
        buildTextures();
        buildModels();
        buildCubemaps();
        buildUniforms();
        buildShaders();
    }

    inline void destroy() {
        destroyVertices();
        destroyShaders();
        destroyUniforms();
        destroyCubemaps();
        destroyModels();
        destroyTextures();
    }

  private:
    void buildVertices();
    void destroyVertices();

    void buildTextures();
    void destroyTextures();

    void buildModels();
    void destroyModels();

    void buildCubemaps();
    void destroyCubemaps();

    void buildShaders();
    void destroyShaders();

    void buildUniforms();
    void destroyUniforms();
};
