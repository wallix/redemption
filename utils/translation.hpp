/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Meng Tan
 */

#ifndef REDEMPTION_TRANSLATION_HPP
#define REDEMPTION_TRANSLATION_HPP

#include <map>
#include "config.hpp"

typedef std::map <const char *, const char *> trans_t;
typedef enum {
    EN,
    FR,
    MAX_LANG
} language_t;

class Translation {
private:
    language_t lang;
    trans_t en_map;
    trans_t fr_map;

    void build_fr_map() {
        fr_map["login"] = "Identifiant";
        fr_map["password"] = "Mot de passe";
        fr_map["diagnostic"] = "Diagnostic";
        fr_map["connection_closed"] = "Connexion fermée";
        fr_map["OK"] = "OK";
        fr_map["cancel"] = "Annuler";
        fr_map["help"] = "Aide";
        fr_map["close"] = "Fermer";
        fr_map["refused"] = "Refuser";
        fr_map["username"] = "Utilisateur";
        fr_map["password_expire"] = "Votre mot de passe va bientôt expirer. Veuillez le changer";
        fr_map["protocol"] = "Protocole";
        fr_map["target_group"] = "Groupe";
        fr_map["target"] = "Cible";
        fr_map["close_time"] = "Date de clôture";
        fr_map["logout"] = "Déconnexion";
        fr_map["apply"] = "Appliquer";
        fr_map["filter"] = "Filtrer";
        fr_map["connect"] = "Connecter";
        fr_map["timeleft"] = "Temps restant";
        fr_map["second"] = "seconde";
        fr_map["minute"] = "minute";
        fr_map["before_closing"] = "avant fermeture";
        fr_map["manager_close_cnx"] = "Le gestionnaire de session a coupé la connexion";
        fr_map["end_connection"] = "Fin de connexion";
        fr_map["help_message"] =
            "Dans la zone de saisie login, entrez:<br>"
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
            "cas de problème pour vous connecter.";
        fr_map["selector"] = "Sélecteur";
        fr_map["session_out_time"] = "L'authorisation de la session a expirée";
        fr_map["miss_keepalive"] = "Absence de réponse de Keepalive de l'ACL";
        fr_map["close_inactivity"] = "Fermeture sur inactivité";
        fr_map["acl_fail"] = "Echec du service d'authentification";
        fr_map["comment"] = "Commentaire";
        fr_map["no_results"] = "Aucun résultat";
    }
    void build_en_map() {
        en_map["login"] = "Login";
        en_map["password"] = "Password";
        en_map["diagnostic"] = "Diagnostic";
        en_map["connection_closed"] = "Connection closed";
        en_map["OK"] = "OK";
        en_map["cancel"] = "Cancel";
        en_map["help"] = "Help";
        en_map["close"] = "Close";
        en_map["refused"] = "Refused";
        en_map["username"] = "Username";
        en_map["password_expire"] = "Your password will expire soon. Please change it.";
        en_map["protocol"] = "Protocol";
        en_map["target_group"] = "Target Group";
        en_map["target"] = "Target";
        en_map["close_time"] = "Close Time";
        en_map["logout"] = "Logout";
        en_map["apply"] = "Apply";
        en_map["filter"] = "Filter";
        en_map["connect"] = "Connect";
        en_map["timeleft"] = "Time left";
        en_map["second"] = "second";
        en_map["minute"] = "minute";
        en_map["before_closing"] = "before closing";
        en_map["manager_close_cnx"] = "Connection closed by manager";
        en_map["end_connection"] = "End of connection";
        en_map["help_message"] =
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
            "experiencing problems.";
        en_map["selector"] = "Selector";
        en_map["session_out_time"] = "Session is out of allowed timeframe";
        en_map["miss_keepalive"] = "Missed keepalive from ACL";
        en_map["close_inactivity"] = "Connection closed on inactivity";
        en_map["acl_fail"] = "Authentifier service failed";
        en_map["comment"] = "Comment";
        en_map["no_results"] = "No results found";
    }

    Translation()
        : lang(EN)
    {
        this->build_fr_map();
        this->build_en_map();
    }
    Translation(Translation const&);
    void operator=(Translation const&);

    ~Translation()
    {}
public:
    static Translation& getInstance() {
        static Translation instance;
        return instance;
    }

    bool set_lang(language_t lang) {
        if (lang >= MAX_LANG)
            return false;
        this->lang = lang;
        return true;
    }

    trans_t & getmap() {
        switch(this->lang) {
        case EN:
            return this->en_map;
            break;
        case FR:
            return this->fr_map;
            break;
        default:
            break;
        }
        return this->en_map;
    }

};

#define TRANSLATIONCONF Translation::getInstance()
static inline const char * TR(const char * key, Inifile & ini) {

    const char * res = NULL;

    if (0 == strcmp(key, "OK")) {
        res = ini.translation.button_ok.get_cstr();
    }
    else if (0 == strcmp(key, "cancel")) {
        res = ini.translation.button_cancel.get_cstr();
    }
    else if (0 == strcmp(key, "help")) {
        res = ini.translation.button_help.get_cstr();
    }
    else if (0 == strcmp(key, "close")) {
        res = ini.translation.button_close.get_cstr();
    }
    else if (0 == strcmp(key, "refused")) {
        res = ini.translation.button_refused.get_cstr();
    }
    else if (0 == strcmp(key, "login")) {
        res = ini.translation.login.get_cstr();
    }
    else if (0 == strcmp(key, "username")) {
        res = ini.translation.username.get_cstr();
    }
    else if (0 == strcmp(key, "password")) {
        res = ini.translation.password.get_cstr();
    }
    else if (0 == strcmp(key, "target")) {
        res = ini.translation.target.get_cstr();
    }
    else if (0 == strcmp(key, "diagnostic")) {
        res = ini.translation.diagnostic.get_cstr();
    }
    else if (0 == strcmp(key, "connection_closed")) {
        res = ini.translation.connection_closed.get_cstr();
    }
    else if (0 == strcmp(key, "help_message")) {
        res = ini.translation.help_message.get_cstr();
    }
    else if (0 == strcmp(key, "manager_close_cnx")) {
        res = ini.translation.manager_close_cnx.get_cstr();
    }
    const char * lang = ini.translation.language.get_cstr();
    if (0 == strcmp("fr", lang)) {
        TRANSLATIONCONF.set_lang(FR);
    }
    else if (0 == strcmp("en", lang)) {
        TRANSLATIONCONF.set_lang(EN);
    }

    if ((res == NULL) ||
        0 == strcmp(res, "") ||
        0 == strcmp(res, "ASK")) {

        trans_t trans = TRANSLATIONCONF.getmap();
        try {
            res = trans.at(key);
        }
        catch (const std::out_of_range & oor) {
            LOG(LOG_INFO, "Translation not found for '%s'", key);
            res = key;
        }
    }
    return res;
};

#endif
