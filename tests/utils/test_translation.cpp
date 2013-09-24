/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Meng Tan

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestTranslation
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
#include "log.hpp"

#include "translation.hpp"
#include <string>

BOOST_AUTO_TEST_CASE(TestTranslation)
{
    BOOST_CHECK_EQUAL(std::string(TR("login")),
                      std::string("Login"));
    BOOST_CHECK_EQUAL(std::string(TR("password")),
                      std::string("Password"));
    BOOST_CHECK_EQUAL(std::string(TR("diagnostic")),
                      std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(std::string(TR("connection_closed")),
                      std::string("Connection closed"));
    BOOST_CHECK_EQUAL(std::string(TR("OK")),
                      std::string("OK"));
    BOOST_CHECK_EQUAL(std::string(TR("cancel")),
                      std::string("Cancel"));
    BOOST_CHECK_EQUAL(std::string(TR("help")),
                      std::string("Help"));
    BOOST_CHECK_EQUAL(std::string(TR("close")),
                      std::string("Close"));
    BOOST_CHECK_EQUAL(std::string(TR("refused")),
                      std::string("Refused"));
    BOOST_CHECK_EQUAL(std::string(TR("username")),
                      std::string("Username"));
    BOOST_CHECK_EQUAL(std::string(TR("password_expire")),
                      std::string("Your password will expire soon. Please change it."));
    BOOST_CHECK_EQUAL(std::string(TR("protocol")),
                      std::string("Protocol"));
    BOOST_CHECK_EQUAL(std::string(TR("target_group")),
                      std::string("Target Group"));
    BOOST_CHECK_EQUAL(std::string(TR("target")),
                      std::string("Target"));
    BOOST_CHECK_EQUAL(std::string(TR("close_time")),
                      std::string("Close Time"));
    BOOST_CHECK_EQUAL(std::string(TR("logout")),
                      std::string("Logout"));
    BOOST_CHECK_EQUAL(std::string(TR("apply")),
                      std::string("Apply"));
    BOOST_CHECK_EQUAL(std::string(TR("connect")),
                      std::string("Connect"));
    BOOST_CHECK_EQUAL(std::string(TR("timeleft")),
                      std::string("Timeleft"));
    BOOST_CHECK_EQUAL(std::string(TR("second")),
                      std::string("second"));
    BOOST_CHECK_EQUAL(std::string(TR("minute")),
                      std::string("minute"));
    BOOST_CHECK_EQUAL(std::string(TR("before_closing")),
                      std::string("before closing"));
    BOOST_CHECK_EQUAL(std::string(TR("help_message")),
                      std::string("You must be authenticated before using this<br>"
                                  "session.<br>"
                                  "<br>"
                                  "Enter a valid username in the username edit box.<br>"
                                  "Enter the password in the password edit box.<br>"
                                  "<br>"
                                  "Both the username and password are case<br>"
                                  "sensitive.<br>"
                                  "<br>"
                                  "Contact your system administrator if you are<br>"
                                  "having problems logging on."));

    BOOST_CHECK_EQUAL(std::string(TR("unknown text made for tests")),
                      std::string("unknown text made for tests"));

    Translation::getInstance().set_lang(FR);

    BOOST_CHECK_EQUAL(std::string(TR("login")),
                      std::string("Identifiant"));
    BOOST_CHECK_EQUAL(std::string(TR("password")),
                      std::string("Mot de passe"));
    BOOST_CHECK_EQUAL(std::string(TR("diagnostic")),
                      std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(std::string(TR("connection_closed")),
                      std::string("Connexion fermée"));
    BOOST_CHECK_EQUAL(std::string(TR("OK")),
                      std::string("OK"));
    BOOST_CHECK_EQUAL(std::string(TR("cancel")),
                      std::string("Annuler"));
    BOOST_CHECK_EQUAL(std::string(TR("help")),
                      std::string("Aide"));
    BOOST_CHECK_EQUAL(std::string(TR("close")),
                      std::string("Fermer"));
    BOOST_CHECK_EQUAL(std::string(TR("refused")),
                      std::string("Refuser"));
    BOOST_CHECK_EQUAL(std::string(TR("username")),
                      std::string("Utilisateur"));
    BOOST_CHECK_EQUAL(std::string(TR("password_expire")),
                      std::string("Votre mot de passe va bientôt expirer. Veuillez le changer"));
    BOOST_CHECK_EQUAL(std::string(TR("protocol")),
                      std::string("Protocole"));
    BOOST_CHECK_EQUAL(std::string(TR("target_group")),
                      std::string("Groupe"));
    BOOST_CHECK_EQUAL(std::string(TR("target")),
                      std::string("Cible"));
    BOOST_CHECK_EQUAL(std::string(TR("close_time")),
                      std::string("Date de clôture"));
    BOOST_CHECK_EQUAL(std::string(TR("logout")),
                      std::string("Déconnexion"));
    BOOST_CHECK_EQUAL(std::string(TR("apply")),
                      std::string("Appliquer"));
    BOOST_CHECK_EQUAL(std::string(TR("connect")),
                      std::string("Connecter"));
    BOOST_CHECK_EQUAL(std::string(TR("timeleft")),
                      std::string("Temps restant"));
    BOOST_CHECK_EQUAL(std::string(TR("second")),
                      std::string("seconde"));
    BOOST_CHECK_EQUAL(std::string(TR("minute")),
                      std::string("minute"));
    BOOST_CHECK_EQUAL(std::string(TR("before_closing")),
                      std::string("avant fermeture"));
    BOOST_CHECK_EQUAL(std::string(TR("help_message")),
                      std::string("Vous devez vous authentifier avant d'utiliser<br>"
                                  "cette session.<br>"
                                  "<br>"
                                  "Entrez un nom de compte valide dans la zone<br>"
                                  "de saisie Identifiant.<br>"
                                  "Entrez le mot de passe dans la zone de saisie <br>"
                                  "Mot de passe.<br>"
                                  "<br>"
                                  "Le nom de l'utilisateur et le mot de passe sont<br>"
                                  "sensible à la case.<br>"
                                  "<br>"
                                  "Contactez votre administrateur système en cas<br>"
                                  "de problème pour vous connecter."));


    BOOST_CHECK_EQUAL(std::string(TR("texte inconnu fait pour les tests")),
                      std::string("texte inconnu fait pour les tests"));

    TRANSLATIONCONF.set_lang((language_t)10000);
    // Unknown language does not change current language.

    BOOST_CHECK_EQUAL(std::string(TR("close")),
                      std::string("Fermer"));


}
