#pragma once

<<<<<<< HEAD
#include "../../geometry/box/box.hpp"
#include "../../geometry/circle/circle.hpp"
#include "../ui.hpp"

class ObjectList : public IUserInterface {
  public:
    ObjectList(std::vector<std::shared_ptr<GameObject>> &objects);

    std::vector<std::shared_ptr<GameObject>> &objects;

    void object_box(Box &box, Renderer &renderer);
    void object_circle(Circle &circle, Renderer &renderer);
    void draw(Renderer &renderer) override;
};
=======
// #include "../../geometry/box/box.hpp"
// #include "../../geometry/circle/circle.hpp"
// #include "../ui.hpp"
//
// class ObjectList : public IUserInterface {
//   public:
//     ObjectList(std::vector<std::shared_ptr<GameObject>> &objects);
//
//     std::vector<std::shared_ptr<GameObject>> &objects;
//
//     void object_box(Box &box, Renderer &renderer);
//     void object_circle(Circle &circle, Renderer &renderer);
//     void draw(Renderer &renderer) override;
// };
>>>>>>> ecs-rewrite
