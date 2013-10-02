#!/usr/bin/python

from logger import Logger

class Right(object):
    attributes = []

    def __init__(self, data, depth = 0):

        self.depth = depth
        for key, default, cls, container in self.attributes:
            try:
                if container is None:
                    item = getattr(data, key)
                    if item is None:
                        setattr(self, key, None)
                    elif issubclass(cls, Right):
                        field = cls(item, self.depth + 4)
                        setattr(self, key, field)
                    else:
                        field = cls(item)
                        setattr(self, key, field)
                else:
                    setattr(self, key, container.__class__())
                    for x in getattr(data, key):
                        if x is None:
                            getattr(self, key).append(None)
                        elif issubclass(cls, Right):
                            getattr(self, key).append(cls(x, self.depth + 4))
                        else:
                            getattr(self, key).append(cls(x))
            except AttributeError, e:
                Logger().info(u"%s %s object miss attribute %s" % (self.__class__.__name__, self, key))
                setattr(self, key, default)
            except Exception, e:
                import traceback
                Logger().info(u"%s object error when reading attribute %s <<<%s>>>" % (
                    self.__class__.__name__, key, traceback.format_exc(e)))

    def __str__(self):
        try:
            return u"<%s(%s)>" % (self.__class__.__name__, self.cn)
        except Exception, e:
            return u"<%s: %s>" % (self.__class__.__name__, id(self))


    def dump_struct(self):
        f = open("/tmp/dump_struct.txt")
        f.write("class %s(object):\n" % self.__class__.__name__)

        f.write("    def __init__(self %s ):\n" % (", ".join([key for key, default, cls, container in self.attributes])))
        for key, default, cls, container in self.attributes:
            f.write("        self.%s = %s\n" % (key, key))
        f.close()

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

#    <
#        resource=ResourceInfo(device=DeviceInfo(deviceAlias='', host='10.10.46.78', cn='win2008', uid=u'140ee23607907e970800279eed97'),
#                              application=None,
#                              service=ServiceInfo(authmechanism=<data=''>, protocol=<cn='RDP'>, cn='RDP', port='3389'))
#        auth_mode='NAM',
#        authorization=<isCritical=True, isRecorded=True>,
#        service_login=u'administrateur@qa@win2008:RDP',
#        subprotocols=[
#            SubprotocolInfo(cn='RDP', uid=u'140ed5f39235d74d0800279eed97')
#                     ],
#        deconnection_time='2099-12-30 23:59:59'
#   >,

class DeviceInfo(Right):
    attributes = [
         ('cn',                 None, u"".__class__, None)
        ,('uid',                None, u"".__class__, None)
        ,('deviceAlias',        None, u"".__class__, None)
        ,('host',               None, u"".__class__, None)
        ,('isKeyAuth',          None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)


class BlobInfo(Right):
    attributes = [
         ('data',               None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class ProtocolInfo(Right):
    attributes = [
         ('cn',               None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class SubprotocolInfo(Right):
    attributes = [
         ('cn',               None, u"".__class__, None)
        ,('uid',              None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)


class ServiceInfo(Right):
    attributes = [
         ('authmechanism',      None, BlobInfo     , None)
        ,('protocol',           None, ProtocolInfo , None)
        ,('cn',                 None, u"".__class__, None)
        ,('port',               None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class ApplicationInfo(Right):
    attributes = [
         ('cn',               None, u"".__class__, None)
        ,('uid',              None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class AppParamsInfo(Right):
    atributes = [
          ('workingdir',         None, u"".__class__, None)
        , ('program',            None, u"".__class__, None)
        , ('params',             None, u"".__class__, None)
        , ('authmechanism',      None, BlobInfo     , None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class ResourceInfo(Right):
    attributes = [
         ('device',             None, DeviceInfo   , None)
        ,('application',        None, u"".__class__, None)
        ,('service',            None, ServiceInfo  , None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class AccountInfo(Right):
    attributes = [
         ('isAgentForwardable', None, u"".__class__, None)
        ,('login',              None, u"".__class__, None)
        ,('password',           None, u"".__class__, None)
        ,('pubkey',             None, u"".__class__, None)
        ,('isKeyAuth',          None, u"".__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class GroupTargetInfo(Right):
    attributes = [(u'cn'               , None, u"".__class__, None)]
    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class AuthorizationInfo(Right):
    attributes = [
         ('isCritical',         None, True.__class__, None)
        ,('isRecorded',         None, True.__class__, None)
    ]

    def __init__(self, data, depth = 0):
        Right.__init__(self, data, depth)

class RightInfo(Right):
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

    def __init__(self, sp, depth = 0):
        Right.__init__(self, sp, depth)

class UserInfo(Right):
    attributes = [(u'cn'               , None , u"".__class__, None)
                 ,(u'preferredLanguage', u'en', u"".__class__, None)
                 ,(u'host'             , u''  , u"".__class__, None)
                 ,(u'pubkey'           , u''  , u"".__class__, None)
                 ,(u'userPassword'     , None , u"".__class__, None)
                 ]

    def __init__(self, user, depth = 0):
        Right.__init__(self, user, depth)


