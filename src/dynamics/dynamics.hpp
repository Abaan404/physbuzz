#include "../geometry/object.hpp"

#include "glm/glm.hpp"
#include <memory>

class Dynamics {
  public:
    static void tick_object(std::shared_ptr<GameObject> object);
};
