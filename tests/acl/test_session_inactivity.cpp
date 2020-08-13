#include <chrono>

#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "acl/session_inactivity.hpp"

RED_AUTO_TEST_CASE(TestSessionInactivity_timer_not_started)
{
    time_t now = 0;
    bool has_user_activity = false;
    
    /* For start timeout, SessionInactivity::start_timer() 
       must be explicitly called after */
    SessionInactivity inactivity;

    /* On SessionInactivity constructor, 
       SessionInactivity::_inactivity_timeout and
       SessionInactivity::_last_activity_time must be 0 */
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 0);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    
    /* No timeout without to call SessionInactivity::start_timer()
       (SessionInactivity::_timer_started is false) */
    RED_CHECK(inactivity.activity(now, has_user_activity));

    /* SessionInactivity::_inactivity_timeout and
       SessionInactivity::_last_activity_time must be 0 
       if SessionInactivity::start_timer() has been never called
       (SessionInactivity::_last_activity_time can be changed into
       activity() call) */
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 0);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_disable_timeout)
{
    std::chrono::seconds timeout = 0s;
    time_t start = 0;
    time_t now = 60;
    bool has_user_activity = false;
    SessionInactivity inactivity;
    
    /* SessionInactivity::_inactivity_timeout 
       will be set to unlimited here because
       we pass 0s for "timeout" variable */
    inactivity.start_timer(timeout, start);
    
    RED_CHECK(inactivity.activity(now, has_user_activity));

    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 0);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);    
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_disable_timeout_because_timeout_too_high)
{
    std::chrono::seconds timeout = 200h;
    time_t start = 0;
    time_t now = std::chrono::seconds(2000h).count();
    bool has_user_activity = false;
    SessionInactivity inactivity;
    
    /* SessionInactivity::_inactivity_timeout will be set 
       to unlimited here because we pass 200h for "timeout variable".
       Timeout cannot exceeds 
       SessionInactivity::ACCEPTED_TIMEOUT_MAX (= 168h) */
    inactivity.start_timer(timeout, start);
    
    RED_CHECK(inactivity.activity(now, has_user_activity));

    /* SessionInactivity::_inactivity_timeout
       must be equal to 0 (unlimited timeout)
       because of start_timer() call */
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 0);
    
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_timeout_lower_than_minimum_then_setting_it_to_minimum)
{
    std::chrono::seconds timeout = 10s;
    time_t start = 0;
    time_t now = 20;
    bool has_user_activity = false;
    SessionInactivity inactivity;
    
    /* SessionInactivity::_inactivity_timeout will be set to 30s here because
       it cannot be lower than minimum allowed which is  
       SessionInactivity::ACCEPTED_TIMEOUT_MIN (= 30s) */
    inactivity.start_timer(timeout, start);

    /* No timeout because he misses 10s of inactivity */
    RED_CHECK(inactivity.activity(now, has_user_activity));

    /* SessionInactivity::_inactivity_timeout must be equal
       to 30s because of start_timer */
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);
    
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_start_timer_but_stop_timer_after)
{
    std::chrono::seconds timeout = 30s;
    time_t start = 0;
    time_t now = 60;
    bool has_user_activity = false;
    SessionInactivity inactivity;

    inactivity.start_timer(timeout, start);
    inactivity.stop_timer();

    /* No timeout because he has been stopped
       (SessionInactivity::_timer_started is false).
       SessionInactivity::start_timer() must be called 
       for re-enable timeout */ 
    RED_CHECK(inactivity.activity(now, has_user_activity));
    
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_timer_started_but_stil_user_activity)
{
    std::chrono::seconds timeout = 30s;
    time_t start = 0;
    time_t now = 60;
    bool has_user_activity = true;
    SessionInactivity inactivity;

    inactivity.start_timer(timeout, start);

    /* No timeout because user is still 
       in activity (has_user_activity is true) */
    RED_CHECK(inactivity.activity(now, has_user_activity));

    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);

    /* SessionInactivity::_last_activity_time is equal to "now" value 
       because of start_timer() call with "has_user_activity" = true */
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), now);
    
    /* has_user_activity must be false after
       SessionInactivity::activity() call */
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_timer_started_but_still_no_timeout)
{
    std::chrono::seconds timeout = 30s;
    time_t start = 0;
    time_t now = 15;
    bool has_user_activity = false;
    SessionInactivity inactivity;

    inactivity.start_timer(timeout, start);

    /* No timeout because he misses 15s of inactivity */
    RED_CHECK(inactivity.activity(now, has_user_activity));
    
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), 0);
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_timer_started_and_timeout)
{
    std::chrono::seconds timeout = 30s;
    time_t start = 0;
    time_t now = 35;
    bool has_user_activity = false;
    SessionInactivity inactivity;

    inactivity.start_timer(timeout, start);

    /* Timeout here because user is in inactivity since 35s */
    RED_CHECK(!inactivity.activity(now, has_user_activity));
    
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);

    /* SessionInactivity::_last_activity_time equal 
       also to "now" value when
       timeout happens */
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), now);
    
    RED_CHECK(!has_user_activity);
}

RED_AUTO_TEST_CASE(TestSessionInactivity_timer_started_and_timeout_after_last_activity_time_update)
{
    std::chrono::seconds timeout = 30s;
    time_t start = 0;
    time_t now = 15;
    time_t now2 = 60;
    bool has_user_activity = true;
    SessionInactivity inactivity;

    inactivity.start_timer(timeout, start);

    /* No timeout here because he misses 15s of inactivity 
       and user is still in activity */
    RED_CHECK(inactivity.activity(now, has_user_activity));
    
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), now);
    
    RED_CHECK(!has_user_activity);

    /* Timeout here because the first call has updated
       SessionInactivity::_last_activity_time which is equal to "now" (= 15s).
       Then, on the activity() call below, 
       the timeout happens because user is in inactivity since 15s :
       now2 > last_activity_time + inactivity_timeout
        = 60s > 15s + 30s
        = 60s > 45s
        = True */
    RED_CHECK(!inactivity.activity(now2, has_user_activity));
    
    RED_CHECK_EQUAL(inactivity.get_inactivity_timeout().count(), 30);
    RED_CHECK_EQUAL(inactivity.get_last_activity_time(), now2);
    
    RED_CHECK(!has_user_activity);
}
