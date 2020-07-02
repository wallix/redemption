# Information Structs
class Challenge(object):
    def __init__(self, challenge_type, title, message, fields, echos,
                 username=None, challenge=None, token=None, recall=False):
        self.challenge_type = challenge_type
        self.title = title
        self.message = message
        self.token = token
        self.fields = fields
        self.echos = echos
        self.challenge = challenge
        self.username = username
        self.recall = recall


def ac_to_challenge(ac):
    """ Convert new Challenge from bastion to internal Challenge

    param challenge: Challenge from bastion
    :rtype: Challenge
    :return: a converted Challenge
    """
    auth_type = ac.get("auth_type")
    title = "= Challenge ="
    message = ""
    prompt = ac.get("prompt", "")
    echo = False
    recall = False
    if "OTP" in prompt:
        echo = True
    return Challenge(
        challenge_type=auth_type,
        title=title,
        message=message,
        fields=[prompt],
        echos=[echo],
        username=ac.get("username"),
        challenge=ac,
        token=None,
        recall=recall,
    )


def wchallenge_to_challenge(challenge):
    """ Convert Challenge from bastion to internal Challenge

    param challenge: Challenge from bastion
    param previous_token: token from previous MFA if needed
    :rtype: Challenge
    :return: a converted Challenge
    """
    return Challenge(
        challenge_type="CHALLENGE",
        title="= Challenge =",
        message="",
        fields=[challenge.message],
        echos=[challenge.promptEcho],
        username=challenge.username,
        challenge=challenge,
        token=getattr(challenge, "mfa_token", None)
    )


def mfa_to_challenge(mfa):
    """ Convert MFA from bastion to internal Challenge

    param mfa: MFA from bastion
    :rtype: Challenge
    :return: a converted Challenge
    """
    if not mfa.fields:
        return None
    message_list = []
    echos = [False for x in mfa.fields]
    fields = mfa.fields
    if hasattr(mfa, "auth_type"):
        message_list.append("Authentication type: %s" % mfa.auth_type)
    if mfa.fields[0] == "username":
        fields = fields[1:]
        echos = echos[1:]
        message_list.append("Username: %s" % mfa.username)
    message = "\n".join(message_list)
    recall = (len(fields) == 0)
    return Challenge(
        challenge_type="MFA",
        title="= MultiFactor Authentication =",
        message=message,
        fields=fields,
        echos=echos,
        username=mfa.username,
        token=mfa.token,
        recall=recall
    )


def aup_to_challenge(aup, username):
    """ Convert AuthenticationUpdatePassword from bastion
    to internal Challenge

    param aup: AuthenticationUpdatePassword from bastion
    param username: Provided username to set in challenge
    :rtype: Challenge
    :return: a converted Challenge
    """
    aup.challenge.username = username
    if aup.challenge.message_id == 1:
        message = (u"Your password has expired, "
                   u"You must change your password")
    elif aup.challenge.message_id == 2:
        message = (u"Please confirm password")
    elif aup.challenge.message_id == 3:
        message = (u"Passwords do not match")
    elif aup.challenge.message_id == 4:
        message = (u"The password does not meet the password policy "
                   u"requirements")
    elif aup.challenge.message_id == 5:
        message = (u"Your password has been reset, "
                   u"You must change your password")
    else:
        message = aup.challenge.message
    return Challenge(
        challenge_type="AUP",
        title="= Update Password =",
        message="",
        fields=[message],
        echos=[aup.challenge.promptEcho],
        username=aup.challenge.username,
        challenge=aup.challenge,
        token=getattr(aup.challenge, "mfa_token", None)
    )
