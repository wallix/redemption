#!/usr/bin/python
# -*- coding: utf-8 -*-

# from fileconf import parse_conf_file

from typing import Dict


def TR(key: str) -> str:
    from logger import Logger
    language = SESMANCONF.language
    trans = SESMANCONF.conf.get(language)
    if trans is None:
        Logger().warning(f"No translation available for language '{language}', "
                         "falling back to english")
        trans = SESMANCONF['en']
    message = trans.get(key)
    if message is None:
        Logger().warning(f"No '{language}' translation available for key '{key}' "
                         "using key as message")
        message = key
    if key.count('%s') != message.count('%s'):
        Logger().warning(f"Variable parameters mismatch in '{language}' "
                         f"message for key '{key}' => '{message}'")
    return message


class Sesmsg(object):
    SESSION_CLOSED_S = 'session_closed_at %s'
    LICENCE_BLOCKER = 'licence_blocker'
    AUTH_FAILED_WAB_S = 'auth_failed_wab %s'
    ERROR_RECORD_PATH = 'error_getting_record_path'
    ERROR_RECORD_PATH_S = 'error_getting_record_path %s'
    VALID_AUTHORISATION = 'valid_authorisation'
    NOT_DISPLAY_MESSAGE = 'not_display_message'
    CHANGEPASSWORD = 'changepassword'
    CONNECTION_CLOSED_BY_CLIENT = 'Connection closed by client'
    EMPTY_USER = 'Empty user, try again'
    INVALID_USER = 'Invalid user, try again'
    NO_ERROR = 'No error'
    TARGET_S_NOT_FOUND = 'Target %s not found in user rights'
    TARGET_UNREACHABLE = 'Target unreachable'
    TOO_MANY_LOGIN_FAILURES = (
        'Too many login failures or selector orders, closing'
    )
    TRACE_WRITER_FAILED_S = 'Trace writer failed for %s'
    X509_AUTH_REFUSED_BY_USER = (
        'x509 browser authentication not validated by user'
    )
    URL_AUTH_REFUSED_BY_USER = (
        "URL Redirection authentication not validated by user"
    )
    NOT_ACCEPT_MESSAGE = 'not_accept_message'
    SELECTED_TARGET = 'selected_target'
    USERNAME_PARSE_ERROR_S = 'Username_parse_error %s'
    PASSWORD_EXPIRE_IN_S_DAYS = (
        'Your Bastion password will expire in %s days. Please change it.'
    )
    PASSWORD_EXPIRE_SOON = (
        'Your Bastion password will expire soon. Please change it.'
    )
    PASSTHROUGH_AUTH_FAILED_S = 'passthrough_auth_failed_wab %s'
    IN_SUBNET_S = 'in_subnet %s'
    NO_MATCH_SUBNET_S_S = 'no_match_subnet %s %s'
    ERROR_S = 'error %s'
    AUTHENTICATION_FAILED = 'authentication_failed'
    ACCOUNT_LOCKED = 'account_locked'
    START_SESSION_FAILED = 'start_session_failed'
    PATTERN_KILL = 'pattern_kill'
    SESPROBE_LAUNCH_FAILED = 'session_probe_launch_failed'
    SESPROBE_KEEPALIVE_MISSED = 'session_probe_keepalive_missed'
    SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED = (
        'session_probe_outbound_connection_blocking_failed'
    )
    SESPROBE_PROCESS_BLOCKING_FAILED = (
        'session_probe_process_blocking_failed'
    )
    SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION = (
        'session_probe_failed_to_run_startup_application'
    )
    SESPROBE_RECONNECTION = 'session_probe_reconnection'
    APPLICATION_FATAL_ERROR = 'application_fatal_error'


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
                Sesmsg.SESSION_CLOSED_S: "Votre session sera fermée à %s.",
                Sesmsg.LICENCE_BLOCKER: (
                    "Connexion refusée (par rapport à votre licence)"
                ),
                Sesmsg.AUTH_FAILED_WAB_S: (
                    "Echec de l'authentification pour l'utilisateur du wab %s"
                ),
                Sesmsg.ERROR_RECORD_PATH: (
                    "Impossible d'obtenir le répertoire d'enregistrement"
                ),
                Sesmsg.ERROR_RECORD_PATH_S: (
                    "Impossible d'obtenir le répertoire d'enregistrement %s"
                ),
                Sesmsg.VALID_AUTHORISATION: (
                    "Veuillez valider l'autorisation sur votre navigateur"
                ),
                Sesmsg.NOT_DISPLAY_MESSAGE: "Impossible d'afficher le message",
                Sesmsg.CHANGEPASSWORD: (
                    "Vous devez changer votre mot de passe "
                    "pour vous connecter."
                ),
                Sesmsg.CONNECTION_CLOSED_BY_CLIENT: (
                    'Connection terminée par le client'
                ),
                Sesmsg.EMPTY_USER: "Utilisateur non défini, reessayez.",
                Sesmsg.INVALID_USER: (
                    "Utilisateur non valide, réessayez."
                ),
                Sesmsg.NO_ERROR: "Pas d'erreur",
                Sesmsg.TARGET_S_NOT_FOUND: (
                    "Cible %s inconnu dans les droits de l'utilisateur"
                ),
                Sesmsg.TARGET_UNREACHABLE: "Impossible d'atteindre à cible",
                Sesmsg.TOO_MANY_LOGIN_FAILURES: (
                    "Trop d'échecs d'authentification, fermeture"
                ),
                Sesmsg.TRACE_WRITER_FAILED_S: (
                    "Echec de l'enregistrement des traces pour %s"
                ),
                Sesmsg.X509_AUTH_REFUSED_BY_USER: (
                    "Authentification X509 via le navigateur non validée."
                ),
                Sesmsg.URL_AUTH_REFUSED_BY_USER: (
                    "Authentification par URL Redirection non validée"
                ),
                Sesmsg.NOT_ACCEPT_MESSAGE: "Message de confirmation refusé.",
                Sesmsg.SELECTED_TARGET: 'Cible sélectionnée',
                Sesmsg.USERNAME_PARSE_ERROR_S: (
                    "Erreur de parsing du nom d'utilisateur %s"
                ),
                Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS: (
                    'Votre mot de passe Bastion expirera dans %s jours. '
                    'Vous devriez le changer.'
                ),
                Sesmsg.PASSWORD_EXPIRE_SOON: (
                    "L'expiration de votre mot de passe Bastion est imminente."
                    " Saisissez un nouveau mot de passe."
                ),
                Sesmsg.PASSTHROUGH_AUTH_FAILED_S: (
                    "Echec de l'authentification passthrough pour "
                    "l'utilisateur %s."
                ),
                Sesmsg.IN_SUBNET_S: "dans le sous-réseau %s.",
                Sesmsg.NO_MATCH_SUBNET_S_S: (
                    "%s n'appartient pas au sous-réseau %s."
                ),
                Sesmsg.ERROR_S: 'Erreur: %s',
                Sesmsg.AUTHENTICATION_FAILED: "Echec d'authentification.",
                Sesmsg.ACCOUNT_LOCKED: "Compte verrouillé.",
                Sesmsg.START_SESSION_FAILED: 'Echec de démarrage de session.',
                Sesmsg.PATTERN_KILL: (
                    'Motif interdit détecté, le gestionnaire de session a '
                    'coupé la connexion.'
                ),
                Sesmsg.SESPROBE_LAUNCH_FAILED: (
                    'Impossible de lancer Session Probe, le gestionnaire de '
                    'session a coupé la connexion.'
                ),
                Sesmsg.SESPROBE_KEEPALIVE_MISSED: (
                    'Keepalive de Session Probe est manquant, le '
                    'gestionnaire de session a coupé la connexion.'
                ),
                Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED: (
                    'Connexion sortante potentiellement indésirable detectée'
                    ', le gestionnaire de session a coupé la connexion.'
                ),
                Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED: (
                    'Processus potentiellement indésirable détecté, '
                    'le gestionnaire de session a coupé la connexion.'
                ),
                Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION: (
                    "Echec de lancement d'application de démarrage, "
                    "le gestionnaire de session a coupé la connexion."
                ),
                Sesmsg.SESPROBE_RECONNECTION: (
                    "Une reconnexion de Session Probe non autorisée est "
                    "détectée."
                ),
                Sesmsg.APPLICATION_FATAL_ERROR: (
                    "Une erreur fatale s'est produite et cette application a "
                    "été interrompue."
                ),
            },
            'en': {
                Sesmsg.SESSION_CLOSED_S: "Your session will close at %s.",
                Sesmsg.LICENCE_BLOCKER: (
                    "Connection rejected (due to licence limitation)"
                ),
                Sesmsg.AUTH_FAILED_WAB_S: (
                    "Authentication failed for wab user %s"
                ),
                Sesmsg.ERROR_RECORD_PATH: (
                    "Failed to get recording path"
                ),
                Sesmsg.ERROR_RECORD_PATH_S: (
                    "Failed to get recording path for %s"
                ),
                Sesmsg.VALID_AUTHORISATION: (
                    "Validate your authorisation on your browser"
                ),
                Sesmsg.NOT_DISPLAY_MESSAGE: (
                    "Unable to display message"
                ),
                Sesmsg.CHANGEPASSWORD: (
                    "You must change your password to login"
                ),
                Sesmsg.CONNECTION_CLOSED_BY_CLIENT: (
                    "Connection closed by client"
                ),
                Sesmsg.EMPTY_USER: "Empty user, try again",
                Sesmsg.INVALID_USER: "Invalid user, try again",
                Sesmsg.NO_ERROR: "No error",
                Sesmsg.TARGET_S_NOT_FOUND: (
                    "Target %s not found in user rights"
                ),
                Sesmsg.TARGET_UNREACHABLE: "Target unreachable",
                Sesmsg.TOO_MANY_LOGIN_FAILURES: (
                    "Too many login failures or selector orders, closing"
                ),
                Sesmsg.TRACE_WRITER_FAILED_S: "Trace writer failed for %s",
                Sesmsg.X509_AUTH_REFUSED_BY_USER: (
                    "X509 browser authentication not validated by user"
                ),
                Sesmsg.URL_AUTH_REFUSED_BY_USER: (
                    "URL Redirection authentication not validated by user"
                ),
                Sesmsg.NOT_ACCEPT_MESSAGE: 'Confirmation message refused.',
                Sesmsg.SELECTED_TARGET: 'Selected target',
                Sesmsg.USERNAME_PARSE_ERROR_S: 'Username parse error %s',
                Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS: (
                    'Your Bastion password will expire in %s days. Please '
                    'change it.'
                ),
                Sesmsg.PASSWORD_EXPIRE_SOON: (
                    'Your Bastion password will expire soon. Please change it.'
                ),
                Sesmsg.PASSTHROUGH_AUTH_FAILED_S: (
                    'Passthrough authentication failed for user %s.'
                ),
                Sesmsg.IN_SUBNET_S: "in %s subnet.",
                Sesmsg.NO_MATCH_SUBNET_S_S: "%s not in %s subnet.",
                Sesmsg.ERROR_S: 'Error: %s',
                Sesmsg.AUTHENTICATION_FAILED: 'Authentication failed.',
                Sesmsg.ACCOUNT_LOCKED: "Account locked.",
                Sesmsg.START_SESSION_FAILED: 'Failed to start session.',
                Sesmsg.PATTERN_KILL: (
                    'Forbidden pattern detected, '
                    'connection closed by manager.'
                ),
                Sesmsg.SESPROBE_LAUNCH_FAILED: (
                    'Could not launch Session Probe, '
                    'connection closed by manager.'
                ),
                Sesmsg.SESPROBE_KEEPALIVE_MISSED: (
                    'Session Probe keepalive missed, '
                    'connection closed by manager.'
                ),
                Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED: (
                    'Potentially undesirable outbound connection detected, '
                    'connection closed by manager.'
                ),
                Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED: (
                    'Potentially undesirable process detected, connection '
                    'closed by manager.'
                ),
                Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION: (
                    'Startup application launch failed, '
                    'connection closed by manager.'
                ),
                Sesmsg.SESPROBE_RECONNECTION: (
                    "Unauthorized Session Probe reconnection is detected."
                ),
                Sesmsg.APPLICATION_FATAL_ERROR: (
                    "A fatal error has occurred and this application has been "
                    "terminated."
                ),
            }
        }

    def __getitem__(self, para: str) -> Dict[str, str]:
        return self.conf[para]


SESMANCONF = SesmanConfig()
