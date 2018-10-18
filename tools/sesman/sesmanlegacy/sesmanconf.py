#!/usr/bin/python
# -*- coding: utf-8 -*-

# from fileconf import parse_conf_file

def TR(key):
    from logger import Logger
    originalkey = key
    key.replace(" %s", "")
    #Logger().warning(u"Looking for translation for key '%s' (in '%s')" % (key, SESMANCONF.language))
    trans = SESMANCONF.get(SESMANCONF.language)
    if trans is None:
        Logger().warning(u"No translation available for language '%s', falling back to english" % SESMANCONF.language)
        trans = SESMANCONF.get('en')
    message = trans.get(originalkey)
    if message is None:
        Logger().warning(u"No '%s' translation available for key '%s' using key as message" % (SESMANCONF.language, key))
        message = originalkey
    if originalkey.count(u'%s') != message.count(u'%s'):
        Logger().warning(u"Variable parameters mismatch in '%s' message for key '%s' => '%s'" % (SESMANCONF.language, originalkey, message))
    return message


################################################################################
class SesmanConfig(object):
################################################################################

    # __INIT__
    #===============================================================================
    def __init__(self):
    #===============================================================================
        self.language = 'en'
        self.conf = {
          u'sesman' :
            { u'record_warning'                  : False
            , u'debug'                           : False
            , u'auth_mode_passthrough'           : False
            , u'default_login'                   : u''
            , u'selector_filters_case_sensitive' : False
            }
        , u'fr':
            { # u'login'                  : u"Login"
            # , u'target'                 : u"Compte cible"
            # , u'password'               : u"Mot de passe"
            # , u'diagnostic'             : u"Diagnostic"
            # , u'connection_closed'      : u"Connexion fermée"
            # , u'OK'                     : u"OK"
            # , u'cancel'                 : u"Abandon"
            # , u'help'                   : u"Aide"
            # , u'close'                  : u"Fermer"
            # , u'refused'                : u"Refuser"
            # , u'username'               : u"Nom de l'utilisateur"
            # , u'password_expire'        : u"Votre mot de passe va bientôt expirer. Veuillez le changer."
            # , u'too_many_connection'    : u"Trop de connexions (par rapport à votre licence)"
            # , u'header_error'           : u"Erreur dans les données d'entêtes reçus du proxy"
            # , u'unexpected_error'       : u"Erreur inattendue"
            # , u'unexpected_error_key %s': u"Erreur inattendue sur la clef %s"
            # , u'session_recorded'       : u"Attention! Votre session va être enregistrée dans un format électronique."
            # , u'failed_select_target %s': u"Impossible d'obtenir le protocole cible pour %s"
            # , u'message_not_validated'  : u"Message non validé"
            # , u'password_not_specified' : u"Mot de passe non spécifié"
            # , u'connection_refused'     : u"Connexion refusée"
            # , u'authorisation_failed %s': u"Impossible d'obtenir les autorisations pour %s"
            # , u'unknown_login %s'       : u"Identifiant ou équipement inconnu pour %s"
            # , u'ACL_error %s'           : u"Erreur ACL inattendue %s"
            # , u'help_message'           :
            #       u"Dans la zone de saisie login, entrez:<br>"
            #      +u"- le login sur la cible et le nom de la machine cible suivi<br>"
            #      +u"  du compte Bastion sous la forme login@serveur:service:WABlogin<br>"
            #      +u"- ou un nom de compte Bastion valide.<br>"
            #      +u"<br>"
            #      +u"Dans la zone de saisie mot de passe,<br>"
            #      +u"entrez le mot de passe du compte.<br>"
            #      +u"<br>"
            #      +u"Les deux champs sont sensibles à la casse.<br>"
            #      +u"<br>"
            #      +u"Contactez votre administrateur système en<br>"
            #      +u"cas de problème pour vous connecter."
                               #  u"Entrez le nom de la mache cible et du compte de la forme login@serveur<br>"
                               # +u"Entrez un nom de compte valide<br>dans la zone de saisie Nom de l'utilisateur.<br>"
                               # +u"Entrez le mot de passe dans la zone de saisie mot de passe.<br>"
                               # +u"Le nom de l'utilisateur et le mot de passe sont sensible à la case.<br>"
                               # +u"Contactez votre administrateur système en cas de problème pour vous connecter."
            # , u'forceChangePwd'         : u"Vous devez changer votre mot de passe"
            # , u'manager_close_cnx': u"Le gestionnaire de session a coupé la connexion"
            # , u"Error while retreiving rights for user %s": u"Erreur de récupération des droits de l'utilisateur %s"
            # , u"Failed to get authorisations for %s":  u"Erreur de récupération des autorisations de %s"
            # , u'Unknown proxy type': u'Type de proxy inconnu'
              u'session_closed_at %s'   : u"Votre session sera fermée à %s."
            , u'licence_blocker'        : u"Connexion refusée (par rapport à votre licence)"
            , u'auth_failed_wab %s'     : u"Echec de l'authentification pour l'utilisateur du wab %s"
            , u'error_getting_record_path'   : u"Impossible d'obtenir le répertoire d'enregistrement"
            , u'error_getting_record_path %s': u"Impossible d'obtenir le répertoire d'enregistrement %s"
            , u'error_getting_metrics_path'   : u"Impossible d'obtenir le répertoire d'enregistrement des logs"
            , u'error_getting_metrics_path %s': u"Impossible d'obtenir le répertoire d'enregistrement des logs %s"
            , u'valid_authorisation'    : u"Veuillez valider l'autorisation sur votre navigateur"
            , u'not_display_message'    : u"Impossible d'afficher le message"
            , u'changepassword'         : u"Vous devez changer votre mot de passe<br>"
                                         +u"pour vous connecter."

            , u"Connection closed by client": u'Connection terminée par le client'
            , u"Empty user, try again"  : u"Utilisteur non défini, reessayez."
            , u"Invalid user, try again": u"Utilisateur non valide, réessayez."
            , u"No error": u"Pas d'erreur"
            , u"Target %s not found in user rights": u"Cible %s inconnu dans les droits de l'utilisateur"
            , u"Target unreachable": u"Target unreachable"
            , u"Too many login failures or selector orders, closing": u"Trop de login incorrect, fermeture"
            , u"Trace writer failed for %s": u"Echec de l'enregistrement des traces pour %s"
            , u"x509 browser authentication not validated by user": u"Authentification X509 via le navigateur non validée."
            , u'not_accept_message': u"Message de confirmation refusé."
            , u'selected_target': u'Cible sélectionnée'
            , u'Username_parse_error %s': u"Erreur de parsing du nom d'utilisateur %s"
            , u'Your password will expire in %s days. Please change it.': u'Votre mot de passe expirera dans %s jours. Vous devriez le changer.'
            , u'Your password will expire soon. Please change it.': u"L'expiration de votre mot de passe est imminente. Saisissez un nouveau mot de passe."
            , u'passthrough_auth_failed_wab %s' : u"Echec de l'authentification passthrough pour l'utilisateur %s."
            , u'in_subnet %s'           : u"dans le sous-réseau %s."
            , u'no_match_subnet %s %s'  : u"%s n'appartient pas au sous-réseau %s."
            , u'error %s'               : u'Erreur: %s'
            , u'authentication_failed' : u"Echec d'authentification."
            , u'account_locked' : u"Compte verrouillé."
            , u'start_session_failed' : u'Echec de démarage de session.'
            , u'pattern_kill' : u'Motif interdit detecté, le gestionnaire de session a coupé la connexion.'
            , u'session_probe_launch_failed' : u'Impossible de lancer Session Probe, le gestionnaire de session a coupé la connexion.'
            , u'session_probe_keepalive_missed' : u'Keepalive de Session Probe est manquant, le gestionnaire de session a coupé la connexion.'
            , u'session_probe_outbound_connection_blocking_failed' : u'Connexion sortante potentiellement indésirable detectée, le gestionnaire de session a coupé la connexion.'
            , u'session_probe_process_blocking_failed' : u'Processus potentiellement indésirable détecté, le gestionnaire de session a coupé la connexion.'
            , u'session_probe_failed_to_run_startup_application' : u"Echec de lancement d'application de démarrage, le gestionnaire de session a coupé la connexion."
            , u'session_probe_reconnection' : u"Une reconnexion de Session Probe non autorisée est détectée."
            }
        , u'en':
            { # u'login'                  : u"Login"
            # , u'target'                 : u"Target"
            # , u'password'               : u"Password"
            # , u'diagnostic'             : u"Diagnostic"
            # , u'connection_closed'      : u"Connection closed"
            # , u'OK'                     : u"OK"
            # , u'cancel'                 : u"Cancel"
            # , u'help'                   : u"Help"
            # , u'close'                  : u"Close"
            # , u'refused'                : u"Refused"
            # , u'username'               : u"username"
            # , u'password_expire'        : u"Your password will expire soon. Please change it."
            # , u'too_many_connection'    : u"Too many simultaneous connections (due to licence limitation)"
            # , u'header_error'           : u"Error receiving header data from RDP proxy"
            # , u'unexpected_error'       : u"Unexpected error"
            # , u'unexpected_error_key %s': u"Unexpected error %s checking keys"
            # , u'session_recorded'       : u"Warning! Your remote session will be recorded and kept in electronic format."
            # , u'failed_select_target %s': u"Failed to select target protocol for %s"
            # , u'message_not_validated'  : u"Message not validated"
            # , u'password_not_specified' : u"password is not specified"
            # , u'connection_refused'     : u"Connection refused"
            # , u'authorisation_failed %s': u"Failed to get authorisations for %s"
            # , u'unknown_login %s'       : u"Unknown login or device for %s"
            # , u'ACL_error %s'           : u"Unexpected ACL error for %s"
            # , u'help_message'           :
            #       u"In login edit box, enter:<br>"
            #      +u"- target device and login with the wab login separated<br>"
            #      +u"  by a semi colon as login@target:service:WABlogin<br>"
            #      +u"- or a valid Bastion authentication user.<br>"
            #      +u"<br>"
            #      +u"In password edit box enter your password<br>"
            #      +u"for user.<br>"
            #      +u"<br>"
            #      +u"Both fields are case sensitive.<br>"
            #      +u"<br>"
            #      +u"Contact your system administrator if you are<br>"
            #      +u"experiencing problems."
                                #  u"Enter target device and login as<br>"
                                # +u"login@server:wabuser.<br>"
                                # +u"Enter a valid authentication user in the login<br>"
                                # +u"edit box.<br>"
                                # +u"Enter the password in the password edit box.<br>"
                                # +u"<br>"
                                # +u"Both the username and password are case<br>"
                                # +u"sensitive.<br>"
                                # +u"<br>"
                                # +u"Contact your system administrator if you are<br>"
                                # +u"experiencing problems logging on."
            # , u'forceChangePwd'         : u"You must change your password"
            # , u'manager_close_cnx': u"Connection closed by manager"
            # , u"Error while retreiving rights for user %s": u"Error while retreiving rights for user %s"
            # , u"Failed to get authorisations for %s":  u"Failed to get authorisations for %s"
            # , u'Unknown proxy type': u'Unknown proxy type'
              u'session_closed_at %s'   : u"Your session will close at %s."
            , u'licence_blocker'        : u"Connection rejected (due to licence limitation)"
            , u'auth_failed_wab %s'     : u"Authentication failed for wab user %s"
            , u'error_getting_record_path'   : u"Failed to get recording path"
            , u'error_getting_record_path %s': u"Failed to get recording path for %s"
            , u'error_getting_metrics_path'   : u"Failed to get log metrics recording path"
            , u'error_getting_metrics_path %s': u"Failed to get log metrics recording path for %s"
            , u'valid_authorisation'    : u"Validate your authorisation on your browser"
            , u'not_display_message'    : u"Unable to display message"
            , u'changepassword'         : u"You must change your password to login"
            , u"Connection closed by client": u"Connection closed by client"
            , u"Empty user, try again"  : u"Empty user, try again"
            , u"Invalid user, try again": u"Invalid user, try again"
            , u"No error": u"No error"
            , u"Target %s not found in user rights": u"Target %s not found in user rights"
            , u"Target unreachable": u"Target unreachable"
            , u"Too many login failures or selector orders, closing": u"Too many login failures or selector orders, closing"
            , u"Trace writer failed for %s": u"Trace writer failed for %s"
            , u"x509 browser authentication not validated by user": u"x509 browser authentication not validated by user"
            , u'not_accept_message': u'Confirmation message refused.'
            , u'selected_target': u'Selected target'
            , u'Username_parse_error %s': u'Username_parse_error %s'
            , u'Your password will expire in %s days. Please change it.': u'Your password will expire in %s days. Please change it.'
            , u'Your password will expire soon. Please change it.': u'Your password will expire soon. Please change it.'
            , u'passthrough_auth_failed_wab %s' : u'Passthrough authentication failed for user %s.'
            , u'in_subnet %s'           : u"in %s subnet."
            , u'no_match_subnet %s %s'  : u"%s not in %s subnet."
            , u'error %s'               : u'Error: %s'
            , u'authentication_failed' : u'Authentication failed.'
            , u'account_locked' : u"Account locked."
            , u'start_session_failed' : u'Failed to start session.'
            , u'pattern_kill' : u'Forbidden pattern detected, connection closed by manager.'
            , u'session_probe_launch_failed' : u'Could not launch Session Probe, connection closed by manager.'
            , u'session_probe_keepalive_missed' : u'Session Probe keepalive missed, connection closed by manager.'
            , u'session_probe_outbound_connection_blocking_failed' : u'Potentially undesirable outbound connection detected, connection closed by manager.'
            , u'session_probe_process_blocking_failed' : u'Potentially undesirable process detected, connection closed by manager.'
            , u'session_probe_failed_to_run_startup_application' : u'Startup application launch failed, connection closed by manager.'
            , u'session_probe_reconnection' : u"Unauthorized Session Probe reconnection is detected."
            }
        }

        # parse_conf_file(self.conf, "/opt/wab/share/sesman/config/sesman.conf")


    def __getitem__(self, para):
        return self.conf[para]

    def get(self, para, default = None):
        return self.conf.get(para, None)

# END CLASS - sesmanConfigUnicode

SESMANCONF = SesmanConfig()
