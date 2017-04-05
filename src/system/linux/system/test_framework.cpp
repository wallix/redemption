#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK

// main

//#include <boost/test/impl/test_main.ipp>
//#include <boost/test/impl/cpp_main.ipp>
#include <boost/test/impl/unit_test_main.ipp>

// Common


#include <boost/version.hpp>

#if ( BOOST_VERSION / 100 % 1000 == 61 )

// boost unit test 1.61
#include <boost/test/impl/compiler_log_formatter.ipp>
#include <boost/test/impl/debug.ipp>
#include <boost/test/impl/framework.ipp>
#include <boost/test/impl/plain_report_formatter.ipp>
#include <boost/test/impl/xml_log_formatter.ipp>
#include <boost/test/impl/xml_report_formatter.ipp>
#include <boost/test/impl/results_collector.ipp>
#include <boost/test/impl/results_reporter.ipp>
#include <boost/test/impl/unit_test_log.ipp>
#include <boost/test/impl/test_tools.ipp>
#include <boost/test/impl/unit_test_monitor.ipp>
#include <boost/test/impl/unit_test_parameters.ipp>
#include <boost/test/impl/progress_monitor.ipp>
#include <boost/test/impl/execution_monitor.ipp>

#include <boost/test/impl/test_tree.ipp>
#include <boost/test/impl/decorator.ipp>

#elif BOOST_VERSION / 100 % 1000 == 55
// version 1.55, expecting other variations for other releases
#include <boost/test/impl/compiler_log_formatter.ipp>
#include <boost/test/impl/debug.ipp>
#include <boost/test/impl/framework.ipp>
#include <boost/test/impl/plain_report_formatter.ipp>
#include <boost/test/impl/xml_log_formatter.ipp>
#include <boost/test/impl/xml_report_formatter.ipp>
#include <boost/test/impl/results_collector.ipp>
#include <boost/test/impl/results_reporter.ipp>
#include <boost/test/impl/unit_test_log.ipp>
#include <boost/test/impl/test_tools.ipp>
#include <boost/test/impl/unit_test_monitor.ipp>
#include <boost/test/impl/unit_test_parameters.ipp>
#include <boost/test/impl/progress_monitor.ipp>
#include <boost/test/impl/execution_monitor.ipp>

#include <boost/test/impl/exception_safety.ipp>
#include <boost/test/impl/interaction_based.ipp>
#include <boost/test/impl/logged_expectations.ipp>
#include <boost/test/impl/unit_test_suite.ipp>

#else

// Other version of boost tests will need other headers files
#include <boost/test/impl/compiler_log_formatter.ipp>
#include <boost/test/impl/debug.ipp>
#include <boost/test/impl/framework.ipp>
#include <boost/test/impl/plain_report_formatter.ipp>
#include <boost/test/impl/xml_log_formatter.ipp>
#include <boost/test/impl/xml_report_formatter.ipp>
#include <boost/test/impl/results_collector.ipp>
#include <boost/test/impl/results_reporter.ipp>
#include <boost/test/impl/unit_test_log.ipp>
#include <boost/test/impl/test_tools.ipp>
#include <boost/test/impl/unit_test_monitor.ipp>
#include <boost/test/impl/unit_test_parameters.ipp>
#include <boost/test/impl/progress_monitor.ipp>
#include <boost/test/impl/execution_monitor.ipp>

#include <boost/test/impl/test_tree.ipp>
#include <boost/test/impl/decorator.ipp>

#endif
