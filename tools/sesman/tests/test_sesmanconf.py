import unittest

from sesmanconf import TR, Sesmsg, SESMANCONF


class Test_translation(unittest.TestCase):
    def test_fr(self):
        """
        Tests some keywords are present in translated message
        """
        SESMANCONF.language = 'fr'
        self.assertIn("fermée à", TR(Sesmsg.SESSION_CLOSED_S))
        self.assertIn("licence", TR(Sesmsg.LICENCE_BLOCKER))
        self.assertIn("authentification",
                      TR(Sesmsg.AUTH_FAILED_WAB_S) % "user")
        self.assertIn("enregistrement", TR(Sesmsg.ERROR_RECORD_PATH))
        self.assertIn("répertoire", TR(Sesmsg.ERROR_RECORD_PATH_S) % "chemin")
        self.assertIn("valider", TR(Sesmsg.VALID_AUTHORISATION))
        self.assertIn("afficher", TR(Sesmsg.NOT_DISPLAY_MESSAGE))
        self.assertIn("changer votre mot de passe", TR(Sesmsg.CHANGEPASSWORD))
        self.assertIn("terminée par le client",
                      TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT))
        self.assertIn("Utilisateur", TR(Sesmsg.EMPTY_USER))
        self.assertIn("non valide", TR(Sesmsg.INVALID_USER))
        self.assertIn("erreur", TR(Sesmsg.NO_ERROR))
        self.assertIn("Cible %s inconnu", TR(Sesmsg.TARGET_S_NOT_FOUND))
        self.assertIn("cible", TR(Sesmsg.TARGET_UNREACHABLE))
        self.assertIn("échecs", TR(Sesmsg.TOO_MANY_LOGIN_FAILURES))
        self.assertIn("enregistrement", TR(Sesmsg.TRACE_WRITER_FAILED_S))
        self.assertIn("X509", TR(Sesmsg.X509_AUTH_REFUSED_BY_USER))
        self.assertIn("URL Redirection", TR(Sesmsg.URL_AUTH_REFUSED_BY_USER))
        self.assertIn("refusé", TR(Sesmsg.NOT_ACCEPT_MESSAGE))
        self.assertIn("sélectionnée", TR(Sesmsg.SELECTED_TARGET))
        self.assertIn("parsing", TR(Sesmsg.USERNAME_PARSE_ERROR_S))
        self.assertIn("expirera", TR(Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS))
        self.assertIn("imminente", TR(Sesmsg.PASSWORD_EXPIRE_SOON))
        self.assertIn("authentification passthrough",
                      TR(Sesmsg.PASSTHROUGH_AUTH_FAILED_S))
        self.assertIn("dans le sous-réseau", TR(Sesmsg.IN_SUBNET_S))
        self.assertIn("pas au sous-réseau", TR(Sesmsg.NO_MATCH_SUBNET_S_S))
        self.assertIn("Erreur", TR(Sesmsg.ERROR_S))
        self.assertIn("Echec d'authentification",
                      TR(Sesmsg.AUTHENTICATION_FAILED))
        self.assertIn("Compte verrouillé", TR(Sesmsg.ACCOUNT_LOCKED))
        self.assertIn("démarrage", TR(Sesmsg.START_SESSION_FAILED))
        self.assertIn("Motif interdit détecté", TR(Sesmsg.PATTERN_KILL))
        self.assertIn("Session Probe", TR(Sesmsg.SESPROBE_LAUNCH_FAILED))
        self.assertIn("coupé la connexion",
                      TR(Sesmsg.SESPROBE_KEEPALIVE_MISSED))
        self.assertIn("Connexion sortante",
                      TR(Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED))
        self.assertIn("Processus", TR(Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED))
        self.assertIn("Echec de lancement",
                      TR(Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION))
        self.assertIn("reconnexion de Session Probe",
                      TR(Sesmsg.SESPROBE_RECONNECTION))
        self.assertIn("erreur fatale",
                      TR(Sesmsg.APPLICATION_FATAL_ERROR))

    def test_en(self):
        """
        Tests some keywords are present in translated message
        """
        self.assertIn("close at", TR(Sesmsg.SESSION_CLOSED_S))
        self.assertIn("licence", TR(Sesmsg.LICENCE_BLOCKER))
        self.assertIn("Authentication",
                      TR(Sesmsg.AUTH_FAILED_WAB_S) % "user")
        self.assertIn("recording path", TR(Sesmsg.ERROR_RECORD_PATH))
        self.assertIn("recording path for",
                      TR(Sesmsg.ERROR_RECORD_PATH_S) % "chemin")
        self.assertIn("Validate", TR(Sesmsg.VALID_AUTHORISATION))
        self.assertIn("display message", TR(Sesmsg.NOT_DISPLAY_MESSAGE))
        self.assertIn("change your password", TR(Sesmsg.CHANGEPASSWORD))
        self.assertIn("closed by client",
                      TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT))
        self.assertIn("Empty user", TR(Sesmsg.EMPTY_USER))
        self.assertIn("Invalid user", TR(Sesmsg.INVALID_USER))
        self.assertIn("No error", TR(Sesmsg.NO_ERROR))
        self.assertIn("not found", TR(Sesmsg.TARGET_S_NOT_FOUND))
        self.assertIn("unreachable", TR(Sesmsg.TARGET_UNREACHABLE))
        self.assertIn("many login failures", TR(Sesmsg.TOO_MANY_LOGIN_FAILURES))
        self.assertIn("writer failed", TR(Sesmsg.TRACE_WRITER_FAILED_S))
        self.assertIn("X509", TR(Sesmsg.X509_AUTH_REFUSED_BY_USER))
        self.assertIn("URL Redirection", TR(Sesmsg.URL_AUTH_REFUSED_BY_USER))
        self.assertIn("refused", TR(Sesmsg.NOT_ACCEPT_MESSAGE))
        self.assertIn("Selected", TR(Sesmsg.SELECTED_TARGET))
        self.assertIn("parse error", TR(Sesmsg.USERNAME_PARSE_ERROR_S))
        self.assertIn("will expire in", TR(Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS))
        self.assertIn("expire soon", TR(Sesmsg.PASSWORD_EXPIRE_SOON))
        self.assertIn("Passthrough authentication failed",
                      TR(Sesmsg.PASSTHROUGH_AUTH_FAILED_S))
        self.assertIn("subnet", TR(Sesmsg.IN_SUBNET_S))
        self.assertIn("not in", TR(Sesmsg.NO_MATCH_SUBNET_S_S))
        self.assertIn("Error", TR(Sesmsg.ERROR_S))
        self.assertIn("Authentication failed",
                      TR(Sesmsg.AUTHENTICATION_FAILED))
        self.assertIn("Account locked", TR(Sesmsg.ACCOUNT_LOCKED))
        self.assertIn("Failed to start", TR(Sesmsg.START_SESSION_FAILED))
        self.assertIn("Forbidden pattern", TR(Sesmsg.PATTERN_KILL))
        self.assertIn("Could not launch Session Probe",
                      TR(Sesmsg.SESPROBE_LAUNCH_FAILED))
        self.assertIn("connection closed",
                      TR(Sesmsg.SESPROBE_KEEPALIVE_MISSED))
        self.assertIn("outbound connection",
                      TR(Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED))
        self.assertIn("process", TR(Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED))
        self.assertIn("application launch failed",
                      TR(Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION))
        self.assertIn("Unauthorized Session Probe reconnection",
                      TR(Sesmsg.SESPROBE_RECONNECTION))
        self.assertIn("fatal error",
                      TR(Sesmsg.APPLICATION_FATAL_ERROR))
