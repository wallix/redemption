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
enum language_t {
    EN,
    FR,
    MAX_LANG
};

class Translation {
private:
    language_t lang;
    trans_t en_map;
    trans_t fr_map;

    void build_fr_map() {
        trans_t & dict = this->fr_map;
        dict["login"] = "Identifiant";
        dict["password"] = "Mot de passe";
        dict["diagnostic"] = "Diagnostic";
        dict["connection_closed"] = "Connexion fermée";
        dict["OK"] = "OK";
        dict["cancel"] = "Annuler";
        dict["help"] = "Aide";
        dict["close"] = "Fermer";
        dict["refused"] = "Refuser";
        dict["username"] = "Utilisateur";
        dict["password_expire"] = "Votre mot de passe va bientôt expirer. Veuillez le changer";
        dict["protocol"] = "Protocole";
        dict["target_group"] = "Groupe";
        dict["target"] = "Cible";
        dict["close_time"] = "Date de clôture";
        dict["logout"] = "Déconnexion";
        dict["apply"] = "Appliquer";
        dict["filter"] = "Filtrer";
        dict["connect"] = "Connecter";
        dict["timeleft"] = "Temps restant";
        dict["second"] = "seconde";
        dict["minute"] = "minute";
        dict["before_closing"] = "avant fermeture";
        dict["manager_close_cnx"] = "Le gestionnaire de session a coupé la connexion";
        dict["end_connection"] = "Fin de connexion";
        dict["help_message"] =
            "Dans la zone de saisie login, entrez:<br>"
            "- le nom de la machine cible et du compte<br>"
            "  sous la forme login@serveur.<br>"
            "- ou un nom de compte valide.<br>"
            "<br>"
            "Dans la zone de saisie mot de passe,<br>"
            "entrez le mot de passe du compte.<br>"
            "<br>"
            "Les deux champs sont sensibles à la casse.<br>"
            "<br>"
            "Contactez votre administrateur système en<br>"
            "cas de problème pour vous connecter.";
        dict["selector"] = "Sélecteur";
        dict["session_out_time"] = "L'autorisation de la session a expiré";
        dict["miss_keepalive"] = "Absence de réponse de Keepalive de l'ACL";
        dict["close_inactivity"] = "Fermeture sur inactivité";
        dict["acl_fail"] = "Echec du service d'authentification";
        dict["target_fail"] = "Echec de la connexion à la cible distante";
        dict["comment"] = "Commentaire";
        dict["no_results"] = "Aucun résultat";

        dict["back_selector"] = "Retour au Sélecteur";
        dict["exit"] = "Déconnexion";
        dict["comment"] = "Commentaire";
        dict["comment_r"] = "Commentaire *";
        dict["ticket"] = "Ticket n°";
        dict["ticket_r"] = "Ticket n° *";
        dict["duration"] = "Durée";
        dict["duration_r"] = "Durée *";
        dict["note_duration_format"] = "format: \"[heures]h[mins]m\" chaque unité est optionnelle.";
        dict["note_required"] = "(*) champs obligatoires.";
        dict["confirm"] = "Confirmer";
        dict["information"] = "Information";
    }
    void build_en_map() {
        trans_t & dict = this->en_map;
        dict["login"] = "Login";
        dict["password"] = "Password";
        dict["diagnostic"] = "Diagnostic";
        dict["connection_closed"] = "Connection closed";
        dict["OK"] = "OK";
        dict["cancel"] = "Cancel";
        dict["help"] = "Help";
        dict["close"] = "Close";
        dict["refused"] = "Refused";
        dict["username"] = "Username";
        dict["password_expire"] = "Your password will expire soon. Please change it.";
        dict["protocol"] = "Protocol";
        dict["target_group"] = "Target Group";
        dict["target"] = "Target";
        dict["close_time"] = "Close Time";
        dict["logout"] = "Logout";
        dict["apply"] = "Apply";
        dict["filter"] = "Filter";
        dict["connect"] = "Connect";
        dict["timeleft"] = "Time left";
        dict["second"] = "second";
        dict["minute"] = "minute";
        dict["before_closing"] = "before closing";
        dict["manager_close_cnx"] = "Connection closed by manager";
        dict["end_connection"] = "End of connection";
        dict["help_message"] =
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
        dict["selector"] = "Selector";
        dict["session_out_time"] = "Session is out of allowed timeframe";
        dict["miss_keepalive"] = "Missed keepalive from ACL";
        dict["close_inactivity"] = "Connection closed on inactivity";
        dict["acl_fail"] = "Authentifier service failed";
        dict["target_fail"] = "Failed to connect to remote TCP host";
        dict["comment"] = "Comment";
        dict["no_results"] = "No results found";

        dict["back_selector"] = "Back to Selector";
        dict["exit"] = "Exit";
        dict["comment"] = "Comment";
        dict["comment_r"] = "Comment *";
        dict["ticket"] = "Ticket n°";
        dict["ticket_r"] = "Ticket n° *";
        dict["duration"] = "Duration";
        dict["duration_r"] = "Duration *";
        dict["note_duration_format"] = "format \"[hours]h[mins]m\" each unit is optional.";
        dict["note_required"] = "(*) required fields.";
        dict["confirm"] = "Confirm";
        dict["information"] = "Information";

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

    static const char * get_value(const trans_t & trans, const char * key) {
        trans_t::const_iterator it = trans.find(key);
        return it != trans.end() ? it->second : 0;
    }

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

    const char * translate(const char * key) const {
        if (this->lang == FR) {
            return this->get_value(this->fr_map, key);
        }
        return this->get_value(this->en_map, key);
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

        res = TRANSLATIONCONF.translate(key);
        if (!res) {
            LOG(LOG_INFO, "Translation not found for '%s'", key);
            res = key;
        }
    }
    return res;
}

#endif
