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

#include "configs/autogen/enums.hpp" // Language

#include <cassert>
#include <type_traits>


namespace trkeys
{
    struct TrKey { const char * translations[2]; };

#define TR_PROTECTED_KV(name, en, fr) \
    constexpr struct TrKey##_##name \
    { const char * translations[2]; } name{{en, fr}}

    TR_PROTECTED_KV(password, "Password", "Mot de passe");

#undef TR_PROTECTED_KV

#define TR_KV(name, en, fr) constexpr TrKey name{{en, fr}}
    TR_KV(login, "Login", "Login");
    TR_KV(diagnostic, "Diagnostic", "Diagnostic");
    TR_KV(connection_closed, "Connection closed", "Connexion fermée");
    TR_KV(OK, "OK", "OK");
    TR_KV(cancel, "Cancel", "Annuler");
    TR_KV(help, "Help", "Aide");
    TR_KV(close, "Close", "Fermer");
    TR_KV(refused, "Refused", "Refuser");
    TR_KV(username, "Username", "Nom d'utilisateur");
    TR_KV(password_expire,
        "Your password will expire soon. Please change it.",
        "Votre mot de passe va bientôt expirer. Veuillez le changer");
    TR_KV(protocol, "Protocol", "Protocole");
    TR_KV(target_group, "Target Group", "Groupe");
    TR_KV(authorization, "Authorization", "Autorisation");
    TR_KV(target, "Target", "Cible");
    TR_KV(close_time, "Close Time", "Date de clôture");
    TR_KV(logout, "Logout", "Déconnexion");
    TR_KV(apply, "Apply", "Appliquer");
    TR_KV(filter, "Filter", "Filtrer");
    TR_KV(connect, "Connect", "Connecter");
    TR_KV(timeleft, "Time left", "Temps restant");
    TR_KV(second, "second", "seconde");
    TR_KV(minute, "minute", "minute");
    TR_KV(before_closing, "before closing", "avant fermeture");
    TR_KV(manager_close_cnx,
        "Connection closed by manager",
        "Le gestionnaire de session a coupé la connexion");
    TR_KV(end_connection, "End of connection", "Fin de connexion");
    TR_KV(help_message,
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
        "cas de problème pour vous connecter.");
    TR_KV(selector, "Selector", "Sélecteur");
    TR_KV(session_out_time,
        "Session is out of allowed timeframe",
        "L'autorisation de la session a expiré");
    TR_KV(miss_keepalive,
        "Missed keepalive from ACL",
        "Absence de réponse de Keepalive de l'ACL");
    TR_KV(close_inactivity,
        "Connection closed on inactivity",
        "Fermeture sur inactivité");
    TR_KV(acl_fail,
        "Authentifier service failed",
        "Echec du service d'authentification");
    TR_KV(target_fail,
        "Failed to connect to remote TCP host",
        "Echec de la connexion à la cible distante");
    TR_KV(no_results, "No results found", "Aucun résultat");
    TR_KV(back_selector, "Back to Selector", "Retour au Sélecteur");
    TR_KV(exit, "Exit", "Sortir");
    TR_KV(comment, "Comment", "Commentaire");
    TR_KV(comment_r, "Comment *", "Commentaire *");
    TR_KV(ticket, "Ticket Ref.", "Ticket Ref.");
    TR_KV(ticket_r, "Ticket Ref. *", "Ticket Ref. *");
    TR_KV(duration, "Duration", "Durée");
    TR_KV(duration_r, "Duration *", "Durée *");
    TR_KV(note_duration_format, "Format: [hours]h[mins]m", "Format: [heures]h[mins]m");
    TR_KV(note_required, "(*) required fields", "(*) champs requis");
    TR_KV(confirm, "Confirm", "Confirmer");
    TR_KV(fmt_field_required,
        "Error: %s field is required.",
        "Erreur: le champ %s est requis.");
    TR_KV(fmt_invalid_format,
        "Error: %s invalid format.",
        "Erreur: format %s invalide.");
    TR_KV(fmt_toohigh_duration,
        "Error: %s is too high (must be < 10000 h).",
        "Erreur: %s trop haute (doit être < 10000 h).");
    TR_KV(information, "Information", "Information");
    TR_KV(authentication_required,  "Authentication Required", "Authentification Requise");
    TR_KV(target_info_required, "Target Information Required", "Informations Cible Requises");
    TR_KV(device, "Device", "Machine");
    TR_KV(disable_osd,
        "(insert key or left click to hide)",
        "(cacher avec touche insert ou clic gauche)");
    TR_KV(disconnected_by_otherconnection,
        "Another user connected to the resource, so your connection was lost.",
        "Un autre utilisateur s'est connecté à la ressource, provoquant la perte de votre connexion."
    );
    TR_KV(process_interrupted_security_policies,
        "The process '%s' was interrupted in accordance with security policies.",
        "Le processus '%s' a été interrompu conformément aux politiques de sécurité."
    );
    TR_KV(session_logoff_in_progress,
        "Session logoff in progress.",
        "Fermeture de session en cours."
    );
    TR_KV(starting_remoteapp,
        "Starting RemoteApp.",
        "Lancement de RemoteApp..."
    );
    TR_KV(closing_remoteapp,
        "All RemoteApp windows are closed.",
        "Toutes les fenêtre de RemoteApp sont fermées."
    );
    TR_KV(disconnect_now,
        "Disconnect Now",
        "Se déconnecter maintenant"
    );
#undef TR_KV
}

class Inifile;

struct Translation
{
    enum language_t {
        EN,
        FR,
        MAX_LANG
    };

private:
    language_t lang;
    Inifile * ini = nullptr;

    Translation() = default;
    Translation(Translation const&) = delete;
    void operator=(Translation const&) = delete;

public:
    static Translation& getInstance()
    {
        static Translation instance;
        return instance;
    }

    bool set_lang(language_t lang)
    {
        if (lang >= MAX_LANG)
            return false;
        this->lang = lang;
        return true;
    }

    void set_ini(Inifile * ini)
    {
        this->ini = ini;
    }

    // implementation in config.hpp
    const char * translate(trkeys::TrKey_password k) const;

    const char * translate(trkeys::TrKey k) const
    {
        return k.translations[this->lang];
    }
};

#define TRANSLATIONCONF Translation::getInstance()

inline const char * TR(trkeys::TrKey_password k, Translation::language_t lang)
{
    TRANSLATIONCONF.set_lang(lang);
    return TRANSLATIONCONF.translate(k);
}

inline const char * TR(trkeys::TrKey k, Translation::language_t lang)
{
    TRANSLATIONCONF.set_lang(lang);
    return TRANSLATIONCONF.translate(k);
}

struct Translator
{
    explicit Translator(Translation::language_t lang = Translation::EN)
      : lang(lang)
    {}

    char const * operator()(trkeys::TrKey_password const & t) const
    {
        return TR(t, this->lang);
    }

    char const * operator()(trkeys::TrKey const & t) const
    {
        return TR(t, this->lang);
    }

private:
    Translation::language_t lang;
};

// implementation in config.hpp
Translation::language_t language(Inifile const & ini);

inline Translation::language_t language(Language lang)
{
    return static_cast<Translation::language_t>(lang);
}
