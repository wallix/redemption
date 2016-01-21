/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Clément Moroldo
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRDPQt
#include <boost/test/auto_unit_test.hpp>
#undef SHARE_PATH
#define SHARE_PATH FIXTURES_PATH

#define LOGNULL
//#define LOGPRINTlibboost_unit_test
//#define TEST_CLOSE_FROM_SCREEN
#include "config.hpp"
//#include "test_transport.hpp"

//#include "rdp/rdp.hpp"
#include "../src/front/front_widget_Qt.hpp"


BOOST_AUTO_TEST_CASE(TestRDPQt)
{
    bool test_boost(false);
    
    int verbose(511);
    int argc(8);
    char *argv[] = {"-n", "QA\\administrateur", "-pwd", "S3cur3!1nux", "-ip", "10.10.46.88", "-p", "3389"}; 
    // test_rdp_Qt -n QA\\administrateur -pwd 'S3cur3!1nux' -ip 10.10.46.8 -p 3389
    
    
    
    
#ifdef TEST_CLOSE_FROM_SCREEN
    
    QApplication app(argc, argv);

    //=====================
    // test connexion init
    //=====================
    std::cout << std::endl;
    std::cout << "FRONT INIT TEST" << std::endl;
    Front_Qt frontInit(argv, argc, verbose);

    if (frontInit._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false; 
    if (frontInit._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (frontInit._callback            != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._connector->_sckRead != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._connector->_sck     != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    BOOST_CHECK_EQUAL(frontInit._callback->get_front_width(), 800);
    BOOST_CHECK_EQUAL(frontInit._callback->get_front_height(), 600);
    
    BOOST_CHECK_EQUAL(frontInit._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(frontInit._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(frontInit._targetIP, "10.10.46.88");
    BOOST_CHECK_EQUAL(frontInit._port,     3389);
    
    BOOST_CHECK_EQUAL(frontInit._connected, true);
    
    
    
    //========================
    // test close from screen
    //========================
    std::cout <<  std::endl << "Test close from screen" <<  std::endl;
    app.exec();
    
    BOOST_CHECK_EQUAL(frontInit._connected, true);
    
    if (frontInit._callback  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._connector->_callback == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._connector->_sckRead  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (frontInit._connector->_sck      == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    //************************************************************************
    
#endif
    
    
    
#ifndef TEST_CLOSE_FROM_SCREEN
    
    QApplication app2(argc, argv);
    
    //=====================
    // test connexion init
    //=====================
    std::cout << std::endl;
    std::cout << "FRONT TEST" << std::endl;
    
    Front_Qt front(argv, argc, verbose);
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback            != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck     != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    BOOST_CHECK_EQUAL(front._callback->get_front_width(), 800);
    BOOST_CHECK_EQUAL(front._callback->get_front_height(), 600);
    
    BOOST_CHECK_EQUAL(front._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._targetIP, "10.10.46.88");
    BOOST_CHECK_EQUAL(front._port,     3389);
    
    BOOST_CHECK_EQUAL(front._connected, true);
    
    
    
    //=====================
    //  test disconnexion
    //=====================
    std::cout <<  std::endl << "Test  disconnexion" <<  std::endl;
    front.disconnect("disconnected");
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    BOOST_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "disconnected");
    
    BOOST_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       "10.10.46.88");
    BOOST_CHECK_EQUAL(front._form->_portField.text().toInt(),          3389);
    
    BOOST_CHECK_EQUAL(front._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._targetIP, "10.10.46.88");
    BOOST_CHECK_EQUAL(front._port,     3389);
    
    BOOST_CHECK_EQUAL(front._connected, false);
    
    
    
    //======================
    // test connexion error
    //======================
    std::cout <<  std::endl << "Test connexion error" <<  std::endl;
    front._form->set_IPField("10.10.46.80");
    front.connexionReleased();
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    BOOST_CHECK_EQUAL(front._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._targetIP, "10.10.46.80");
    BOOST_CHECK_EQUAL(front._port,     3389);
    
    BOOST_CHECK_EQUAL(front._connected, false);
    
    BOOST_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "<font color='Red'>Cannot connect to [10.10.46.80].</font>");
    
    BOOST_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       "10.10.46.80");
    BOOST_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    

    
    //=====================
    //  test reconnexion
    //=====================
    std::cout <<  std::endl << "Test reconnexion" <<  std::endl;
    front._form->set_IPField("10.10.46.88");
    
    BOOST_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       "10.10.46.88");
    BOOST_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    
    front.connexionReleased();
    
    BOOST_CHECK_EQUAL(front._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._targetIP, "10.10.46.88");
    BOOST_CHECK_EQUAL(front._port,     3389);
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback            != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck     != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    BOOST_CHECK_EQUAL(front._callback->get_front_width(), 800);
    BOOST_CHECK_EQUAL(front._callback->get_front_height(), 600);
    
    BOOST_CHECK_EQUAL(front._connected, true);
    
    
    
    //====================
    //    test options
    //====================
    std::cout <<  std::endl << "Test options" <<  std::endl;
    front.disconnect("");
    
    BOOST_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       "10.10.46.88");
    BOOST_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    
    BOOST_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "");
    
    BOOST_CHECK_EQUAL(front._connected, false);
    
    for (int i = 0; i < 50; i++) {
        DialogOptions_Qt * dia = new DialogOptions_Qt(&front, front._form);
        dia->close();
    }
    
    
    
    //======================
    // test drop connexion
    //======================
    std::cout <<  std::endl << "Test drop connexion" <<  std::endl;
    front.connexionReleased();
    
    BOOST_CHECK_EQUAL(front._connected, true);
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback             != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    front._connector->drop_connexion();
    
    if (front._screen    != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    BOOST_CHECK_EQUAL(front._userName, "QA\\administrateur");
    BOOST_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    BOOST_CHECK_EQUAL(front._targetIP, "10.10.46.88");
    BOOST_CHECK_EQUAL(front._port,     3389);
    
    BOOST_CHECK_EQUAL(front._connected, true);
    
    BOOST_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "");
    
    
    
    //========================
    // test close from form
    //========================
    std::cout <<  std::endl << "Test close from form" <<  std::endl;
    front.connexionReleased();
    std::cout <<  std::endl << "Test close from form" <<  std::endl;
    front.disconnect("");
    app2.exec();

    BOOST_CHECK_EQUAL(front._connected, false);

    if (front._callback  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    BOOST_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    //************************************************************************
    
#endif
    
}

