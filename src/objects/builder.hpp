#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/mesh.hpp>

class ObjectBuilder {
  public:
    ObjectBuilder(Physbuzz::Scene *scene);
    ~ObjectBuilder();

    template <typename T>
    Physbuzz::ObjectID create(T &info) {
        Physbuzz::ObjectID id = scene->createObject();
        return create(id, info);
    }

    template <typename T>
    Physbuzz::ObjectID create(Physbuzz::ObjectID id, T &info) {
        scene->createObject(id);
        return create(info);
    }

    Physbuzz::Scene *scene;

  private:
    // Common Util Functions
    static void generateTexCoords(Physbuzz::MeshInfo &mesh);
    static void generateNormals(Physbuzz::MeshInfo &mesh);
};
