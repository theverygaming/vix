#pragma once
#include <cppstd/vector.h>

template <class T>
class event {
public:
    /* when true is returned the event will be destroyed */
    bool (*f)(void *ctx, const T &data);
    void *ctx; // context passed to listener

    bool call(const T &data) {
        return f(ctx, data);
    }

private:
};

template <class T>
class event_dispatcher {
public:
    void register_listener(bool (*f)(void *ctx, const T &data), void *ctx) {
        event<T> ev;
        ev.f = f;
        ev.ctx = ctx;
        listeners.push_back(ev);
    }

    void dispatch(const T &data) {
        size_t i = 0;
        while (i < listeners.size()) {
            if (listeners[i].call(data)) {
                listeners.erase(i);
                continue;
            }
            i++;
        }
    }

private:
    std::vector<event<T>> listeners;
};
