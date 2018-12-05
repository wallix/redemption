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

#define RED_TEST_MODULE TestTranslation
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "configs/config.hpp"
#include "utils/translation.hpp"

RED_AUTO_TEST_CASE(TestTranslation)
{
    Translation::language_t lang = Translation::EN;

    RED_CHECK_EQUAL(TR(trkeys::login, lang),             "Login");
    RED_CHECK_EQUAL(TR(trkeys::password, lang),          "Password");
    RED_CHECK_EQUAL(TR(trkeys::diagnostic, lang),        "Diagnostic");
    RED_CHECK_EQUAL(TR(trkeys::connection_closed, lang), "Connection closed");
    RED_CHECK_EQUAL(TR(trkeys::OK, lang),                "OK");
    RED_CHECK_EQUAL(TR(trkeys::cancel, lang),            "Cancel");
    RED_CHECK_EQUAL(TR(trkeys::help, lang),              "Help");
    RED_CHECK_EQUAL(TR(trkeys::close, lang),             "Close");
    RED_CHECK_EQUAL(TR(trkeys::refused, lang),           "Refused");
    RED_CHECK_EQUAL(TR(trkeys::username, lang),          "Username");
    RED_CHECK_EQUAL(TR(trkeys::password_expire, lang),   "Your Bastion password will expire soon. Please change it.");
    RED_CHECK_EQUAL(TR(trkeys::protocol, lang),          "Protocol");
    RED_CHECK_EQUAL(TR(trkeys::target, lang),            "Target");
    RED_CHECK_EQUAL(TR(trkeys::close_time, lang),        "Close Time");
    RED_CHECK_EQUAL(TR(trkeys::logout, lang),            "Logout");
    RED_CHECK_EQUAL(TR(trkeys::apply, lang),             "Apply");
    RED_CHECK_EQUAL(TR(trkeys::connect, lang),           "Connect");
    RED_CHECK_EQUAL(TR(trkeys::timeleft, lang),          "Time left");
    RED_CHECK_EQUAL(TR(trkeys::second, lang),            "second");
    RED_CHECK_EQUAL(TR(trkeys::minute, lang),            "minute");
    RED_CHECK_EQUAL(TR(trkeys::before_closing, lang),    "before closing");
    RED_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang), "Connection closed by manager.");
    RED_CHECK_EQUAL(TR(trkeys::end_connection, lang),    "End of connection");
    RED_CHECK_EQUAL(TR(trkeys::help_message, lang),
                                  "In login edit box, enter:<br>"
                                  "- target device and login with the Bastion login separated<br>"
                                  "  by a semi colon as login@target:service:Bastionlogin<br>"
                                  "- or a valid Bastion authentication user.<br>"
                                  "<br>"
                                  "In password edit box enter your password<br>"
                                  "for user.<br>"
                                  "<br>"
                                  "Both fields are case sensitive.<br>"
                                  "<br>"
                                  "Contact your system administrator if you are<br>"
                                  "experiencing problems."
                                  );
    RED_CHECK_EQUAL(TR(trkeys::selector, lang),           "Selector");
    RED_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   "Session is out of allowed timeframe");
    RED_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     "Missed keepalive from ACL");
    RED_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   "Connection closed on inactivity");
    RED_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           "Authentifier service failed");

    Translation::getInstance().set_lang(Translation::FR);
    lang = Translation::FR;

    RED_CHECK_EQUAL(TR(trkeys::login, lang),              "Login");
    RED_CHECK_EQUAL(TR(trkeys::password, lang),           "Mot de passe");
    RED_CHECK_EQUAL(TR(trkeys::diagnostic, lang),         "Diagnostic");
    RED_CHECK_EQUAL(TR(trkeys::connection_closed, lang),  "Connexion fermée");
    RED_CHECK_EQUAL(TR(trkeys::OK, lang),                 "OK");
    RED_CHECK_EQUAL(TR(trkeys::cancel, lang),             "Annuler");
    RED_CHECK_EQUAL(TR(trkeys::help, lang),               "Aide");
    RED_CHECK_EQUAL(TR(trkeys::close, lang),              "Fermer");
    RED_CHECK_EQUAL(TR(trkeys::refused, lang),            "Refuser");
    RED_CHECK_EQUAL(TR(trkeys::username, lang),           "Nom d'utilisateur");
    RED_CHECK_EQUAL(TR(trkeys::password_expire, lang),    "Votre mot de passe Bastion va bientôt expirer. Veuillez le changer.");
    RED_CHECK_EQUAL(TR(trkeys::protocol, lang),           "Protocole");
    RED_CHECK_EQUAL(TR(trkeys::target, lang),             "Cible");
    RED_CHECK_EQUAL(TR(trkeys::close_time, lang),         "Date de clôture");
    RED_CHECK_EQUAL(TR(trkeys::logout, lang),             "Déconnexion");
    RED_CHECK_EQUAL(TR(trkeys::apply, lang),              "Appliquer");
    RED_CHECK_EQUAL(TR(trkeys::connect, lang),            "Connecter");
    RED_CHECK_EQUAL(TR(trkeys::timeleft, lang),           "Temps restant");
    RED_CHECK_EQUAL(TR(trkeys::second, lang),             "seconde");
    RED_CHECK_EQUAL(TR(trkeys::minute, lang),             "minute");
    RED_CHECK_EQUAL(TR(trkeys::before_closing, lang),     "avant fermeture");
    RED_CHECK_EQUAL(TR(trkeys::manager_close_cnx, lang),  "Le gestionnaire de session a coupé la connexion.");
    RED_CHECK_EQUAL(TR(trkeys::end_connection, lang),     "Fin de connexion");
    RED_CHECK_EQUAL(TR(trkeys::help_message, lang),
                                  "Dans la zone de saisie login, entrez:<br>"
                                  "- le login sur la cible et le nom de la machine cible suivi<br>"
                                  "  du compte Bastion sous la forme login@serveur:service:Bastionlogin<br>"
                                  "- ou un nom de compte Bastion valide.<br>"
                                  "<br>"
                                  "Dans la zone de saisie mot de passe,<br>"
                                  "entrez le mot de passe du compte.<br>"
                                  "<br>"
                                  "Les deux champs sont sensibles à la casse.<br>"
                                  "<br>"
                                  "Contactez votre administrateur système en<br>"
                                  "cas de problème pour vous connecter."
                                  );
    RED_CHECK_EQUAL(TR(trkeys::selector, lang),           "Sélecteur");
    RED_CHECK_EQUAL(TR(trkeys::session_out_time, lang),   "L'autorisation de la session a expiré");
    RED_CHECK_EQUAL(TR(trkeys::miss_keepalive, lang),     "Absence de réponse de Keepalive de l'ACL");
    RED_CHECK_EQUAL(TR(trkeys::close_inactivity, lang),   "Fermeture sur inactivité");
    RED_CHECK_EQUAL(TR(trkeys::acl_fail, lang),           "Echec du service d'authentification");

    RED_CHECK_EQUAL(TR(trkeys::close, lang),      "Fermer");
    RED_CHECK_EQUAL(TR(trkeys::password, lang),   "Mot de passe");

    Inifile ini;
    TRANSLATIONCONF.set_ini(&ini);
    ini.set<cfg::translation::password_en>("pouloup");
    RED_CHECK_EQUAL(TR(trkeys::password, lang), "Mot de passe");
    ini.set<cfg::translation::password_fr>("pompidou");
    RED_CHECK_EQUAL(TR(trkeys::password, lang), "pompidou");
    lang = Translation::EN;
    RED_CHECK_EQUAL(TR(trkeys::password, lang), "pouloup");

    // inhibit scan-build warn
    TRANSLATIONCONF.set_ini(nullptr);
}
