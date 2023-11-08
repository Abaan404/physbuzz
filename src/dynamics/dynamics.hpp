#include "../geometry/object.hpp"
#include "../geometry/physics_box.hpp"
#include "../geometry/physics_circle.hpp"

#include "glm/glm.hpp"
#include <memory>

class Dynamics {
  public:
    static void tick_object(std::shared_ptr<GameObject> object);

  private:
    static void apply_dynamics(PhysicsCircle &circle);
    static void apply_dynamics(PhysicsBox &box);
};
