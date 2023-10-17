#include "../geometry/object.hpp"
#include <list>

class Physics {
  public:
    Physics(std::list<Object *> *objects);

    void step();

  private:
    std::list<Object *> *objects;
};
