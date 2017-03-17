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
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/translation.hpp"
#include <string>

BOOST_AUTO_TEST_CASE(TestTranslation)
{
    Translation::language_t lang = Translation::EN;

    BOOST_CHECK_EQUAL(TR(trkeys::login, lang),             std::string("Login"));
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang),          std::string("Password"));
    BOOST_CHECK_EQUAL(TR(trkeys::diagnostic, lang),        std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(TR(trkeys::connection_closed, lang), std::string("Connection closed"));
    BOOST_CHECK_EQUAL(TR(trkeys::OK, lang),                std::string("OK"));
    BOOST_CHECK_EQUAL(TR(trkeys::cancel, lang),            std::string("Cancel"));
    BOOST_CHECK_EQUAL(TR(trkeys::help, lang),              std::string("Help"));
    BOOST_CHECK_EQUAL(TR(trkeys::close, lang),             std::string("Close"));
    BOOST_CHECK_EQUAL(TR(trkeys::refused, lang),           std::string("Refused"));
    BOOST_CHECK_EQUAL(TR(trkeys::username, lang),          std::string("Username"));
    BOOST_CHECK_EQUAL(TR(trkeys::password_expire, lang),   std::string("Your password will expire soon. Please change it."));
    BOOST_CHECK_EQUAL(TR(trkeys::protocol, lang),          std::string("Protocol"));
    BOOST_CHECK_EQUAL(TR(trkeys::target_group, lang),      std::string("Target Group"));
    BOOST_CHECK_EQUAL(TR(trkeys::target, lang),            std::string("Target"));
    BOOST_CHECK_EQUAL(TR(trkeys::close_time, lang),        std::string("Close Time"));
    BOOST_CHECK_EQUAL(TR(trkeys::logout, lang),            std::string("Logout"));
    BOOST_CHECK_EQUAL(TR(trkeys::apply, lang),             std::string("Apply"));
    BOOST_CHECK_EQUAL(TR(trkeys::connect, lang),           std::string("Connect"));
    BOOST_CHECK_EQUAL(TR(trkeys::timeleft, lang),          std::string("Time left"));
    BOOST_CHECK_EQUAL(TR(trkeys::second, lang),            std::string("second"));
    BOOST_CHECK_EQUAL(TR(trkeys::minute, lang),            std::string("minute"));
    BOOST_CHECK_EQUAL(TR(trkeys::before_closing, lang),    std::string("before closing"));
    BOOST_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang), std::string("Connection closed by manager"));
    BOOST_CHECK_EQUAL(TR(trkeys::end_connection, lang),    std::string("End of connection"));
    BOOST_CHECK_EQUAL(TR(trkeys::help_message, lang),
                      std::string(
                                  "In login edit box, enter:<br>"
                                  "- target device and login with the wab login separated<br>"
                                  "  by a semi colon as login@target:service:WABlogin<br>"
                                  "- or a valid WAB authentication user.<br>"
                                  "<br>"
                                  "In password edit box enter your password<br>"
                                  "for user.<br>"
                                  "<br>"
                                  "Both fields are case sensitive.<br>"
                                  "<br>"
                                  "Contact your system administrator if you are<br>"
                                  "experiencing problems."
                                  ));
    BOOST_CHECK_EQUAL(TR(trkeys::selector, lang),           std::string("Selector"));
    BOOST_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   std::string("Session is out of allowed timeframe"));
    BOOST_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     std::string("Missed keepalive from ACL"));
    BOOST_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   std::string("Connection closed on inactivity"));
    BOOST_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           std::string("Authentifier service failed"));

    Translation::getInstance().set_lang(Translation::FR);
    lang = Translation::FR;

    BOOST_CHECK_EQUAL(TR(trkeys::login, lang),              std::string("Login"));
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang),           std::string("Mot de passe"));
    BOOST_CHECK_EQUAL(TR(trkeys::diagnostic, lang),         std::string("Diagnostic"));
    BOOST_CHECK_EQUAL(TR(trkeys::connection_closed, lang),  std::string("Connexion fermée"));
    BOOST_CHECK_EQUAL(TR(trkeys::OK, lang),                 std::string("OK"));
    BOOST_CHECK_EQUAL(TR(trkeys::cancel, lang),             std::string("Annuler"));
    BOOST_CHECK_EQUAL(TR(trkeys::help, lang),               std::string("Aide"));
    BOOST_CHECK_EQUAL(TR(trkeys::close, lang),              std::string("Fermer"));
    BOOST_CHECK_EQUAL(TR(trkeys::refused, lang),            std::string("Refuser"));
    BOOST_CHECK_EQUAL(TR(trkeys::username, lang),           std::string("Nom d'utilisateur"));
    BOOST_CHECK_EQUAL(TR(trkeys::password_expire, lang),    std::string("Votre mot de passe va bientôt expirer. Veuillez le changer"));
    BOOST_CHECK_EQUAL(TR(trkeys::protocol, lang),           std::string("Protocole"));
    BOOST_CHECK_EQUAL(TR(trkeys::target_group, lang),       std::string("Groupe"));
    BOOST_CHECK_EQUAL(TR(trkeys::target, lang),             std::string("Cible"));
    BOOST_CHECK_EQUAL(TR(trkeys::close_time, lang),         std::string("Date de clôture"));
    BOOST_CHECK_EQUAL(TR(trkeys::logout, lang),             std::string("Déconnexion"));
    BOOST_CHECK_EQUAL(TR(trkeys::apply, lang),              std::string("Appliquer"));
    BOOST_CHECK_EQUAL(TR(trkeys::connect, lang),            std::string("Connecter"));
    BOOST_CHECK_EQUAL(TR(trkeys::timeleft, lang),           std::string("Temps restant"));
    BOOST_CHECK_EQUAL(TR(trkeys::second, lang),             std::string("seconde"));
    BOOST_CHECK_EQUAL(TR(trkeys::minute, lang),             std::string("minute"));
    BOOST_CHECK_EQUAL(TR(trkeys::before_closing, lang),     std::string("avant fermeture"));
    BOOST_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang),  std::string("Le gestionnaire de session a coupé la connexion"));
    BOOST_CHECK_EQUAL(TR(trkeys::end_connection, lang),     std::string("Fin de connexion"));
    BOOST_CHECK_EQUAL(TR(trkeys::help_message, lang),       std::string(
                                  "Dans la zone de saisie login, entrez:<br>"
                                  "- le login sur la cible et le nom de la machine cible suivi<br>"
                                  "  du compte WAB sous la forme login@serveur:service:WABlogin<br>"
                                  "- ou un nom de compte WAB valide.<br>"
                                  "<br>"
                                  "Dans la zone de saisie mot de passe,<br>"
                                  "entrez le mot de passe du compte.<br>"
                                  "<br>"
                                  "Les deux champs sont sensibles à la casse.<br>"
                                  "<br>"
                                  "Contactez votre administrateur système en<br>"
                                  "cas de problème pour vous connecter."
                                  ));
    BOOST_CHECK_EQUAL(TR(trkeys::selector, lang),           std::string("Sélecteur"));
    BOOST_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   std::string("L'autorisation de la session a expiré"));
    BOOST_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     std::string("Absence de réponse de Keepalive de l'ACL"));
    BOOST_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   std::string("Fermeture sur inactivité"));
    BOOST_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           std::string("Echec du service d'authentification"));

    BOOST_CHECK_EQUAL(TRANSLATIONCONF.set_lang(static_cast<Translation::language_t>(10000)), false);
    // Unknown language does not change current language.

    BOOST_CHECK_EQUAL(TR(trkeys::close, lang),      std::string("Fermer"));
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang),   std::string("Mot de passe"));

    Inifile ini;
    TRANSLATIONCONF.set_ini(&ini);
    ini.set<cfg::translation::password_en>("pouloup");
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang), std::string("Mot de passe"));
    ini.set<cfg::translation::password_fr>("pompidou");
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang), std::string("pompidou"));
    lang = Translation::EN;
    BOOST_CHECK_EQUAL(TR(trkeys::password, lang), std::string("pouloup"));
}
