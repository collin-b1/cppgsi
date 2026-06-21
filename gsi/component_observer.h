#pragma once
#include <cstdint>
#include <functional>

namespace cs2gsi
{
    using SubId = uint32_t;

    // Transient proxy returned by GSIDispatcher::bomb(), map(), etc.
    // Use on() to subscribe to a specific field within a component.
    // The proxy is only valid for the lifetime of the originating GSIDispatcher.
    template <typename Component>
    class ComponentObserver
    {
    public:
        using ComponentCb = std::function<void(const Component&, const Component&)>;

        ComponentObserver(
            std::function<void(SubId)> unsub,
            std::function<SubId(ComponentCb)> sub)
            : unsub_(std::move(unsub)), sub_(std::move(sub)) {}

        // Subscribe to changes in a single field. Callback receives (old_value, new_value)
        // and fires only when that field's value differs between consecutive dispatches.
        template <typename Field, typename Cb>
        SubId on(Field Component::* member, Cb&& cb)
        {
            return sub_([member, cb = std::forward<Cb>(cb)](const Component& o, const Component& n) {
                if (o.*member != n.*member)
                    cb(o.*member, n.*member);
            });
        }

        void unsubscribe(SubId id) { unsub_(id); }

    private:
        std::function<void(SubId)> unsub_;
        std::function<SubId(ComponentCb)> sub_;
    };
} // namespace cs2gsi
