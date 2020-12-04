#include "acl/session_inactivity.hpp"
#include "utils/difftimeval.hpp"
#include "utils/log.hpp"


namespace
{
    constexpr std::chrono::seconds ACCEPTED_TIMEOUT_MIN = 30s;
    constexpr std::chrono::seconds ACCEPTED_TIMEOUT_MAX = 168h; // one week
}

bool SessionInactivity::activity(time_t now, bool& has_user_activity)
{
    if (_timer_started
     && !has_user_activity
     && _inactivity_timeout != 0s
     && now > _last_activity_time + _inactivity_timeout.count()
    ){
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
    else
    {
        LOG(LOG_INFO,
            "User session inactivity : set to %ld seconds",
            _inactivity_timeout.count());
    }
}

void SessionInactivity::start_timer(std::chrono::seconds timeout, time_t start)
{
    if (!_timer_started)
    {
        update_inactivity_timeout(timeout);
        _last_activity_time = start;
        _timer_started = true;
        LOG(LOG_INFO, "User session inactivity : timer has started !");
    }
}

void SessionInactivity::stop_timer()
{
    if (_timer_started)
    {
        _timer_started = false;
        LOG(LOG_INFO, "User session inactivity : timer is stopped !");
    }
}

void SessionInactivity::restart_timer(std::chrono::seconds timeout, time_t start)
{
    stop_timer();
    start_timer(timeout, start);
}
