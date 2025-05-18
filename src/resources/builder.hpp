#pragma once

class ResourceBuilder {
  public:
    inline void build() {
        buildTextures();
        buildModels();
        buildCubemaps();
        buildUniforms();
        buildShaders();
    }

    inline void destroy() {
        destroyShaders();
        destroyUniforms();
        destroyCubemaps();
        destroyModels();
        destroyTextures();
    }

  private:
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
