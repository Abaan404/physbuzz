#include "../geometry/object.hpp"
#include <list>

class Physics {
  public:
    Physics(std::list<Object *> *objects);

    void tick();

  private:
    std::list<Object *> *objects;
};
