# Information Structs
class Challenge(object):
    def __init__(self, challenge_type, title, message, fields, echos,
                 username=None, challenge=None, token=None, link=None,
                 timeout=None, first_password=False, recall=False):
        self.challenge_type = challenge_type
        self.title = title
        self.message = message
        self.token = token
        self.fields = fields
        self.echos = echos
        self.challenge = challenge
        self.username = username
        self.link = link
        self.timeout = timeout
        self.first_password = first_password
        self.recall = recall


def ac_to_challenge(ac, check_state=False):
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
    first_password = check_state and prompt.startswith("Password")
    return Challenge(
        challenge_type=auth_type,
        title=title,
        message=message,
        fields=[prompt],
        echos=[echo],
        username=ac.get("username"),
        challenge=ac,
        token=None,
        first_password=first_password,
        recall=recall,
    )


def ur_to_challenge(saml):
    """ Convert url redirect challenge to internal Challenge

    param challenge: URL redirect Challenge from bastion
    :rtype: Challenge
    :return: a converted Challenge
    """
    auth_type = saml.get("auth_type")
    title = "= URL REDIRECTION ="
    prompt = saml.get("prompt", "")
    url = saml.get("url", "")
    timeout = saml.get("timeout")
    recall = False
    return Challenge(
        challenge_type=auth_type,
        title=title,
        message=prompt,
        fields=[],
        echos=[],
        username=saml.get("username"),
        challenge=saml,
        token=None,
        link=url,
        timeout=timeout,
        recall=recall,
    )


def md_to_challenge(md):
    """ Convert new Challenge from bastion to internal Challenge

    param challenge: Challenge from bastion
    :rtype: Challenge
    :return: a converted Challenge
    """
    auth_type = md.get("auth_type")
    title = "= MOBILE DEVICE ="
    message = md.get("prompt", "")
    return Challenge(
        challenge_type=auth_type,
        title=title,
        message=message,
        fields=[],
        echos=[],
        username=md.get("username"),
        challenge=md,
        token=None,
        recall=True,
    )
