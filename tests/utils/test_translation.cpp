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
    Inifile ini;
    ini.translation.language.set_from_cstr("en");
    ini.translation.help_message.set_from_cstr("");

    BOOST_CHECK_EQUAL(std::string(TR("login", ini)),             std::string("Login"));
    BOOST_CHECK_EQUAL(std::string(TR("password", ini)),          std::string("Password"));
    BOOST_CHECK_EQUAL(std::string(TR("diagnostic", ini)),        std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(std::string(TR("connection_closed", ini)), std::string("Connection closed"));
    BOOST_CHECK_EQUAL(std::string(TR("OK", ini)),                std::string("OK"));
    BOOST_CHECK_EQUAL(std::string(TR("cancel", ini)),            std::string("Cancel"));
    BOOST_CHECK_EQUAL(std::string(TR("help", ini)),              std::string("Help"));
    BOOST_CHECK_EQUAL(std::string(TR("close", ini)),             std::string("Close"));
    BOOST_CHECK_EQUAL(std::string(TR("refused", ini)),           std::string("Refused"));
    BOOST_CHECK_EQUAL(std::string(TR("username", ini)),          std::string("Username"));
    BOOST_CHECK_EQUAL(std::string(TR("password_expire", ini)),   std::string("Your password will expire soon. Please change it."));
    BOOST_CHECK_EQUAL(std::string(TR("protocol", ini)),          std::string("Protocol"));
    BOOST_CHECK_EQUAL(std::string(TR("target_group", ini)),      std::string("Target Group"));
    BOOST_CHECK_EQUAL(std::string(TR("target", ini)),            std::string("Target"));
    BOOST_CHECK_EQUAL(std::string(TR("close_time", ini)),        std::string("Close Time"));
    BOOST_CHECK_EQUAL(std::string(TR("logout", ini)),            std::string("Logout"));
    BOOST_CHECK_EQUAL(std::string(TR("apply", ini)),             std::string("Apply"));
    BOOST_CHECK_EQUAL(std::string(TR("connect", ini)),           std::string("Connect"));
    BOOST_CHECK_EQUAL(std::string(TR("timeleft", ini)),          std::string("Time left"));
    BOOST_CHECK_EQUAL(std::string(TR("second", ini)),            std::string("second"));
    BOOST_CHECK_EQUAL(std::string(TR("minute", ini)),            std::string("minute"));
    BOOST_CHECK_EQUAL(std::string(TR("before_closing", ini)),    std::string("before closing"));
    BOOST_CHECK_EQUAL(std::string(TR("manager_close_cnx", ini)),
                      std::string("Connection closed by manager"));
    BOOST_CHECK_EQUAL(std::string(TR("end_connection", ini)),
                      std::string("End of connection"));
    BOOST_CHECK_EQUAL(std::string(TR("help_message", ini)),
                      std::string(
                                  "In login edit box, enter:<br>"
                                  "- target device and login as login@target.<br>"
                                  "- or a valid authentication user.<br>"
                                  "<br>"
                                  "In password edit box enter your password<br>"
                                  "for user.<br>"
                                  "<br>"
                                  "Both fields are case sensitive.<br>"
                                  "<br>"
                                  "Contact your system administrator if you are<br>"
                                  "experiencing problems."));

    BOOST_CHECK_EQUAL(std::string(TR("unknown text made for tests", ini)),  std::string("unknown text made for tests"));

    Translation::getInstance().set_lang(FR);
    ini.translation.language.set_from_cstr("fr");
    ini.translation.login.set_from_cstr("");
    ini.translation.password.set_from_cstr("");
    ini.translation.button_cancel.set_from_cstr("");
    ini.translation.connection_closed.set_from_cstr("");
    ini.translation.button_help.set_from_cstr("");
    ini.translation.button_close.set_from_cstr("");
    ini.translation.button_refused.set_from_cstr("");
    ini.translation.help_message.set_from_cstr("");
    ini.translation.username.set_from_cstr("");
    ini.translation.target.set_from_cstr("");
    ini.translation.manager_close_cnx.set_from_cstr("");

    BOOST_CHECK_EQUAL(std::string(TR("login", ini)),              std::string("Identifiant"));
    BOOST_CHECK_EQUAL(std::string(TR("password", ini)),           std::string("Mot de passe"));
    BOOST_CHECK_EQUAL(std::string(TR("diagnostic", ini)),         std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(std::string(TR("connection_closed", ini)),  std::string("Connexion fermée"));
    BOOST_CHECK_EQUAL(std::string(TR("OK", ini)),                 std::string("OK"));
    BOOST_CHECK_EQUAL(std::string(TR("cancel", ini)),             std::string("Annuler"));
    BOOST_CHECK_EQUAL(std::string(TR("help", ini)),               std::string("Aide"));
    BOOST_CHECK_EQUAL(std::string(TR("close", ini)),              std::string("Fermer"));
    BOOST_CHECK_EQUAL(std::string(TR("refused", ini)),            std::string("Refuser"));
    BOOST_CHECK_EQUAL(std::string(TR("username", ini)),           std::string("Utilisateur"));
    BOOST_CHECK_EQUAL(std::string(TR("password_expire", ini)),    std::string("Votre mot de passe va bientôt expirer. Veuillez le changer"));
    BOOST_CHECK_EQUAL(std::string(TR("protocol", ini)),           std::string("Protocole"));
    BOOST_CHECK_EQUAL(std::string(TR("target_group", ini)),       std::string("Groupe"));
    BOOST_CHECK_EQUAL(std::string(TR("target", ini)),             std::string("Cible"));
    BOOST_CHECK_EQUAL(std::string(TR("close_time", ini)),         std::string("Date de clôture"));
    BOOST_CHECK_EQUAL(std::string(TR("logout", ini)),             std::string("Déconnexion"));
    BOOST_CHECK_EQUAL(std::string(TR("apply", ini)),              std::string("Appliquer"));
    BOOST_CHECK_EQUAL(std::string(TR("connect", ini)),            std::string("Connecter"));
    BOOST_CHECK_EQUAL(std::string(TR("timeleft", ini)),           std::string("Temps restant"));
    BOOST_CHECK_EQUAL(std::string(TR("second", ini)),             std::string("seconde"));
    BOOST_CHECK_EQUAL(std::string(TR("minute", ini)),             std::string("minute"));
    BOOST_CHECK_EQUAL(std::string(TR("before_closing", ini)),     std::string("avant fermeture"));
    BOOST_CHECK_EQUAL(std::string(TR("manager_close_cnx", ini)),
                      std::string("Le gestionnaire de session a coupé la connexion"));
    BOOST_CHECK_EQUAL(std::string(TR("end_connection", ini)),
                      std::string("Fin de connexion"));
    BOOST_CHECK_EQUAL(std::string(TR("help_message", ini)),
                      std::string("Dans la zone de saisie login, entrez:<br>"
                                  "- le nom de la machine cible et du compte<br>"
                                  "  sous la forme login@serveur.<br>"
                                  "- ou un nom de compte valide.<br>"
                                  "<br>"
                                  "Dans la zone de saisie mot de passe,<br>"
                                  "entrez le mot de passe du compte.<br>"
                                  "<br>"
                                  "Les deux champs sont sensibles à la case.<br>"
                                  "<br>"
                                  "Contactez votre administrateur système en<br>"
                                  "cas de problème pour vous connecter."));


    BOOST_CHECK_EQUAL(std::string(TR("texte inconnu fait pour les tests", ini)),  std::string("texte inconnu fait pour les tests"));

    TRANSLATIONCONF.set_lang((language_t)10000);
    // Unknown language does not change current language.

    BOOST_CHECK_EQUAL(std::string(TR("close", ini)), std::string("Fermer"));


}
