#pragma once

#include "physbuzz/collision.hpp"
#include <physbuzz/defines.hpp>
#include <physbuzz/dynamics.hpp>
#include <physbuzz/mesh.hpp>
#include <physbuzz/scene.hpp>
#include <physbuzz/texture.hpp>

class ObjectBuilder {
  public:
    ObjectBuilder(Physbuzz::Scene &scene);
    ~ObjectBuilder();

    void build();
    void destroy();

    template <typename T>
    Physbuzz::ObjectID create(Physbuzz::ObjectID id, T &info) {
        scene.createObject(id);
        return create(info);
    }

    template <typename T>
    Physbuzz::ObjectID create(T &info) {
        Physbuzz::ObjectID id = scene.createObject();
        return create(scene.getObject(id), info);
    }

    template <typename T>
    Physbuzz::ObjectID create(Physbuzz::Object &object, T &info);

    Physbuzz::Scene &scene;

  private:
    // Common Util Functions
    static void applyTransformsToMesh(const Physbuzz::TransformableComponent &transform, Physbuzz::Mesh &mesh);
    static void generate2DTexCoords(const Physbuzz::BoundingComponent &bounding, Physbuzz::Mesh &mesh);
    static void generate2DNormals(Physbuzz::Mesh &mesh);

    Physbuzz::TextureArray m_Textures;
};
