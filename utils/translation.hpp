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
 *   Author(s): Christophe Grosjean, Meng Tan, Jonathan Poelen
 */

#ifndef REDEMPTION_TRANSLATION_HPP
#define REDEMPTION_TRANSLATION_HPP

#include <array>
#include <algorithm>
#include "config.hpp"

struct Translation
{
    enum language_t {
        EN,
        FR,
        MAX_LANG
    };
private:
    struct value_type
    {
        const char * key; const char * s;

        bool operator<(const value_type & other) const noexcept
        { return std::less<const char*>()(this->key, other.key); }

        bool operator<(const char * k) const noexcept
        { return std::less<const char*>()(this->key, k); }

        bool operator==(const char * k) const noexcept
        { return std::equal_to<const char*>()(this->key, k); }

        bool operator!=(const char * k) const noexcept
        { return !(*this == k); }
    };
    typedef std::array<value_type, 34> trans_t;

    language_t lang;
    trans_t trans[MAX_LANG];

    Translation()
    : lang(EN)
    , trans{trans_t{{
        {"login", "Login"},
        {"password", "Password"},
        {"diagnostic", "Diagnostic"},
        {"connection_closed", "Connection closed"},
        {"OK", "OK"},
        {"cancel", "Cancel"},
        {"help", "Help"},
        {"close", "Close"},
        {"refused", "Refused"},
        {"username", "Username"},
        {"password_expire", "Your password will expire soon. Please change it."},
        {"protocol", "Protocol"},
        {"target_group", "Target Group"},
        {"target", "Target"},
        {"close_time", "Close Time"},
        {"logout", "Logout"},
        {"apply", "Apply"},
        {"filter", "Filter"},
        {"connect", "Connect"},
        {"timeleft", "Time left"},
        {"second", "second"},
        {"minute", "minute"},
        {"before_closing", "before closing"},
        {"manager_close_cnx", "Connection closed by manager"},
        {"end_connection", "End of connection"},
        {"help_message",
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
            "experiencing problems."},
        {"selector", "Selector"},
        {"session_out_time", "Session is out of allowed timeframe"},
        {"miss_keepalive", "Missed keepalive from ACL"},
        {"close_inactivity", "Connection closed on inactivity"},
        {"acl_fail", "Authentifier service failed"},
        {"target_fail", "Failed to connect to remote TCP host"},
        {"comment", "Comment"},
        {"no_results", "No results found"},
    }}, trans_t{{
        {"login", "Identifiant"},
        {"password", "Mot de passe"},
        {"diagnostic", "Diagnostic"},
        {"connection_closed", "Connexion fermée"},
        {"OK", "OK"},
        {"cancel", "Annuler"},
        {"help", "Aide"},
        {"close", "Fermer"},
        {"refused", "Refuser"},
        {"username", "Utilisateur"},
        {"password_expire", "Votre mot de passe va bientôt expirer. Veuillez le changer"},
        {"protocol", "Protocole"},
        {"target_group", "Groupe"},
        {"target", "Cible"},
        {"close_time", "Date de clôture"},
        {"logout", "Déconnexion"},
        {"apply", "Appliquer"},
        {"filter", "Filtrer"},
        {"connect", "Connecter"},
        {"timeleft", "Temps restant"},
        {"second", "seconde"},
        {"minute", "minute"},
        {"before_closing", "avant fermeture"},
        {"manager_close_cnx", "Le gestionnaire de session a coupé la connexion"},
        {"end_connection", "Fin de connexion"},
        {"help_message",
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
            "cas de problème pour vous connecter."},
        {"selector", "Sélecteur"},
        {"session_out_time", "L'authorisation de la session a expirée"},
        {"miss_keepalive", "Absence de réponse de Keepalive de l'ACL"},
        {"close_inactivity", "Fermeture sur inactivité"},
        {"acl_fail", "Echec du service d'authentification"},
        {"target_fail", "Echec de la connexion à la cible distante"},
        {"comment", "Commentaire"},
        {"no_results", "Aucun résultat"},
    }}}
    {
        for (auto & t : this->trans) {
            std::sort(t.begin(), t.end());
        }
    }
    Translation(Translation const&) = delete;
    void operator=(Translation const&) = delete;

    static const char * get_value(const trans_t & trans, const char * key) {
        auto it = std::lower_bound(trans.begin(), trans.end(), key);
        return it == trans.end() || *it != key ? 0 : it->s;
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
        return this->get_value(this->trans[this->lang], key);
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

    if ((res == NULL) ||
        0 == strcmp(res, "") ||
        0 == strcmp(res, "ASK")) {

        const char * lang = ini.translation.language.get_cstr();
        if (0 == strcmp("fr", lang)) {
            TRANSLATIONCONF.set_lang(Translation::FR);
        }
        else if (0 == strcmp("en", lang)) {
            TRANSLATIONCONF.set_lang(Translation::EN);
        }
        res = TRANSLATIONCONF.translate(key);
        if (!res) {
            LOG(LOG_INFO, "Translation not found for '%s'", key);
            res = key;
        }
    }
    return res;
}

#endif
