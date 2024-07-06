#pragma once

namespace Physbuzz {

class Scene;
class Contact;

struct OnCollisionDetectEvent {
    Scene &scene;
    Contact &contact;
};

struct OnCollisionResolveEvent {
    Scene &scene;
    const Contact &contact;
};

} // namespace Physbuzz
