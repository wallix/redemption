/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qtclient/event_manager.hpp"
#include "utils/sugar/numerics/safe_conversions.hpp"


qtclient::EventManager::EventManager()
{
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, [this]{ execute_timer(); });
}

namespace qtclient
{
    namespace
    {
        constexpr static int INITIALIZED_EVENT_FD = -2;
        static_assert(INVALID_SOCKET != INITIALIZED_EVENT_FD);

        TimeBase& update_times(EventContainer& event_container)
        {
            auto& time_base = detail::ProtectedEventContainer::get_writable_time_base(event_container);
            time_base = TimeBase::now();
            return time_base;
        }
    } // anonymous namespace
} // namespace qtclient


REDEMPTION_NOINLINE
void qtclient::EventManager::add_fdevent(Event* const& event)
{
    auto* fdevent_ptr = fdevents.emplace_back(std::unique_ptr<FdEvent>{new FdEvent{
        QSocketNotifier(event->fd, QSocketNotifier::Read),
        *event,
    }}).get();
    event->fd = INITIALIZED_EVENT_FD;
    auto fn = [this, fdevent_ptr]{ execute_fd(*fdevent_ptr); };
    QObject::connect(&fdevent_ptr->notifier, &QSocketNotifier::activated, fn);
}

REDEMPTION_NOINLINE
void qtclient::EventManager::remove_fdevent(Event* const& event)
{
    for (auto& fdevent : fdevents) {
        if (fdevent && event == &fdevent->event) {
            fdevent = std::move(fdevents.back());
            fdevents.pop_back();
            break;
        }
    }
    detail::ProtectedEventContainer::delete_event(event);
}

void qtclient::EventManager::execute_fd(FdEvent& fdevent)
{
    auto now = update_times(event_container).monotonic_time;
    // restore fd
    fdevent.event.fd = checked_int(fdevent.notifier.socket());
    fdevent.event.trigger_time = now + fdevent.event.grace_delay;
    try {
        fdevent.event.actions.exec_action(fdevent.event);
    }
    catch (Error& error) {
        // mark as initialized
        fdevent.event.fd = INITIALIZED_EVENT_FD;
        exception_notifier(error);
        update();
        return;
    }

    // mark as initialized
    fdevent.event.fd = INITIALIZED_EVENT_FD;

    update();
}

void qtclient::EventManager::execute_timer()
{
    auto now = update_times(event_container).monotonic_time;
    try {
        for (auto* event : detail::ProtectedEventContainer::get_events(event_container)) {
            if (!event->garbage && detail::trigger_event_timer(*event, now)) {
                event->actions.exec_timeout(*event);
            }
        }
    }
    catch (Error& error) {
        exception_notifier(error);
    }

    update();
}

void qtclient::EventManager::update()
{
    bool has_timer = false;
    MonotonicTimePoint trigger_time = MonotonicTimePoint::max();

    auto& events = detail::ProtectedEventContainer::get_writable_events(event_container);
    size_t i = 0;
    size_t len = events.size();
    while (i < len) {
        auto* event = events[i];
        if (REDEMPTION_LIKELY(!event->garbage)) {
            if (REDEMPTION_UNLIKELY(event->fd >= 0)) {
                add_fdevent(event);
            }
            if (event->active_timer) {
                trigger_time = std::min(trigger_time, event->trigger_time);
                has_timer = true;
            }
            ++i;
        }
        else {
            if (REDEMPTION_LIKELY(event->fd == INITIALIZED_EVENT_FD)) {
                remove_fdevent(event);
            }
            --len;
            events[i] = events[len];
        }
    }

    events.resize(len);

    if (has_timer) {
        auto delay = trigger_time - update_times(event_container).monotonic_time;
        using Duration = std::chrono::milliseconds;
        timer.start(std::max(Duration(), std::chrono::duration_cast<Duration>(delay)));
    }
    else {
        timer.stop();
    }
}
