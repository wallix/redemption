#!/usr/bin/python
# -*- coding: utf-8 -*-

# from fileconf import parse_conf_file

from typing import Dict


def TR(key: str) -> str:
    from logger import Logger
    originalkey = key
    key.replace(" %s", "")
    language = SESMANCONF.language
    # Logger().warning(f"Looking for translation for key '{key}' (in '{language}')")
    trans = SESMANCONF.conf.get(language)
    if trans is None:
        Logger().warning(f"No translation available for language '{language}', "
                         "falling back to english")
        trans = SESMANCONF['en']
    message = trans.get(originalkey)
    if message is None:
        Logger().warning(f"No '{language}' translation available for key '{key}' "
                         "using key as message")
        message = originalkey
    if originalkey.count('%s') != message.count('%s'):
        Logger().warning(f"Variable parameters mismatch in '{language}' "
                         f"message for key '{originalkey}' => '{message}'")
    return message


class SesmanConfig:
    def __init__(self):
        self.language = 'en'
        self.conf = {
            'sesman': {
                'record_warning': True,
                'debug': False,
                'auth_mode_passthrough': False,
                'default_login': '',
                'selector_filters_case_sensitive': False,
            },
            'fr': {
                'session_closed_at %s': "Votre session sera fermée à %s.",
                'licence_blocker': (
                    "Connexion refusée (par rapport à votre licence)"
                ),
                'auth_failed_wab %s': (
                    "Echec de l'authentification pour l'utilisateur du wab %s"
                ),
                'error_getting_record_path': (
                    "Impossible d'obtenir le répertoire d'enregistrement"
                ),
                'error_getting_record_path %s': (
                    "Impossible d'obtenir le répertoire d'enregistrement %s"
                ),
                'valid_authorisation': (
                    "Veuillez valider l'autorisation sur votre navigateur"
                ),
                'not_display_message': "Impossible d'afficher le message",
                'changepassword': (
                    "Vous devez changer votre mot de passe "
                    "pour vous connecter."
                ),
                "Connection closed by client": (
                    'Connection terminée par le client'
                ),
                "Empty user, try again": "Utilisteur non défini, reessayez.",
                "Invalid user, try again": (
                    "Utilisateur non valide, réessayez."
                ),
                "No error": "Pas d'erreur",
                "Target %s not found in user rights": (
                    "Cible %s inconnu dans les droits de l'utilisateur"
                ),
                "Target unreachable": "Target unreachable",
                "Too many login failures or selector orders, closing": (
                    "Trop de login incorrect, fermeture"
                ),
                "Trace writer failed for %s": (
                    "Echec de l'enregistrement des traces pour %s"
                ),
                "x509 browser authentication not validated by user": (
                    "Authentification X509 via le navigateur non validée."
                ),
                'not_accept_message': "Message de confirmation refusé.",
                'selected_target': 'Cible sélectionnée',
                'Username_parse_error %s': (
                    "Erreur de parsing du nom d'utilisateur %s"
                ),
                'Your Bastion password will expire in %s days. Please change it.': (
                    'Votre mot de passe Bastion expirera dans %s jours. '
                    'Vous devriez le changer.'
                ),
                'Your Bastion password will expire soon. Please change it.': (
                    "L'expiration de votre mot de passe Bastion est imminente."
                    " Saisissez un nouveau mot de passe."
                ),
                'passthrough_auth_failed_wab %s': (
                    "Echec de l'authentification passthrough pour "
                    "l'utilisateur %s."
                ),
                'In_subnet %s': "dans le sous-réseau %s.",
                'no_match_subnet %s %s': (
                    "%s n'appartient pas au sous-réseau %s."
                ),
                'error %s': 'Erreur: %s',
                'authentication_failed': "Echec d'authentification.",
                'account_locked': "Compte verrouillé.",
                'start_session_failed': 'Echec de démarage de session.',
                'pattern_kill': (
                    'Motif interdit detecté, le gestionnaire de session a '
                    'coupé la connexion.'
                ),
                'session_probe_launch_failed': (
                    'Impossible de lancer Session Probe, le gestionnaire de '
                    'session a coupé la connexion.'
                ),
                'session_probe_keepalive_missed': (
                    'Keepalive de Session Probe est manquant, le '
                    'gestionnaire de session a coupé la connexion.'
                ),
                'session_probe_outbound_connection_blocking_failed': (
                    'Connexion sortante potentiellement indésirable detectée'
                    ', le gestionnaire de session a coupé la connexion.'
                ),
                'session_probe_process_blocking_failed': (
                    'Processus potentiellement indésirable détecté, '
                    'le gestionnaire de session a coupé la connexion.'
                ),
                'session_probe_failed_to_run_startup_application': (
                    "Echec de lancement d'application de démarrage, "
                    "le gestionnaire de session a coupé la connexion."
                ),
                'session_probe_reconnection': (
                    "Une reconnexion de Session Probe non autorisée est "
                    "détectée."
                ),
                'application_fatal_error': (
                    "Une erreur fatale s'est produite et cette application a "
                    "été interrompue."
                ),
            },
            'en': {
                'session_closed_at %s': "Your session will close at %s.",
                'licence_blocker': (
                    "Connection rejected (due to licence limitation)"
                ),
                'auth_failed_wab %s': (
                    "Authentication failed for wab user %s"
                ),
                'error_getting_record_path': (
                    "Failed to get recording path"
                ),
                'error_getting_record_path %s': (
                    "Failed to get recording path for %s"
                ),
                'valid_authorisation': (
                    "Validate your authorisation on your browser"
                ),
                'not_display_message': (
                    "Unable to display message"
                ),
                'changepassword': (
                    "You must change your password to login"
                ),
                "Connection closed by client": (
                    "Connection closed by client"
                ),
                "Empty user, try again": "Empty user, try again",
                "Invalid user, try again": "Invalid user, try again",
                "No error": "No error",
                "Target %s not found in user rights": (
                    "Target %s not found in user rights"
                ),
                "Target unreachable": "Target unreachable",
                "Too many login failures or selector orders, closing": (
                    "Too many login failures or selector orders, closing"
                ),
                "Trace writer failed for %s": "Trace writer failed for %s",
                "x509 browser authentication not validated by user": (
                    "x509 browser authentication not validated by user"
                ),
                'not_accept_message': 'Confirmation message refused.',
                'selected_target': 'Selected target',
                'Username_parse_error %s': 'Username_parse_error %s',
                'Your Bastion password will expire in %s days. Please change it.': (
                    'Your Bastion password will expire in %s days. Please '
                    'change it.'
                ),
                'Your Bastion password will expire soon. Please change it.': (
                    'Your Bastion password will expire soon. Please change it.'
                ),
                'passthrough_auth_failed_wab %s': (
                    'Passthrough authentication failed for user %s.'
                ),
                'in_subnet %s': "in %s subnet.",
                'no_match_subnet %s %s': "%s not in %s subnet.",
                'error %s': 'Error: %s',
                'authentication_failed': 'Authentication failed.',
                'account_locked': "Account locked.",
                'start_session_failed': 'Failed to start session.',
                'pattern_kill': (
                    'Forbidden pattern detected, '
                    'connection closed by manager.'
                ),
                'session_probe_launch_failed': (
                    'Could not launch Session Probe, '
                    'connection closed by manager.'
                ),
                'session_probe_keepalive_missed': (
                    'Session Probe keepalive missed, '
                    'connection closed by manager.'
                ),
                'session_probe_outbound_connection_blocking_failed': (
                    'Potentially undesirable outbound connection detected, '
                    'connection closed by manager.'
                ),
                'session_probe_process_blocking_failed': (
                    'Potentially undesirable process detected, connection '
                    'closed by manager.'
                ),
                'session_probe_failed_to_run_startup_application': (
                    'Startup application launch failed, '
                    'connection closed by manager.'
                ),
                'session_probe_reconnection': (
                    "Unauthorized Session Probe reconnection is detected."
                ),
                'application_fatal_error': (
                    "A fatal error has occurred and this application has been "
                    "terminated."
                ),
            }
        }

    def __getitem__(self, para: str) -> Dict[str, str]:
        return self.conf[para]


SESMANCONF = SesmanConfig()
