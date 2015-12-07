/*

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRDPQtDrawable
#include <boost/test/auto_unit_test.hpp>
#include <QApplication>

#define LOGNULL
//#define LOGPRINT

#include "RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "RDPQtDrawable.hpp"


BOOST_AUTO_TEST_CASE(TestRDPQtDrawable)
{
    int argc(0);
    char *argv[] {"myprog"};
    QApplication app(argc, argv);
    RDPQtDrawable drawer(400, 300);
    
    Rect rect(100, 100, 100, 100);
    uint32_t color(0xFF);
    RDPOpaqueRect opaqueRect(rect, color);
    
    drawer.draw(opaqueRect, rect);
    drawer.flush();
    
    drawer.show();
    app.exec();
    
}


