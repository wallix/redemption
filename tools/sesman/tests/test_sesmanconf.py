import unittest

from sesmanconf import TR, Sesmsg, SESMANCONF


class Test_translation(unittest.TestCase):
    def test_fr(self):
        """
        Tests some keywords are present in translated message
        """
        SESMANCONF.language = 'fr'
        assert "fermée à" in TR(Sesmsg.SESSION_CLOSED_S)
        assert "licence" in TR(Sesmsg.LICENCE_BLOCKER)
        assert "authentification" in TR(Sesmsg.AUTH_FAILED_WAB_S) % "user"
        assert "enregistrement" in TR(Sesmsg.ERROR_RECORD_PATH)
        assert "répertoire" in TR(Sesmsg.ERROR_RECORD_PATH_S) % "chemin"
        assert "valider" in TR(Sesmsg.VALID_AUTHORISATION)
        assert "afficher" in TR(Sesmsg.NOT_DISPLAY_MESSAGE)
        assert "changer votre mot de passe" in TR(Sesmsg.CHANGEPASSWORD)
        assert "terminée par le client" in TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT)
        assert "Utilisateur" in TR(Sesmsg.EMPTY_USER)
        assert "non valide" in TR(Sesmsg.INVALID_USER)
        assert "erreur" in TR(Sesmsg.NO_ERROR)
        assert "Cible %s inconnu" in TR(Sesmsg.TARGET_S_NOT_FOUND)
        assert "cible" in TR(Sesmsg.TARGET_UNREACHABLE)
        assert "échecs" in TR(Sesmsg.TOO_MANY_LOGIN_FAILURES)
        assert "enregistrement" in TR(Sesmsg.TRACE_WRITER_FAILED_S)
        assert "X509" in TR(Sesmsg.X509_AUTH_REFUSED_BY_USER)
        assert "URL Redirection" in TR(Sesmsg.URL_AUTH_REFUSED_BY_USER)
        assert "refusé" in TR(Sesmsg.NOT_ACCEPT_MESSAGE)
        assert "sélectionnée" in TR(Sesmsg.SELECTED_TARGET)
        assert "parsing" in TR(Sesmsg.USERNAME_PARSE_ERROR_S)
        assert "expirera" in TR(Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS)
        assert "imminente" in TR(Sesmsg.PASSWORD_EXPIRE_SOON)
        assert "authentification passthrough" in TR(Sesmsg.PASSTHROUGH_AUTH_FAILED_S)
        assert "dans le sous-réseau" in TR(Sesmsg.IN_SUBNET_S)
        assert "pas au sous-réseau" in TR(Sesmsg.NO_MATCH_SUBNET_S_S)
        assert "Erreur" in TR(Sesmsg.ERROR_S)
        assert "Echec d'authentification" in TR(Sesmsg.AUTHENTICATION_FAILED)
        assert "Compte verrouillé" in TR(Sesmsg.ACCOUNT_LOCKED)
        assert "démarrage" in TR(Sesmsg.START_SESSION_FAILED)
        assert "Motif interdit détecté" in TR(Sesmsg.PATTERN_KILL)
        assert "Session Probe" in TR(Sesmsg.SESPROBE_LAUNCH_FAILED)
        assert "coupé la connexion" in TR(Sesmsg.SESPROBE_KEEPALIVE_MISSED)
        assert "Connexion sortante" in TR(Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED)
        assert "Processus" in TR(Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED)
        assert "Echec de lancement" in TR(Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION)
        assert "reconnexion de Session Probe" in TR(Sesmsg.SESPROBE_RECONNECTION)
        assert "erreur fatale" in TR(Sesmsg.APPLICATION_FATAL_ERROR)

    def test_en(self):
        """
        Tests some keywords are present in translated message
        """
        assert "close at" in TR(Sesmsg.SESSION_CLOSED_S)
        assert "licence" in TR(Sesmsg.LICENCE_BLOCKER)
        assert "Authentication" in TR(Sesmsg.AUTH_FAILED_WAB_S) % "user"
        assert "recording path" in TR(Sesmsg.ERROR_RECORD_PATH)
        assert "recording path for" in TR(Sesmsg.ERROR_RECORD_PATH_S) % "chemin"
        assert "Validate" in TR(Sesmsg.VALID_AUTHORISATION)
        assert "display message" in TR(Sesmsg.NOT_DISPLAY_MESSAGE)
        assert "change your password" in TR(Sesmsg.CHANGEPASSWORD)
        assert "closed by client" in TR(Sesmsg.CONNECTION_CLOSED_BY_CLIENT)
        assert "Empty user" in TR(Sesmsg.EMPTY_USER)
        assert "Invalid user" in TR(Sesmsg.INVALID_USER)
        assert "No error" in TR(Sesmsg.NO_ERROR)
        assert "not found" in TR(Sesmsg.TARGET_S_NOT_FOUND)
        assert "unreachable" in TR(Sesmsg.TARGET_UNREACHABLE)
        assert "many login failures" in TR(Sesmsg.TOO_MANY_LOGIN_FAILURES)
        assert "writer failed" in TR(Sesmsg.TRACE_WRITER_FAILED_S)
        assert "X509" in TR(Sesmsg.X509_AUTH_REFUSED_BY_USER)
        assert "URL Redirection" in TR(Sesmsg.URL_AUTH_REFUSED_BY_USER)
        assert "refused" in TR(Sesmsg.NOT_ACCEPT_MESSAGE)
        assert "Selected" in TR(Sesmsg.SELECTED_TARGET)
        assert "parse error" in TR(Sesmsg.USERNAME_PARSE_ERROR_S)
        assert "will expire in" in TR(Sesmsg.PASSWORD_EXPIRE_IN_S_DAYS)
        assert "expire soon" in TR(Sesmsg.PASSWORD_EXPIRE_SOON)
        assert "Passthrough authentication failed" in TR(Sesmsg.PASSTHROUGH_AUTH_FAILED_S)
        assert "subnet" in TR(Sesmsg.IN_SUBNET_S)
        assert "not in" in TR(Sesmsg.NO_MATCH_SUBNET_S_S)
        assert "Error" in TR(Sesmsg.ERROR_S)
        assert "Authentication failed" in TR(Sesmsg.AUTHENTICATION_FAILED)
        assert "Account locked" in TR(Sesmsg.ACCOUNT_LOCKED)
        assert "Failed to start" in TR(Sesmsg.START_SESSION_FAILED)
        assert "Forbidden pattern" in TR(Sesmsg.PATTERN_KILL)
        assert "Could not launch Session Probe" in TR(Sesmsg.SESPROBE_LAUNCH_FAILED)
        assert "connection closed" in TR(Sesmsg.SESPROBE_KEEPALIVE_MISSED)
        assert "outbound connection" in TR(Sesmsg.SESPROBE_OUTBOUND_CONNECTION_BLOCKING_FAILED)
        assert "process" in TR(Sesmsg.SESPROBE_PROCESS_BLOCKING_FAILED)
        assert "application launch failed" in TR(Sesmsg.SESPROBE_FAILED_TO_RUN_STARTUP_APPLICATION)
        assert "Unauthorized Session Probe reconnection" in TR(Sesmsg.SESPROBE_RECONNECTION)
        assert "fatal error" in TR(Sesmsg.APPLICATION_FATAL_ERROR)
