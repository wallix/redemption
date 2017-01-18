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


#pragma once

#include <array>
#include <algorithm>
#include "configs/config.hpp"
#include "utils/log.hpp"

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
        const char * key;

        struct Translation {
            const char * translation[MAX_LANG];
            Translation(const char * en, const char * fr)
            : translation{en, fr}
            {}
            const char * operator[](language_t i) const { return this->translation[i]; }
        } translation;

        bool operator<(const value_type & other) const noexcept
        { return *this < other.key; }

        // WARNING comparison with string literal results in unspecified behaviour
        bool operator<(const char * k) const noexcept
        { return std::less<const char*>()(this->key, k); }

        // WARNING comparison with string literal results in unspecified behaviour
        bool operator==(const char * k) const noexcept
        { return std::equal_to<const char*>()(this->key, k); }

        bool operator!=(const char * k) const noexcept
        { return !(*this == k); }
    };

    typedef std::array<value_type, 60> trans_t;

    language_t lang;
    trans_t trans;

    Inifile * ini = nullptr;

    Translation()
    : lang(EN)
    , trans{{
        {"login", {"Login", "Login"}},
        {"password", {"Password", "Mot de passe"}},
        {"diagnostic", {"Diagnostic", "Diagnostic"}},
        {"connection_closed", {"Connection closed", "Connexion fermée"}},
        {"OK", {"OK", "OK"}},
        {"cancel", {"Cancel", "Annuler"}},
        {"help", {"Help", "Aide"}},
        {"close", {"Close", "Fermer"}},
        {"refused", {"Refused", "Refuser"}},
        {"username", {"Username", "Nom d'utilisateur"}},
        {"password_expire", {
            "Your password will expire soon. Please change it.",
            "Votre mot de passe va bientôt expirer. Veuillez le changer"
        }},
        {"protocol", {"Protocol", "Protocole"}},
        {"target_group", {"Target Group", "Groupe"}},
        {"authorization", {"Authorization", "Autorisation"}},
        {"target", {"Target", "Cible"}},
        {"close_time", {"Close Time", "Date de clôture"}},
        {"logout", {"Logout", "Déconnexion"}},
        {"apply", {"Apply", "Appliquer"}},
        {"filter", {"Filter", "Filtrer"}},
        {"connect", {"Connect", "Connecter"}},
        {"timeleft", {"Time left", "Temps restant"}},
        {"second", {"second", "seconde"}},
        {"minute", {"minute", "minute"}},
        {"before_closing", {"before closing", "avant fermeture"}},
        {"manager_close_cnx", {"Connection closed by manager",
                               "Le gestionnaire de session a coupé la connexion"}},
        {"end_connection", {"End of connection", "Fin de connexion"}},
        {"help_message", {
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
            "experiencing problems.",

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
        }},
        {"selector", {"Selector", "Sélecteur"}},
        {"session_out_time", {"Session is out of allowed timeframe",
                              "L'autorisation de la session a expiré"}},
        {"miss_keepalive", {"Missed keepalive from ACL",
                            "Absence de réponse de Keepalive de l'ACL"}},
        {"close_inactivity", {"Connection closed on inactivity",
                              "Fermeture sur inactivité"}},
        {"acl_fail", {"Authentifier service failed",
                      "Echec du service d'authentification"}},
        {"target_fail", {"Failed to connect to remote TCP host",
                         "Echec de la connexion à la cible distante"}},
        {"comment", {"Comment", "Commentaire"}},
        {"no_results", {"No results found", "Aucun résultat"}},
        {"back_selector", {"Back to Selector", "Retour au Sélecteur"}},
        {"exit", {"Exit", "Sortir"}},
        {"comment", {"Comment", "Commentaire"}},
        {"comment_r", {"Comment *", "Commentaire *"}},
        {"ticket", {"Ticket Ref.", "Ticket Ref."}},
        {"ticket_r", {"Ticket Ref. *", "Ticket Ref. *"}},
        {"duration", {"Duration", "Durée"}},
        {"duration_r", {"Duration *", "Durée *"}},
        {"note_duration_format",
                {"Format: [hours]h[mins]m",
                 "Format: [heures]h[mins]m"}},
        {"note_required", {"(*) required fields", "(*) champs requis"}},
        {"confirm", {"Confirm", "Confirmer"}},
        {"%s field_required", {"Error: %s field is required.",
                               "Erreur: le champ %s est requis."}},
        {"%s invalid_format", {"Error: %s invalid format.",
                               "Erreur: format %s invalide."}},
        {"%s toohigh_duration", {"Error: %s is too high (must be < 10000 h).",
                                 "Erreur: %s trop haute (doit être < 10000 h)."}},
        {"information", {"Information", "Information"}},
        {"authentication_required", {"Authentication Required",
                                     "Authentification Requise"}},
        {"target_info_required", {"Target Information Required",
                                  "Informations Cible Requises"}},
        {"device", {"Device",
                    "Machine"}},
        {"disable_osd", {"(insert key or left click to hide)",
                         "(cacher avec touche insert ou clic gauche)"}},
        {"disconnected_by_otherconnection",
            {"Another user connected to the resource, so your connection was lost.",
             "Un autre utilisateur s'est connecté à la ressource, provoquant la perte de votre connexion."}},
        {"process_interrupted_security_policies",
            {"The process '%s' was interrupted in accordance with security policies.",
             "Le processus '%s' a été interrompu conformément aux politiques de sécurité."}},
        {"session_logoff_in_progress",
            {"Session logoff in progress.",
             "Fermeture de session en cours."}},

        {"starting_remoteapp",
            {"Starting RemoteApp.",
             "Lancement de RemoteApp..."}},

        {"closing_remoteapp",
            {"All RemoteApp windows are closed.",
             "Toutes les fenêtre de RemoteApp sont fermées."}},

        {"disconnect_now",
            {"Disconnect Now",
             "Se déconnecter maintenant"}},
    }}
    {
        std::sort(this->trans.begin(), this->trans.end());
    }
    Translation(Translation const&) = delete;
    void operator=(Translation const&) = delete;

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

    void set_ini(Inifile * ini) {
        this->ini = ini;
    }

    const char * translate(const char * key) const {
        if (this->ini) {
            if (this->lang == Translation::EN) {
                if ((0 == strcmp("password", key)) &&
                    !this->ini->get<cfg::translation::password_en>().empty()) {
                    return this->ini->get<cfg::translation::password_en>().c_str();
                }
            }
            else if (this->lang == Translation::FR) {
                if ((0 == strcmp("password", key)) &&
                    !this->ini->get<cfg::translation::password_fr>().empty()) {
                    return this->ini->get<cfg::translation::password_fr>().c_str();
                }
            }
        }

        auto it = std::lower_bound(trans.begin(), trans.end(), key);
        return it == trans.end() || *it != key ? nullptr : it->translation[this->lang];
    }
};

#define TRANSLATIONCONF Translation::getInstance()
static inline const char * TR(const char * key, Translation::language_t lang)
{
    const char * res = nullptr;

    TRANSLATIONCONF.set_lang(lang);
    res = TRANSLATIONCONF.translate(key);
    if (!res) {
        LOG(LOG_INFO, "Translation not found for '%s'", key);
        res = key;
    }

    return res;
}

struct Translator {
    explicit Translator(Translation::language_t lang = Translation::EN)
      : lang(lang)
    {}

    char const * operator()(char const * key) const {
        return TR(key, this->lang);
    }

private:
    Translation::language_t lang;
};

inline Translation::language_t language(Inifile const & ini) {
    return static_cast<Translation::language_t>(ini.get<cfg::translation::language>());
}

inline Translation::language_t language(Language lang) {
    return static_cast<Translation::language_t>(lang);
}
