#pragma once

#include <physbuzz/ecs/scene.hpp>
#include <physbuzz/render/mesh.hpp>

class ObjectBuilder {
  public:
    ObjectBuilder(Physbuzz::Scene *scene);
    ~ObjectBuilder();

    template <typename T>
    Physbuzz::ObjectID create(T &info) {
        // ad-hoc counter for auto-generated object ids
        while (scene->containsObject(m_ObjectIdCounter)) {
            m_ObjectIdCounter++;
        }

        Physbuzz::ObjectID id = scene->createObject(m_ObjectIdCounter++);
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
    static void generate2DTexCoords(Physbuzz::Mesh &mesh);
    static void generate2DNormals(Physbuzz::Mesh &mesh);

    Physbuzz::ObjectID m_ObjectIdCounter = 0;
};
