#!/usr/bin/python
# -*- coding: utf-8 -*-

from fileconf import parse_conf_file

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

def translations():
    data = {}
    for auth_key, conf_key in (
        (u'trans_ok'               , u'OK'),
        (u'trans_cancel'           , u'cancel'),
        (u'trans_help'             , u'help'),
        (u'trans_close'            , u'close'),
        (u'trans_refused'          , u'refused'),
        (u'trans_login'            , u'login'),
        (u'trans_username'         , u'username'),
        (u'trans_password'         , u'password'),
        (u'trans_target'           , u'target'),
        (u'trans_diagnostic'       , u'diagnostic'),
        (u'trans_connection_closed', u'connection_closed'),
        (u'trans_help_message'     , u'help_message'),
        (u'trans_manager_close_cnx', u'manager_close_cnx'),
       ):
        data[auth_key] = TR(conf_key)
    return data


################################################################################
class SesmanConfig(object):
################################################################################

    # __INIT__
    #===============================================================================
    def __init__(self):
    #===============================================================================
        self.language = 'en'
        self.conf = {
          u'RDP':
            { u'clipboard'         : u'True'
            , u'device_redirection': u'True'
            , u'codec_id'          : u'flv'
            }
        , u'VNC':
            { u'clipboard'         : u'False'
            , u'device_redirection': u'False'
            , u'codec_id'          : u'flv'
            }
        , u'sesman' :
            { u'licence'            : u'localhost'
            , u'sql'                : u'localhost'
            , u'x509_authenticated' : u'localhost'
            , u'record_warning'     : u'True'
            , u'DEBUG'              : u'False'
            }
        , u'fr':
            { u'login'                  : u"Login"
            , u'target'                 : u"Compte cible"
            , u'password'               : u"Mot de passe"
            , u'diagnostic'             : u"Diagnostic"
            , u'connection_closed'      : u"Connexion fermée"
            , u'OK'                     : u"OK"
            , u'cancel'                 : u"Abandon"
            , u'help'                   : u"Aide"
            , u'close'                  : u"Fermer"
            , u'refused'                : u"Refuser"
            , u'username'               : u"Nom de l'utilisateur"
            , u'password_expire'        : u"Votre mot de passe va bientôt expirer. Veuillez le changer."
            , u'session_closed_at %s'   : u"Votre session sera fermée à %s."
            , u'too_many_connection'    : u"Trop de connexions (par rapport à votre licence)"
            , u'license_blocker'        : u"Connexion refusée (par rapport à votre licence)"
            , u'header_error'           : u"Erreur dans les données d'entêtes reçus du proxy"
            , u'unexpected_error'       : u"Erreur inattendue"
            , u'unexpected_error_key %s': u"Erreur inattendue sur la clef %s"
            , u'auth_failed_wab %s'     : u"Echec de l'authentification pour l'utilisateur du wab %s"
            , u'error_creating_record_path %s': u"Impossible d'obtenir le répertoire d'enregistrement %s"
            , u'session_recorded'       : u"Attention! Votre session va être enregistrée dans un format électronique."
            , u'failed_select_target %s': u"Impossible d'obtenir le protocole cible pour %s"
            , u'valid_authorisation'    : u"Veuillez valider l'autorisation sur votre navigateur"
            , u'message_not_validated'  : u"Message non validé"
            , u'not_display_message %s' : u"Impossible d'afficher le message %s"
            , u'password_not_specified' : u"Mot de passe non spécifié"
            , u'connection_refused'     : u"Connexion refusée"
            , u'authorisation_failed %s': u"Impossible d'obtenir les autorisations pour %s"
            , u'unknown_login %s'       : u"Identifiant ou équipement inconnu pour %s"
            , u'ACL_error %s'           : u"Erreur ACL inattendue %s"
            , u'help_message'           :
                  u"Dans la zone de saisie login, entrez:<br>"
                 +u"- le nom de la machine cible et du compte<br>"
                 +u"  sous la forme login@serveur.<br>"
                 +u"- ou un nom de compte valide.<br>"
                 +u"<br>"
                 +u"Dans la zone de saisie mot de passe,<br>"
                 +u"entrez le mot de passe du compte.<br>"
                 +u"<br>"
                 +u"Les deux champs sont sensibles à la case.<br>"
                 +u"<br>"
                 +u"Contactez votre administrateur système en<br>"
                 +u"cas de problème pour vous connecter."
                               #  u"Entrez le nom de la mache cible et du compte de la forme login@serveur<br>"
                               # +u"Entrez un nom de compte valide<br>dans la zone de saisie Nom de l'utilisateur.<br>"
                               # +u"Entrez le mot de passe dans la zone de saisie mot de passe.<br>"
                               # +u"Le nom de l'utilisateur et le mot de passe sont sensible à la case.<br>"
                               # +u"Contactez votre administrateur système en cas de problème pour vous connecter."
            , u'forceChangePwd'         : u"Vous devez changer votre mot de passe"
            , u'manager_close_cnx': u"Le gestionnaire de session a coupé la connexion"
            , u'changepassword'         : u"Vous devez changer votre mot de passe<br>"
                                         +u"pour vous connecter."

            , u"Connection closed by client": u'Connection terminée par le client'
            , u"Empty user, try again": u"Utilisteur non défini, reessayez."
            , u"Error while retreiving rights for user %s": u"Erreur de récupération des droits de l'utilisateur %s"
            , u"Failed to get authorisations for %s":  u"Erreur de récupération des autorisations de %s"
            , u"Invalid user, try again": u"Utilisateur non valide, réessayez."
            , u"No error": u"Pas d'erreur"
            , u"Target %s not found in user rights": u"Cible %s inconnu dans les droits de l'utilisateur"
            , u"Target unreachable": u"Target unreachable"
            , u"Too many login failures or selector orders, closing": u"Trop de login incorrect, fermeture"
            , u"Trace writer failed for %s": u"Echec de l'enregistrement des traces pour %s"
            , u"x509 browser authentication not validated by user": u"Authentification X509 via le navigateur non validée."
            , u'not_accept_message':u"Echec d'affichage du mot de passe de confirmation"
            , u'Unknown proxy type': u'Type de proxy inconnu'
            , u'Username_parse_error %s': u"Erreur de parsing du nom d'utilisateur %s"
            , u'Your password will expire in %s days. Please change it.': u'Votre mot de passe expirera dans %s jours. Vous devriez le changer.'
            , u'Your password will expire soon. Please change it.': u"L'expiration de votre mot de passe est imminente. Saisissez un nouveau mot de passe."
            }
        , u'en':
            { u'login'                  : u"Login"
            , u'target'                 : u"Target"
            , u'password'               : u"Password"
            , u'diagnostic'             : u"Diagnostic"
            , u'connection_closed'      : u"Connection closed"
            , u'OK'                     : u"OK"
            , u'cancel'                 : u"Cancel"
            , u'help'                   : u"Help"
            , u'close'                  : u"Close"
            , u'refused'                : u"Refused"
            , u'username'               : u"username"
            , u'password_expire'        : u"Your password will expire soon. Please change it."
            , u'session_closed_at %s'   : u"Your session will close at %s."
            , u'too_many_connection'    : u"Too many simultaneous connections (due to licence limitation)"
            , u'license_blocker'        : u"Connection rejected (due to licence limitation)"
            , u'header_error'           : u"Error receiving header data from RDP proxy"
            , u'unexpected_error'       : u"Unexpected error"
            , u'unexpected_error_key %s': u"Unexpected error %s checking keys"
            , u'auth_failed_wab %s'     : u"Authentication failed for wab user %s"
            , u'error_creating_record_path %s': u"Failed to get recording path for %s"
            , u'session_recorded'       : u"Warning! Your remote session will be recorded and kept in electronic format."
            , u'failed_select_target %s': u"Failed to select target protocol for %s"
            , u'valid_authorisation'    : u"Validate your authorisation on your browser"
            , u'message_not_validated'  : u"Message not validated"
            , u'not_display_message'    : u"Unable to display message"
            , u'password_not_specified' : u"password is not specified"
            , u'connection_refused'     : u"Connection refused"
            , u'authorisation_failed %s': u"Failed to get authorisations for %s"
            , u'unknown_login %s'       : u"Unknown login or device for %s"
            , u'ACL_error %s'           : u"Unexpected ACL error for %s"
            , u'help_message'           :
                  u"In login edit box, enter:<br>"
                 +u"- target device and login as login@target.<br>"
                 +u"- or a valid authentication user.<br>"
                 +u"<br>"
                 +u"In password edit box enter your password<br>"
                 +u"for user.<br>"
                 +u"<br>"
                 +u"Both fields are case sensitive.<br>"
                 +u"<br>"
                 +u"Contact your system administrator if you are<br>"
                 +u"experiencing problems."
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
            , u'forceChangePwd'         : u"You must change your password"
            , u'manager_close_cnx': u"Connection closed by manager"
            , u'changepassword'         : u"You must change your password to login"
            , u"Connection closed by client": u"Connection closed by client"
            , u"Empty user, try again": u"Empty user, try again"
            , u"Error while retreiving rights for user %s": u"Error while retreiving rights for user %s"
            , u"Failed to get authorisations for %s":  u"Failed to get authorisations for %s"
            , u"Invalid user, try again": u"Invalid user, try again"
            , u"No error": u"No error"
            , u"Target %s not found in user rights": u"Target %s not found in user rights"
            , u"Target unreachable": u"Target unreachable"
            , u"Too many login failures or selector orders, closing": u"Too many login failures or selector orders, closing"
            , u"Trace writer failed for %s": u"Trace writer failed for %s"
            , u"x509 browser authentication not validated by user": u"x509 browser authentication not validated by user"
            , u'not_accept_message':u'Unable to display confirmation message'
            , u'Unknown proxy type': u'Unknown proxy type'
            , u'Username_parse_error %s': u'Username_parse_error %s'
            , u'Your password will expire in %s days. Please change it.': u'Your password will expire in %s days. Please change it.'
            , u'Your password will expire soon. Please change it.': u'Your password will expire soon. Please change it.'
            }
        }

        parse_conf_file(self.conf, "/opt/wab/share/sesman/config/sesman.conf")


    def __getitem__(self, para):
        return self.conf[para]

    def get(self, para, default = None):
        return self.conf.get(para, None)

# END CLASS - sesmanConfigUnicode

SESMANCONF = SesmanConfig()

