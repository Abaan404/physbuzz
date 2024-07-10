#pragma once

#include "defines.hpp"

#include <functional>
#include <memory>
#include <unordered_map>

namespace Physbuzz {

class IEvent {
  public:
    virtual ~IEvent() = default;
};

template <typename T>
class Event : public IEvent {
  public:
    EventID add(std::function<void(const T &)> &callback) {
        m_Callbacks[m_IdCount] = callback;
        return m_IdCount++;
    }

    void remove(const EventID &id) {
        m_Callbacks.erase(id);
    }

    void notify(const T &param) const {
        for (const auto &[id, callback] : m_Callbacks) {
            callback(param);
        }
    }

  private:
    EventID m_IdCount = 0;
    std::unordered_map<EventID, std::function<void(const T &)>> m_Callbacks;
};

class IEventSubject {
  public:
    template <typename T>
    EventID addCallback(std::function<void(const T &)> callback) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        return event->add(callback);
    }

    template <typename T>
    void removeCallback(EventID id) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        event->remove(id);
    }

    template <typename T>
    void notifyCallbacks(const T &params) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        event->notify(params);
    }

  protected:
    template <typename T>
    std::shared_ptr<Event<T>> getEvent() {
        std::string name = typeid(T).name();

        if (!m_EventMap.contains(name)) {
            m_EventMap[name] = std::make_shared<Event<T>>();
        }
        return std::static_pointer_cast<Event<T>>(m_EventMap[name]);
    }

    std::unordered_map<std::string, std::shared_ptr<IEvent>> m_EventMap;
};

} // namespace Physbuzz
