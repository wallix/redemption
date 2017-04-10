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

#define UNIT_TEST_MODULE TestTranslation
#include "system/redemption_unit_tests.hpp"


#define LOGNULL
//#define LOGPRINT

#include "utils/translation.hpp"
#include <string>

RED_AUTO_TEST_CASE(TestTranslation)
{
    Translation::language_t lang = Translation::EN;

    RED_CHECK_EQUAL(TR(trkeys::login, lang),             std::string("Login"));
    RED_CHECK_EQUAL(TR(trkeys::password, lang),          std::string("Password"));
    RED_CHECK_EQUAL(TR(trkeys::diagnostic, lang),        std::string("Diagnostic"));
    RED_CHECK_EQUAL(TR(trkeys::connection_closed, lang), std::string("Connection closed"));
    RED_CHECK_EQUAL(TR(trkeys::OK, lang),                std::string("OK"));
    RED_CHECK_EQUAL(TR(trkeys::cancel, lang),            std::string("Cancel"));
    RED_CHECK_EQUAL(TR(trkeys::help, lang),              std::string("Help"));
    RED_CHECK_EQUAL(TR(trkeys::close, lang),             std::string("Close"));
    RED_CHECK_EQUAL(TR(trkeys::refused, lang),           std::string("Refused"));
    RED_CHECK_EQUAL(TR(trkeys::username, lang),          std::string("Username"));
    RED_CHECK_EQUAL(TR(trkeys::password_expire, lang),   std::string("Your password will expire soon. Please change it."));
    RED_CHECK_EQUAL(TR(trkeys::protocol, lang),          std::string("Protocol"));
    RED_CHECK_EQUAL(TR(trkeys::target_group, lang),      std::string("Target Group"));
    RED_CHECK_EQUAL(TR(trkeys::target, lang),            std::string("Target"));
    RED_CHECK_EQUAL(TR(trkeys::close_time, lang),        std::string("Close Time"));
    RED_CHECK_EQUAL(TR(trkeys::logout, lang),            std::string("Logout"));
    RED_CHECK_EQUAL(TR(trkeys::apply, lang),             std::string("Apply"));
    RED_CHECK_EQUAL(TR(trkeys::connect, lang),           std::string("Connect"));
    RED_CHECK_EQUAL(TR(trkeys::timeleft, lang),          std::string("Time left"));
    RED_CHECK_EQUAL(TR(trkeys::second, lang),            std::string("second"));
    RED_CHECK_EQUAL(TR(trkeys::minute, lang),            std::string("minute"));
    RED_CHECK_EQUAL(TR(trkeys::before_closing, lang),    std::string("before closing"));
    RED_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang), std::string("Connection closed by manager"));
    RED_CHECK_EQUAL(TR(trkeys::end_connection, lang),    std::string("End of connection"));
    RED_CHECK_EQUAL(TR(trkeys::help_message, lang),
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
    RED_CHECK_EQUAL(TR(trkeys::selector, lang),           std::string("Selector"));
    RED_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   std::string("Session is out of allowed timeframe"));
    RED_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     std::string("Missed keepalive from ACL"));
    RED_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   std::string("Connection closed on inactivity"));
    RED_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           std::string("Authentifier service failed"));

    Translation::getInstance().set_lang(Translation::FR);
    lang = Translation::FR;

    RED_CHECK_EQUAL(TR(trkeys::login, lang),              std::string("Login"));
    RED_CHECK_EQUAL(TR(trkeys::password, lang),           std::string("Mot de passe"));
    RED_CHECK_EQUAL(TR(trkeys::diagnostic, lang),         std::string("Diagnostic"));
    RED_CHECK_EQUAL(TR(trkeys::connection_closed, lang),  std::string("Connexion fermée"));
    RED_CHECK_EQUAL(TR(trkeys::OK, lang),                 std::string("OK"));
    RED_CHECK_EQUAL(TR(trkeys::cancel, lang),             std::string("Annuler"));
    RED_CHECK_EQUAL(TR(trkeys::help, lang),               std::string("Aide"));
    RED_CHECK_EQUAL(TR(trkeys::close, lang),              std::string("Fermer"));
    RED_CHECK_EQUAL(TR(trkeys::refused, lang),            std::string("Refuser"));
    RED_CHECK_EQUAL(TR(trkeys::username, lang),           std::string("Nom d'utilisateur"));
    RED_CHECK_EQUAL(TR(trkeys::password_expire, lang),    std::string("Votre mot de passe va bientôt expirer. Veuillez le changer"));
    RED_CHECK_EQUAL(TR(trkeys::protocol, lang),           std::string("Protocole"));
    RED_CHECK_EQUAL(TR(trkeys::target_group, lang),       std::string("Groupe"));
    RED_CHECK_EQUAL(TR(trkeys::target, lang),             std::string("Cible"));
    RED_CHECK_EQUAL(TR(trkeys::close_time, lang),         std::string("Date de clôture"));
    RED_CHECK_EQUAL(TR(trkeys::logout, lang),             std::string("Déconnexion"));
    RED_CHECK_EQUAL(TR(trkeys::apply, lang),              std::string("Appliquer"));
    RED_CHECK_EQUAL(TR(trkeys::connect, lang),            std::string("Connecter"));
    RED_CHECK_EQUAL(TR(trkeys::timeleft, lang),           std::string("Temps restant"));
    RED_CHECK_EQUAL(TR(trkeys::second, lang),             std::string("seconde"));
    RED_CHECK_EQUAL(TR(trkeys::minute, lang),             std::string("minute"));
    RED_CHECK_EQUAL(TR(trkeys::before_closing, lang),     std::string("avant fermeture"));
    RED_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang),  std::string("Le gestionnaire de session a coupé la connexion"));
    RED_CHECK_EQUAL(TR(trkeys::end_connection, lang),     std::string("Fin de connexion"));
    RED_CHECK_EQUAL(TR(trkeys::help_message, lang),       std::string(
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
    RED_CHECK_EQUAL(TR(trkeys::selector, lang),           std::string("Sélecteur"));
    RED_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   std::string("L'autorisation de la session a expiré"));
    RED_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     std::string("Absence de réponse de Keepalive de l'ACL"));
    RED_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   std::string("Fermeture sur inactivité"));
    RED_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           std::string("Echec du service d'authentification"));

    RED_CHECK_EQUAL(TRANSLATIONCONF.set_lang(static_cast<Translation::language_t>(10000)), false);
    // Unknown language does not change current language.

    RED_CHECK_EQUAL(TR(trkeys::close, lang),      std::string("Fermer"));
    RED_CHECK_EQUAL(TR(trkeys::password, lang),   std::string("Mot de passe"));

    Inifile ini;
    TRANSLATIONCONF.set_ini(&ini);
    ini.set<cfg::translation::password_en>("pouloup");
    RED_CHECK_EQUAL(TR(trkeys::password, lang), std::string("Mot de passe"));
    ini.set<cfg::translation::password_fr>("pompidou");
    RED_CHECK_EQUAL(TR(trkeys::password, lang), std::string("pompidou"));
    lang = Translation::EN;
    RED_CHECK_EQUAL(TR(trkeys::password, lang), std::string("pouloup"));
}
