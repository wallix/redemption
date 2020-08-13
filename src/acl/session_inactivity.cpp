#include "utils/log.hpp"

#include "session_inactivity.hpp"

SessionInactivity::SessionInactivity() noexcept :
    _inactivity_timeout(0s),
    _last_activity_time(0),
    _timer_started(false)
{ }

std::chrono::seconds SessionInactivity::get_inactivity_timeout() const noexcept
{
    return _inactivity_timeout;
}

time_t SessionInactivity::get_last_activity_time() const noexcept
{
    return _last_activity_time;
}

bool SessionInactivity::activity(time_t now, bool& has_user_activity)
    noexcept
{
    if (_timer_started
        && !has_user_activity
        && _inactivity_timeout != 0s
        && now > _last_activity_time + _inactivity_timeout.count())
    {
        _last_activity_time = now;
        LOG(LOG_INFO, "User session inactivity : closing");
        return false;
    }
    if (has_user_activity)
    {
        _last_activity_time = now;
    }
    has_user_activity = false;
    return true;
}

void SessionInactivity::update_inactivity_timeout(std::chrono::seconds timeout)
{    
    _inactivity_timeout = timeout;
    if (timeout == 0s)
    {
        _inactivity_timeout = 0s;
        LOG(LOG_INFO, "User session inactivity : unlimited");
    }
    else if (timeout < ACCEPTED_TIMEOUT_MIN)
    {
        _inactivity_timeout = ACCEPTED_TIMEOUT_MIN;
        LOG(LOG_INFO,
            "Timeout wanted %ld is lower than minimum allowed, setting it to minimum",
            timeout.count());
    }
    else if (timeout > ACCEPTED_TIMEOUT_MAX)
    {
        _inactivity_timeout = 0s;
        LOG(LOG_INFO, "Inactivity timeout is larger than one week : timeout disabled");
    }
    LOG(LOG_INFO,
        "User session inactivity : set to %ld seconds",
        _inactivity_timeout.count());
}

void SessionInactivity::start_timer(std::chrono::seconds timeout, time_t start)
    noexcept
{    
    if (!_timer_started)
    {
        update_inactivity_timeout(timeout);
        _last_activity_time = start;
        _timer_started = true;
        LOG(LOG_INFO, "User session inactivity : timer has started !");
    }
}

void SessionInactivity::stop_timer() noexcept
{
    if (_timer_started)
    {
        _timer_started = false;
        LOG(LOG_INFO, "User session inactivity : timer is stopped !");
    }
}
