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

#define UNIT_TEST_MODULE TestRDPQt
#include "system/redemption_unit_tests.hpp"
#include <unistd.h>

#define LOGNULL
//#define LOGPRINTlibboost_unit_test
#include "configs/config.hpp"
//#include "test_transport.hpp"

//#include "mod/rdp/rdp.hpp"
#include "../src/front/front_widget_Qt.hpp"


#define TARGET_IP "10.10.46.73"
//#define TARGET_IP "10.10.46.88"


//./bin/gcc-5.2.1/release/threading-multi/test_rdp_Qt -n QA\administrateur -pwd 'S3cur3!1nux' -ip 10.10.46.73 -p 3389

RED_AUTO_TEST_CASE(TestRDPQt)
{
    bool test_boost(false);
    std::string targetIP(TARGET_IP); // 10.10.46.73
    int verbose(511);
    int argc(8);
    const char *argv[] = {"-n", "QA\\administrateur", "-pwd", "S3cur3!1nux", "-ip", TARGET_IP, "-p", "3389"}; 
    // test_rdp_Qt -n QA\\administrateur -pwd 'S3cur3!1nux' -ip 10.10.46.88 -p 3389

    
    QApplication app(argc, const_cast<char**>(argv));
    
    //=====================
    // test connexion init
    //=====================
    std::cout << std::endl;
    std::cout << "FRONT TEST" << std::endl;
    
    Front_Qt front(const_cast<char**>(argv), argc, verbose);
    
    if (front._screen    != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback            != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    if (test_boost) {
        RED_CHECK_EQUAL(front._callback->get_front_width(), 800);
        RED_CHECK_EQUAL(front._callback->get_front_height(), 600);
    }
    test_boost = false;
    if (front._connector->_sckRead != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck     != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    RED_CHECK_EQUAL(front._userName, "QA\\administrateur");
    RED_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._targetIP, targetIP);
    RED_CHECK_EQUAL(front._port,     3389);
    
    RED_CHECK_EQUAL(front._connected, true);
    
    
    
    //=====================
    //  test disconnexion
    //=====================
    std::cout <<  std::endl << "Test  disconnection" <<  std::endl;
    front.disconnexionReleased();
    
    if (front._screen    == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    RED_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "");
    
    RED_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    RED_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       targetIP);
    RED_CHECK_EQUAL(front._form->_portField.text().toInt(),          3389);
    
    RED_CHECK_EQUAL(front._userName, "QA\\administrateur");
    RED_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._targetIP, targetIP);
    RED_CHECK_EQUAL(front._port,     3389);
    
    RED_CHECK_EQUAL(front._connected, false);
    
    
    
    //======================
    // test connexion error
    //======================
    std::cout <<  std::endl << "Test connection error" <<  std::endl;
    front._form->set_IPField(targetIP+"0");
    front.connexionReleased();
    
    if (front._screen    == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    RED_CHECK_EQUAL(front._userName, "QA\\administrateur");
    RED_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._targetIP, targetIP+"0");
    RED_CHECK_EQUAL(front._port,     3389);
    
    RED_CHECK_EQUAL(front._connected, false);
    
    RED_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "<font color='Red'>Cannot connect to ["+targetIP+"0].</font>");
    
    RED_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    RED_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       targetIP+"0");
    RED_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    

    
    //=====================
    //  test reconnexion
    //=====================
    std::cout <<  std::endl << "Test reconnection" <<  std::endl;
    front._form->set_IPField(targetIP);
    
    RED_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    RED_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       targetIP);
    RED_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    
    front.connexionReleased();
    
    RED_CHECK_EQUAL(front._userName, "QA\\administrateur");
    RED_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._targetIP, targetIP);
    RED_CHECK_EQUAL(front._port,     3389);
    
    if (front._screen    != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback            != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    if (test_boost) {
        RED_CHECK_EQUAL(front._callback->get_front_width(), 800);
        RED_CHECK_EQUAL(front._callback->get_front_height(), 600);
    }
    test_boost = false;
    if (front._connector->_sckRead != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck     != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    RED_CHECK_EQUAL(front._connected, true);
    
    
    
    //====================
    //    test options
    //====================
    std::cout <<  std::endl << "Test options" <<  std::endl;
    front.disconnect("");
    
    RED_CHECK_EQUAL(front._form->_userNameField.text().toStdString(), "QA\\administrateur");
    RED_CHECK_EQUAL(front._form->_PWDField.text().toStdString(),      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._form->_IPField.text().toStdString(),       targetIP);
    RED_CHECK_EQUAL(front._form->_portField.text().toInt(),           3389);
    
    RED_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "");
    
    RED_CHECK_EQUAL(front._connected, false);
    
    for (int i = 0; i < 50; i++) {
        DialogOptions_Qt * dia = new DialogOptions_Qt(&front, front._form);
        dia->close();
    }
    
    
    
    //======================
    // test drop connexion
    //======================
    std::cout <<  std::endl << "Test drop connection" <<  std::endl;
    front.connexionReleased();
    
    RED_CHECK_EQUAL(front._connected, true);
    
    if (front._screen    != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    if (front._callback             != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    front._connector->drop_connexion();
    
    if (front._screen    != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._form      != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector != nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;

    if (front._callback             == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_callback == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sckRead  == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    if (front._connector->_sck      == nullptr) { test_boost = true;}
    RED_CHECK_EQUAL(test_boost, true);
    test_boost = false;
    
    RED_CHECK_EQUAL(front._userName, "QA\\administrateur");
    RED_CHECK_EQUAL(front._pwd,      "S3cur3!1nux");
    RED_CHECK_EQUAL(front._targetIP, targetIP);
    RED_CHECK_EQUAL(front._port,     3389);
    
    RED_CHECK_EQUAL(front._connected, true);
    
    RED_CHECK_EQUAL(front._form->_errorLabel.text().toStdString(), "");
    
    
    
    //========================
    //       test show
    //========================
    std::cout <<  std::endl << "Test show window" <<  std::endl;
    front.connexionReleased();;
    front.disconnect("");
    app.exec();
    
    std::cout <<  std::endl << "Test close" <<  std::endl;
    if (front._connected) {
        
        //========================
        // test close from screen
        //========================
        
        RED_CHECK_EQUAL(front._connected, true);
        
        if (front._callback  == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_callback == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_sckRead  == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_sck      == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        
        //************************************************************************
        
    } else { 

        //========================
        // test close from form
        //========================

        RED_CHECK_EQUAL(front._connected, false);

        if (front._callback  == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_callback == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_sckRead  == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;
        if (front._connector->_sck      == nullptr) { test_boost = true;}
        RED_CHECK_EQUAL(test_boost, true);
        test_boost = false;

        //************************************************************************
    }
    
    
}

