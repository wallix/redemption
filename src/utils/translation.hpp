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


namespace trkeys
{
    struct TrKey { const char * translations[2]; };

#define TR_PROTECTED_KV(name, en, fr) \
    constexpr struct TrKey##_##name   \
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
        "Your Bastion password will expire soon. Please change it.",
        "Votre mot de passe Bastion va bientôt expirer. Veuillez le changer.");
    TR_KV(protocol, "Protocol", "Protocole");
    TR_KV(authorization, "Authorization", "Autorisation");
    TR_KV(target, "Target", "Cible");
    TR_KV(description, "Description", "Description");
    TR_KV(close_time, "Close Time", "Date de clôture");
    TR_KV(logout, "Logout", "Déconnexion");
    TR_KV(apply, "Apply", "Appliquer");
    TR_KV(filter, "Filter", "Filtrer");
    TR_KV(connect, "Connect", "Connecter");
    TR_KV(timeleft, "Time left", "Temps restant");
    TR_KV(second, "second", "seconde");
    TR_KV(minute, "minute", "minute");
    TR_KV(hour, "hour", "heure");
    TR_KV(before_closing, "before closing", "avant fermeture");

    TR_KV(enable_rt_display, "Your session is observed by an administrator", "Votre session est observée par un administrateur.");
    TR_KV(disable_rt_display, "Your session is no longer observed by an administrator.", "Votre session n'est plus observée par un administrateur.");

    TR_KV(manager_close_cnx,
        "Connection closed by manager.",
        "Le gestionnaire de session a coupé la connexion.");
    TR_KV(end_connection, "End of connection", "Fin de connexion");
    TR_KV(help_message,
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
        "experiencing problems.",

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
        "Failed to connect to remote host.",
        "Échec de la connexion à l'hôte distant.");

    TR_KV(authentification_rdp_fail,
        "Failed to authenticate with remote RDP host.",
        "Échec de l'authentification avec l'hôte RDP distant.");
    TR_KV(authentification_vnc_fail,
        "Failed to authenticate with remote VNC host.",
        "Échec de l'authentification avec l'hôte VNC distant.");
    TR_KV(authentification_x_fail,
        "Failed to authenticate with remote X host.",
        "Échec de l'authentification avec l'hôte X distant.");
    TR_KV(connection_ended,
        "Connection to server ended.",
        "Connexion au serveur terminée.");

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
        "Error: %s is too high (max: %d minutes).",
        "Erreur: %s trop haute (max: %d minutes).");
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
        "Starting RemoteApp ...",
        "Lancement de RemoteApp ..."
    );
    TR_KV(closing_remoteapp,
        "All RemoteApp windows are closed.",
        "Toutes les fenêtre de RemoteApp sont fermées."
    );
    TR_KV(disconnect_now,
        "Disconnect Now",
        "Se déconnecter maintenant"
    );
    TR_KV(err_rdp_server_redir,
        "The computer that you are trying to connect to is redirecting you to another computer!",
        "L'ordinateur auquel vous essayez de vous connecter vous redirige vers un autre ordinateur!"
    );
    TR_KV(err_nla_authentication_failed,
        "NLA Authentication Failed!",
        "Échec d'authentification NLA!"
    );
    TR_KV(err_transport_tls_certificate_changed,
        "TLS certificate changed!",
        "Certificat TLS modifié!"
    );
    TR_KV(err_transport_tls_certificate_missed,
        "TLS certificate missed!",
        "Certificat TLS est manquant!"
    );
    TR_KV(err_transport_tls_certificate_corrupted,
        "TLS certificate corrupted!",
        "Certificat TLS est corrompu!"
    );
    TR_KV(err_transport_tls_certificate_inaccessible,
        "TLS certificate  is inaccessible!",
        "Certificat TLS est inaccessible!"
    );
    TR_KV(err_vnc_connection_error,
        "VNC connection error!",
        "Error de connexion VNC!"
    );
    TR_KV(err_rdp_unsupported_monitor_layout,
        "Unsupported client display monitor layout!",
        "La disposition du moniteur d'affichage du client n'est pas supportée!"
    );
    TR_KV(err_lic,
        "An error occurred during the licensing protocol!",
        "Une erreur de protocole de licence s'est produite!"
    );
    TR_KV(err_rail_client_execute,
        "The RemoteApp program did not start on the remote computer!",
        "Le programme RemoteApp n'a pas démarré sur l'ordinateur distant!"
    );
    TR_KV(err_rail_starting_program,
        "Cannot start the RemoteApp program!",
        "Impossible de démarrer le programme RemoteApp!"
    );
    TR_KV(err_rail_unauthorized_program,
        "The RemoteApp program is not in the list of authorized programs!",
        "Le programme RemoteApp n'est pas dans la liste des programmes autorisés!"
    );
    TR_KV(err_rdp_open_session_timeout,
        "Logon timer expired!",
        "Le délai d'attente d'ouverture de session a expiré!"
    );
    TR_KV(err_session_probe_launch,
        "Could not launch Session Probe!",
        "Impossible de lancer Session Probe!"
    );
    TR_KV(err_session_probe_asbl_fsvc_unavailable,
        "(ASBL) Could not launch Session Probe! File System Virtual Channel is unavailable. Please allow the drive redirection in the Remote Desktop Services settings of the target.",
        "(ASBL) Impossible de lancer Session Probe! Le canal virtuel du système de fichiers n'est pas disponible. Veuillez autorisez la redirection du lecteur dans les paramètres des Services de bureau à distance de la cible."
    );
    TR_KV(err_session_probe_asbl_maybe_something_blocks,
        "(ASBL) Could not launch Session Probe! Maybe something blocks it on the target. Is the target running under Microsoft Server products? The Command Prompt should be published as the RemoteApp program and accept any command-line parameters. Please also check the temporary directory to ensure there is enough free space.",
        "(ASBL) Impossible de lancer Session Probe! Peut-être que quelque chose le bloque sur la cible. La cible fonctionne-t-elle sous un produit de famille Microsoft Server? L'invite de commande doit être publiée comme programme RemoteApp et accepter tout type de paramètres de ligne de commande. Veuillez vérifier également le répertoire temporaire afin d'assurer qu'il y a suffisamment d'espace libre."
    );
    TR_KV(err_session_probe_asbl_unknown_reason,
        "(ASBL) Session Probe launch has failed for unknown reason!",
        "(ASBL) Le lancement de Session Probe a échoué pour une raison inconnue!"
    );
    TR_KV(err_session_probe_cbbl_fsvc_unavailable,
        "(CBBL) Could not launch Session Probe! File System Virtual Channel is unavailable. Please allow the drive redirection in the Remote Desktop Services settings of the target.",
        "(CBBL) Impossible de lancer Session Probe! Le canal virtuel du système de fichiers n'est pas disponible. Veuillez autorisez la redirection du lecteur dans les paramètres des Services de bureau à distance de la cible."
    );
    TR_KV(err_session_probe_cbbl_cbvc_unavailable,
        "(CBBL) Could not launch Session Probe! Clipboard Virtual Channel is unavailable. Please allow the clipboard redirection in the Remote Desktop Services settings of the target.",
        "(CBBL) Impossible de lancer Session Probe! Le canal virtuel du presse-papier n'est pas disponible. Veuillez autorisez la redirection du presse-papier dans les paramètres des Services de bureau à distance de la cible."
    );
    TR_KV(err_session_probe_cbbl_drive_not_ready_yet,
        "(CBBL) Could not launch Session Probe! Drive of Session Probe is not ready yet. Is the target running under Windows Server 2008 R2 or more recent version?",
        "(CBBL) Impossible de lancer Session Probe! Le lecteur de Session Probe n'est pas encore prêt. La cible fonctionne-t-elle sous Windows Server 2008 R2 ou une version plus récente?"
    );
    TR_KV(err_session_probe_cbbl_maybe_something_blocks,
        "(CBBL) Session Probe is not launched! Maybe something blocks it on the target. Please also check the temporary directory to ensure there is enough free space.",
        "(CBBL) Impossible de lancer Session Probe! Peut-être que quelque chose le bloque sur la cible. Veuillez vérifier également le répertoire temporaire afin d'assurer qu'il y a suffisamment d'espace libre."
    );
    TR_KV(err_session_probe_cbbl_launch_cycle_interrupted,
        "(CBBL) Session Probe launch cycle has been interrupted! The launch timeout duration may be too short.",
        "(CBBL) Le cycle de lancement de Session Probe a été interrompu! La durée du délai d'attente de lancement peut être trop courte."
    );
    TR_KV(err_session_probe_cbbl_unknown_reason_refer_to_syslog,
        "(CBBL) Session Probe launch has failed for unknown reason! Please refer to the syslog file for more detailed information regarding the error condition.",
        "(CBBL) Le lancement de Session Probe a échoué pour une raison inconnue! Veuillez vous reporter au fichier syslog pour obtenir des informations plus détaillées concernant la condition d'erreur."
    );
    TR_KV(err_session_probe_rp_launch_refer_to_syslog,
        "(RP) Could not launch Session Probe! Please refer to the syslog file for more detailed information regarding the error condition.",
        "(RP) Impossible de lancer Session Probe! Veuillez vous reporter au fichier syslog pour obtenir des informations plus détaillées concernant la condition d'erreur."
    );
    TR_KV(err_session_unknown_backend,
        "Unknown backend failure.",
        "Erreur de backend inconnue."
    );
    TR_KV(err_login_password,
        "Provided login/password is probably incorrect.",
        "Le nom d'utilisateur/mot de passe fourni est probablement incorrect."
    );
    TR_KV(err_nla_required,
        "Enable NLA is probably required.",
        "Il est probablement nécessaire d'activer NLA."
    );
    TR_KV(err_tls_required,
        "Enable TLS is probably required.",
        "Il est probablement nécessaire d'activer TLS."
    );
    TR_KV(err_server_denied_connection,
        "Please check provided Load Balance Info.",
        "Veuillez vérifier le Load Balance Info fournies."
    );

    TR_KV(err_mod_rdp_nego,
        "Fail during TLS security exchange.",
        "Échec lors de l'échange de sécurité TLS."
    );
    TR_KV(err_mod_rdp_basic_settings_exchange,
        "Fail during basic setting exchange.",
        "Échec lors de l'échange de paramètres de base."
    );
    TR_KV(err_mod_rdp_channel_connection_attach_user,
        "Fail during channels connection.",
        "Échec lors de la connexion des canaux."
    );
    TR_KV(mod_rdp_channel_join_confirme,
        "Fail during channels connection.",
        "Échec lors de la connexion des canaux."
    );
    TR_KV(mod_rdp_get_license,
        "Failed while trying to get licence.",
        "Échec en essayant d'obtenir une licence."
    );
    TR_KV(err_mod_rdp_connected,
        "Fail while connecting session on the target.",
        "Échec lors de la connexion de la session sur la cible."
    );
#undef TR_KV
} // namespace trkeys

class Inifile;

struct Translation
{
    enum language_t : unsigned char
    {
        EN,
        FR,
        MAX_LANG
    };

private:
    language_t lang;
    Inifile * ini = nullptr;

    Translation() = default;

public:
    Translation(Translation const&) = delete;
    void operator=(Translation const&) = delete;

    static Translation& getInstance()
    {
        static Translation instance;
        return instance;
    }

    bool set_lang(language_t lang)
    {
        if (lang >= MAX_LANG) {
            return false;
        }
        this->lang = lang;
        return true;
    }

    void set_ini(Inifile * ini)
    {
        this->ini = ini;
    }

    // implementation in config.cpp
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

// implementation in config.cpp
Translation::language_t language(Inifile const & ini);

inline Translation::language_t language(Language lang)
{
    return static_cast<Translation::language_t>(lang);
}

struct Translator
{
    explicit Translator(Translation::language_t lang = Translation::EN)
      : lang(lang)
    {}

    explicit Translator(Inifile const & ini)
      : lang(language(ini))
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
