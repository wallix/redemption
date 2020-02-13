#!/usr/bin/python
# -*- coding: utf-8 -*-

# from fileconf import parse_conf_file


def TR(key):
    from logger import Logger
    originalkey = key
    key.replace(" %s", "")
    # Logger().warning(u"Looking for translation for key '%s' (in '%s')" %
    #                  (key, SESMANCONF.language))
    trans = SESMANCONF.get(SESMANCONF.language)
    if trans is None:
        Logger().warning(u"No translation available for language '%s', "
                         u"falling back to english" % SESMANCONF.language)
        trans = SESMANCONF.get('en')
    message = trans.get(originalkey)
    if message is None:
        Logger().warning(u"No '%s' translation available for key '%s' "
                         u"using key as message" % (SESMANCONF.language, key))
        message = originalkey
    if originalkey.count(u'%s') != message.count(u'%s'):
        Logger().warning(u"Variable parameters mismatch in '%s' "
                         u"message for key '%s' => '%s'" %
                         (SESMANCONF.language, originalkey, message))
    return message


class SesmanConfig(object):

    def __init__(self):
        self.language = 'en'
        self.conf = {
            u'sesman': {
                u'record_warning': True,
                u'debug': False,
                u'auth_mode_passthrough': False,
                u'default_login': u'',
                u'selector_filters_case_sensitive': False,
            },
            u'fr': {
                u'session_closed_at %s': u"Votre session sera fermée à %s.",
                u'licence_blocker': (
                    u"Connexion refusée (par rapport à votre licence)"
                ),
                u'auth_failed_wab %s': (
                    u"Echec de l'authentification pour l'utilisateur du wab %s"
                ),
                u'error_getting_record_path': (
                    u"Impossible d'obtenir le répertoire d'enregistrement"
                ),
                u'error_getting_record_path %s': (
                    u"Impossible d'obtenir le répertoire d'enregistrement %s"
                ),
                u'valid_authorisation': (
                    u"Veuillez valider l'autorisation sur votre navigateur"
                ),
                u'not_display_message': u"Impossible d'afficher le message",
                u'changepassword': (
                    u"Vous devez changer votre mot de passe<br>"
                    u"pour vous connecter."
                ),
                u"Connection closed by client": (
                    u'Connection terminée par le client'
                ),
                u"Empty user, try again": u"Utilisteur non défini, reessayez.",
                u"Invalid user, try again": (
                    u"Utilisateur non valide, réessayez."
                ),
                u"No error": u"Pas d'erreur",
                u"Target %s not found in user rights": (
                    u"Cible %s inconnu dans les droits de l'utilisateur"
                ),
                u"Target unreachable": u"Target unreachable",
                u"Too many login failures or selector orders, closing": (
                    u"Trop de login incorrect, fermeture"
                ),
                u"Trace writer failed for %s": (
                    u"Echec de l'enregistrement des traces pour %s"
                ),
                u"x509 browser authentication not validated by user": (
                    u"Authentification X509 via le navigateur non validée."
                ),
                u'not_accept_message': u"Message de confirmation refusé.",
                u'selected_target': u'Cible sélectionnée',
                u'Username_parse_error %s': (
                    u"Erreur de parsing du nom d'utilisateur %s"
                ),
                u'Your Bastion password will expire in %s days. Please change it.': (
                    u'Votre mot de passe Bastion expirera dans %s jours. '
                    u'Vous devriez le changer.'
                ),
                u'Your Bastion password will expire soon. Please change it.': (
                    u"L'expiration de votre mot de passe Bastion est imminente."
                    u" Saisissez un nouveau mot de passe."
                ),
                u'passthrough_auth_failed_wab %s': (
                    u"Echec de l'authentification passthrough pour "
                    u"l'utilisateur %s."
                ),
                u'In_subnet %s': u"dans le sous-réseau %s.",
                u'no_match_subnet %s %s': (
                    u"%s n'appartient pas au sous-réseau %s."
                ),
                u'error %s': u'Erreur: %s',
                u'authentication_failed': u"Echec d'authentification.",
                u'account_locked': u"Compte verrouillé.",
                u'start_session_failed': u'Echec de démarage de session.',
                u'pattern_kill': (
                    u'Motif interdit detecté, le gestionnaire de session a '
                    u'coupé la connexion.'
                ),
                u'session_probe_launch_failed': (
                    u'Impossible de lancer Session Probe, le gestionnaire de '
                    u'session a coupé la connexion.'
                ),
                u'session_probe_keepalive_missed': (
                    u'Keepalive de Session Probe est manquant, le '
                    u'gestionnaire de session a coupé la connexion.'
                ),
                u'session_probe_outbound_connection_blocking_failed': (
                    u'Connexion sortante potentiellement indésirable detectée'
                    u', le gestionnaire de session a coupé la connexion.'
                ),
                u'session_probe_process_blocking_failed': (
                    u'Processus potentiellement indésirable détecté, '
                    u'le gestionnaire de session a coupé la connexion.'
                ),
                u'session_probe_failed_to_run_startup_application': (
                    u"Echec de lancement d'application de démarrage, "
                    u"le gestionnaire de session a coupé la connexion."
                ),
                u'session_probe_reconnection': (
                    u"Une reconnexion de Session Probe non autorisée est "
                    u"détectée."
                ),
            },
            u'en': {
                u'session_closed_at %s': u"Your session will close at %s.",
                u'licence_blocker': (
                    u"Connection rejected (due to licence limitation)"
                ),
                u'auth_failed_wab %s': (
                    u"Authentication failed for wab user %s"
                ),
                u'error_getting_record_path': (
                    u"Failed to get recording path"
                ),
                u'error_getting_record_path %s': (
                    u"Failed to get recording path for %s"
                ),
                u'valid_authorisation': (
                    u"Validate your authorisation on your browser"
                ),
                u'not_display_message': (
                    u"Unable to display message"
                ),
                u'changepassword': (
                    u"You must change your password to login"
                ),
                u"Connection closed by client": (
                    u"Connection closed by client"
                ),
                u"Empty user, try again": u"Empty user, try again",
                u"Invalid user, try again": u"Invalid user, try again",
                u"No error": u"No error",
                u"Target %s not found in user rights": (
                    u"Target %s not found in user rights"
                ),
                u"Target unreachable": u"Target unreachable",
                u"Too many login failures or selector orders, closing": (
                    u"Too many login failures or selector orders, closing"
                ),
                u"Trace writer failed for %s": u"Trace writer failed for %s",
                u"x509 browser authentication not validated by user": (
                    u"x509 browser authentication not validated by user"
                ),
                u'not_accept_message': u'Confirmation message refused.',
                u'selected_target': u'Selected target',
                u'Username_parse_error %s': u'Username_parse_error %s',
                u'Your Bastion password will expire in %s days. Please change it.': (
                    u'Your Bastion password will expire in %s days. Please '
                    u'change it.'
                ),
                u'Your Bastion password will expire soon. Please change it.': (
                    u'Your Bastion password will expire soon. Please change it.'
                ),
                u'passthrough_auth_failed_wab %s': (
                    u'Passthrough authentication failed for user %s.'
                ),
                u'in_subnet %s': u"in %s subnet.",
                u'no_match_subnet %s %s': u"%s not in %s subnet.",
                u'error %s': u'Error: %s',
                u'authentication_failed': u'Authentication failed.',
                u'account_locked': u"Account locked.",
                u'start_session_failed': u'Failed to start session.',
                u'pattern_kill': (
                    u'Forbidden pattern detected, '
                    u'connection closed by manager.'
                ),
                u'session_probe_launch_failed': (
                    u'Could not launch Session Probe, '
                    u'connection closed by manager.'
                ),
                u'session_probe_keepalive_missed': (
                    u'Session Probe keepalive missed, '
                    u'connection closed by manager.'
                ),
                u'session_probe_outbound_connection_blocking_failed': (
                    u'Potentially undesirable outbound connection detected, '
                    u'connection closed by manager.'
                ),
                u'session_probe_process_blocking_failed': (
                    u'Potentially undesirable process detected, connection '
                    u'closed by manager.'
                ),
                u'session_probe_failed_to_run_startup_application': (
                    u'Startup application launch failed, '
                    u'connection closed by manager.'
                ),
                u'session_probe_reconnection': (
                    u"Unauthorized Session Probe reconnection is detected."
                ),
            }
        }

    def __getitem__(self, para):
        return self.conf[para]

    def get(self, para, default=None):
        return self.conf.get(para, None)
# END CLASS - sesmanConfigUnicode


SESMANCONF = SesmanConfig()
