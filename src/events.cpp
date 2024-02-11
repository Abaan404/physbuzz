#include "events.hpp"

#include "geometry/box/box.hpp"
#include "geometry/circle/circle.hpp"

#include <glad/gl.h>

void EventHandler::keyboard_keydown(SDL_KeyboardEvent &event) {
    switch (event.keysym.scancode | (1 << 30)) {
    case (SDLK_F3):
        interface.draw_interface ^= true;
        break;
    }
}

void EventHandler::keyboard_keyup(SDL_KeyboardEvent &event) {}

void EventHandler::mouse_mousedown(SDL_MouseButtonEvent &event) {
    if (interface.io.WantCaptureMouse)
        return;

    if (event.button == SDL_BUTTON_LEFT) {
        // std::shared_ptr<PhysicsCircle> circle = std::make_shared<PhysicsCircle>(glm::vec2(event.x, event.y), 50, 2.0f);
        // circle->dynamics.velocity = glm::vec2(-.05f, -.01f);
        //
        // SDL_Color color = {0, 255, 255, 255};
        // circle->texture = renderer.render_circle(circle, color);
        // objects.push_back(circle);

        std::shared_ptr<Box> box = std::make_shared<Box>(glm::vec2(event.x, event.y), 10, 10, 1.0f);
        box->draw(&renderer, GL_DYNAMIC_DRAW);
        box->dynamics.velocity = glm::vec2(-.05f, -.01f);

        objects.push_back(box);

    } else if (event.button == SDL_BUTTON_RIGHT) {
        std::shared_ptr<Circle> circle = std::make_shared<Circle>(glm::vec2(event.x, event.y), 100, 1.0f);
        circle->dynamics.velocity = glm::vec2(.1f, .1f);

        // renderer.render_circle(circle);
        objects.push_back(circle);
    }
}
