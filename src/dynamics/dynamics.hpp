#include "../geometry/object.hpp"
#include "../geometry/box/box.hpp"
#include "../geometry/circle/circle.hpp"

#include <memory>

class Dynamics {
  public:
    static void tick(std::shared_ptr<GameObject> object);

  private:
    static void apply_dynamics(Circle &circle);
    static void apply_dynamics(Box &box);
};
