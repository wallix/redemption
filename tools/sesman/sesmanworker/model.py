class Right(object):
    depth = 0
    def __repr__(self):
        prefix = " " * self.depth
        res1 = ["%s%s(" % (prefix, self.__class__.__name__)]
        res2 = []
        for key, default, cls, container in self.attributes:
            value = getattr(self, key)
            if container is None:
                res2.append("%s = %r" % (key, value))
            else:
                res3 = []
                for x in value:
                    res3.append("%r" % x)
                res2.append("%s = [\n%s        %s\n%s      ]" % (key, prefix, ("\n%s        , " % prefix).join(res3), prefix))

        return "%s(\n%s      %s\n%s    )" % (self.__class__.__name__, prefix, ("\n%s    , " % prefix).join(res2), prefix)

class BlobInfo(Right):
    depth = 16
    attributes = [
         ('data',               None, u"".__class__, None)
    ]

    def __init__(self, data):
        self.data = data

class ProtocolInfo(Right):
    depth = 16
    attributes = [
         ('cn',               None, u"".__class__, None)
    ]

    def __init__(self, cn):
        self.cn = cn

class DeviceInfo(Right):
    depth = 12
    attributes = [
         ('cn',                 None, u"".__class__, None)
        ,('uid',                None, u"".__class__, None)
        ,('deviceAlias',        None, u"".__class__, None)
        ,('host',               None, u"".__class__, None)
        ,('isKeyAuth',          None, u"".__class__, None)
    ]

    def __init__(self, cn, uid, deviceAlias, host, isKeyAuth):
        self.cn = cn
        self.uid = uid
        self.deviceAlias = deviceAlias
        self.host = host
        self.isKeyAuth = isKeyAuth

class ServiceInfo(Right):
    depth = 12
    attributes = [
         ('authmechanism',      None, BlobInfo     , None)
        ,('protocol',           None, ProtocolInfo , None)
        ,('cn',                 None, u"".__class__, None)
        ,('port',               None, u"".__class__, None)
    ]

    def __init__(self, authmechanism, protocol, cn, port):
        self.authmechanism = authmechanism
        self.protocol = protocol
        self.cn = cn
        self.port = port


class SubprotocolInfo(Right):
    depth = 8
    attributes = [
         ('cn',               None, u"".__class__, None)
        ,('uid',              None, u"".__class__, None)
    ]

    def __init__(self, cn, uid):
        self.cn = cn
        self.uid = uid

class ApplicationInfo(Right):
    attributes = [
         ('cn',               None, u"".__class__, None)
        ,('uid',              None, u"".__class__, None)
    ]

    def __init__(self, cn, uid):
        self.cn = cn
        self.uid = uid

class AppParamsInfo(Right):
    attributes = [
          ('workingdir',         None, u"".__class__, None)
        , ('program',            None, u"".__class__, None)
        , ('params',             None, u"".__class__, None)
        , ('authmechanism',      None, BlobInfo     , None)
    ]

    def __init__(self, workingdir, program, params, authmechanism):
        self.workingdir = workingdir
        self.program = program
        self.params = params
        self.authmechanism = authmechanism

class TargetPasswordInfo(Right):
    attributes = [
          ('account',            None, u"".__class__, None)
        , ('resource',           None, u"".__class__, None)
        , ('protocol',           None, u"".__class__, None)
    ]

    def __init__(self, account, resource, protocol, password):
        self.account = account
        self.resource = resource
        self.protocol = protocol
        self.password = password

class ResourceInfo(Right):
    attributes = [
         ('device',             None, DeviceInfo     , None)
        ,('application',        None, ApplicationInfo, None)
        ,('service',            None, ServiceInfo    , None)
    ]

    def __init__(self, device, application, service):
        self.device = device
        self.application = application
        self.service = service

class AccountInfo(Right):
    depth = 8
    attributes = [
         ('isAgentForwardable', None, u"".__class__, None)
        ,('login',              None, u"".__class__, None)
        ,('password',           None, u"".__class__, None)
        ,('pubkey',             None, u"".__class__, None)
        ,('isKeyAuth',          None, u"".__class__, None)
    ]

    def __init__(self, isAgentForwardable, login, password, pubkey, isKeyAuth):
        self.isAgentForwardable = isAgentForwardable
        self.login = login
        self.password = password
        self.pubkey = pubkey
        self.isKeyAuth = isKeyAuth

class GroupTargetInfo(Right):
    depth = 8
    attributes = [(u'cn'               , None, u"".__class__, None)]
    def __init__(self, cn):
        self.cn = cn

class AuthorizationInfo(Right):
    depth = 8
    attributes = [
         ('isCritical',         None, True.__class__, None)
        ,('isRecorded',         None, True.__class__, None)
    ]

    def __init__(self, isCritical, isRecorded):
        self.isCritical = isCritical
        self.isRecorded = isRecorded

class RightInfo(Right):
    depth = 4
    attributes = [(u'account'               , None, AccountInfo,       None)
                 ,(u'group_targets'         , []  , GroupTargetInfo,   [] )
                 ,(u'target_groups'         , u"" , u"".__class__,     None)
                 ,(u'resource'              , None, ResourceInfo,      None)
                 ,(u'auth_mode'             , None, u"".__class__,      None)
                 ,(u'authorization'         , None, AuthorizationInfo, None)
                 ,(u'service_login'         , None, u"".__class__,      None)
                 ,(u'subprotocols'          , []  , SubprotocolInfo,   [])
                 ,(u'deconnection_time'     , None, u"".__class__,      None)
                 ]

    def __init__(self, account, group_targets, target_groups, resource, auth_mode, authorization, service_login, subprotocols, deconnection_time):
        self.account = account
        self.group_targets = group_targets
        self.target_groups = target_groups
        self.resource = resource
        self.auth_mode = auth_mode
        self.authorization = authorization
        self.service_login = service_login
        self.subprotocols = subprotocols
        self.deconnection_time = deconnection_time

class UserInfo(Right):
    attributes = [(u'cn'               , None , u"".__class__, None)
                 ,(u'preferredLanguage', u'en', u"".__class__, None)
                 ,(u'host'             , u''  , u"".__class__, None)
                 ,(u'pubkey'           , u''  , u"".__class__, None)
                 ,(u'userPassword'     , None , u"".__class__, None)
                 ]

    def __init__(self, cn, preferredLanguage, host, pubkey, userPassword):
        self.cn = cn
        self.preferredLanguage = preferredLanguage
        self.host = host
        self.pubkey = pubkey
        self.userPassword = userPassword

if __name__ == '__main__':

    rights = [

        RightInfo(
              account = AccountInfo(
                  isAgentForwardable = u'0'
                , login = u'administrateur@qa'
                , password = u'S3cur3!1nux'
                , pubkey = None
                , isKeyAuth = None
                )
            , group_targets = [
                GroupTargetInfo(
                  cn = u'win1'
                )
              ]
            , target_groups = u'win1'
            , resource = ResourceInfo(
                  device = DeviceInfo(
                      cn = u'win2008'
                    , uid = u'140ee23607907e970800279eed97'
                    , deviceAlias = u''
                    , host = u'10.10.46.78'
                    , isKeyAuth = None
                    )
                , application = None
                , service = ServiceInfo(
                      authmechanism = BlobInfo(
                          data = u''
                        )
                    , protocol = ProtocolInfo(
                          cn = u'RDP'
                        )
                    , cn = u'RDP'
                    , port = u'3389'
                    )
                )
            , auth_mode = u'NAM'
            , authorization = AuthorizationInfo(
                  isCritical = True
                , isRecorded = True
                )
            , service_login = u'administrateur@qa@win2008:RDP'
            , subprotocols = [
                SubprotocolInfo(
                  cn = u'RDP'
                , uid = u'140ed5f39235d74d0800279eed97'
                )
              ]
            , deconnection_time = u'2099-12-30 23:59:59'
            )

        ,

        RightInfo(
              account = AccountInfo(
                  isAgentForwardable = u'0'
                , login = u'qa\\administrateur'
                , password = u'S3cur3!1nux'
                , pubkey = None
                , isKeyAuth = None
                )
            , group_targets = [
                GroupTargetInfo(
                  cn = u'win1'
                )
                , GroupTargetInfo(
                  cn = u'win2'
                )
              ]
            , target_groups = u'win1;win2'
            , resource = ResourceInfo(
                  device = DeviceInfo(
                      cn = u'win2008'
                    , uid = u'140ee23607907e970800279eed97'
                    , deviceAlias = u''
                    , host = u'10.10.46.78'
                    , isKeyAuth = None
                    )
                , application = None
                , service = ServiceInfo(
                      authmechanism = BlobInfo(
                          data = u''
                        )
                    , protocol = ProtocolInfo(
                          cn = u'RDP'
                        )
                    , cn = u'RDP'
                    , port = u'3389'
                    )
                )
            , auth_mode = u'NAM'
            , authorization = AuthorizationInfo(
                  isCritical = True
                , isRecorded = True
                )
            , service_login = u'qa\\administrateur@win2008:RDP'
            , subprotocols = [
                SubprotocolInfo(
                  cn = u'RDP'
                , uid = u'140ed5f39235d74d0800279eed97'
                )
              ]
            , deconnection_time = u'2099-12-30 23:59:59'
            )
    ]

    ui = UserInfo(
          cn = u'x'
        , preferredLanguage = u'fr'
        , host = u''
        , pubkey = None
        , userPassword = u'{CRYPT}$6$/GnRBryreTYVtw/g$A.VuiCNQMKGekkrEmolE9m/7QZSARJobnRbj0OjGuV/M2/Br4H7OOx8DnRzS8g.fY3KbfS8r9wK5xhtkoFgYC.'
        )

    import pprint

    pprint.pprint(ui)
    pprint.pprint(rights)

