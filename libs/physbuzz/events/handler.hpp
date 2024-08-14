#pragma once

#include "../misc/signature.hpp"
#include <functional>
#include <memory>
#include <unordered_map>

namespace Physbuzz {

using EventID = std::uint32_t;

class IEvent {
  public:
    virtual ~IEvent() = default;
};

template <typename T>
class Event : public IEvent {
  public:
    inline EventID add(std::function<void(const T &)> &callback) {
        m_Callbacks[m_IdCount] = callback;
        return m_IdCount++;
    }

    inline void remove(const EventID &id) {
        m_Callbacks.erase(id);
    }

    inline void notify(const T &param) const {
        for (const auto &[id, callback] : m_Callbacks) {
            callback(param);
        }
    }

  private:
    EventID m_IdCount = 0;
    std::unordered_map<EventID, std::function<void(const T &)>> m_Callbacks;
};

class EventSubject {
  public:
    virtual ~EventSubject() = default;

    template <typename T>
    inline EventID addCallback(std::function<void(const T &)> callback) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        return event->add(callback);
    }

    template <typename T>
    inline void eraseCallback(EventID id) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        event->remove(id);
    }

    template <typename T>
    inline void notifyCallbacks(const T &params) {
        std::shared_ptr<Event<T>> event = getEvent<T>();
        event->notify(params);
    }

    template <typename T>
    inline bool containsCallback() {
        SignatureID id = Signature::ID<T>();
        return m_EventMap.contains(id);
    }

    void clearCallbacks() {
        m_EventMap.clear();
    }

  protected:
    template <typename T>
    std::shared_ptr<Event<T>> getEvent() {
        SignatureID id = Signature::ID<T>();

        if (!m_EventMap.contains(id)) {
            m_EventMap[id] = std::make_shared<Event<T>>();
        }

        return std::static_pointer_cast<Event<T>>(m_EventMap[id]);
    }

  private:
    std::unordered_map<SignatureID, std::shared_ptr<IEvent>> m_EventMap;
};

} // namespace Physbuzz
