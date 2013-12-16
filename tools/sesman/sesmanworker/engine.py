#!/usr/bin/python

from sesmanconf import TR

class AuthenticationFailed(Exception): pass

from model import *
from logger import Logger

from password import PASSWORD1, PASSWORD2, PASSWORD3, PASSWORD4, PASSWORD5, PASSWORD6, PASSWORD7, PASSWORD8

class Engine(object):
    config_app_params = {
        'notepaduser@NOTEPAD':
        {
            'NOTEPAD_administrateur@qa@10.10.146.78:APP':
                AppParamsInfo(
                    workingdir = u'c:\\windows',
                    program = u'c:\\windows\\system32\\notepad.exe',
                    params = None,
                    authmechanism = None
                ),
            'NOTEPAD_admin@10.10.46.78:APP':
                AppParamsInfo(
                    workingdir = u'c:\\windows',
                    program = u'c:\\windows\\system32\\notepad.exe',
                    params = None,
                    authmechanism = None
                ),
            'NOTEPAD_qa\\administrateur@10.10.46.78:APP':
                AppParamsInfo(
                    workingdir = u'c:\\windows',
                    program = u'c:\\windows\\system32\\notepad.exe',
                    params = None,
                    authmechanism = None
                )
        },
        'wablnchuser@WABLNCH':
        {
            'WABLNCH_administrateur@10.10.47.228:APP':
                AppParamsInfo(
                    workingdir = u'C:\\Users\\Administrateur\\Desktop\\redemption-wab.9.x\\tools\\browser_plugins\\WABLauncherCS\\bin\\Debug',
                    program = u'C:\\Users\\Administrateur\\Desktop\\redemption-wab.9.x\\tools\\browser_plugins\\WABLauncherCS\\bin\\Debug\\WABLauncherCS.exe /shell',
                    params = None,
                    authmechanism = None
                )
        },
    }

    config_effective_targets = {
        'NOTEPAD_administrateur@qa@10.10.146.78:APP':
            RightInfo(
                account = AccountInfo(
                    isAgentForwardable = u'False',
                    login = u'administrateur@qa',
                    password = PASSWORD1,
                    pubkey = None,
                    isKeyAuth = None
                ),
                group_targets = [],
                target_groups = u'',
                resource = ResourceInfo(
                    device = DeviceInfo(
                        cn = u'10.10.146.78',
                        uid = u'140ee23607907e970800279eed97',
                        deviceAlias = u'',
                        host = u'10.10.146.78',
                        isKeyAuth = None
                    ),
                    application = None,
                    service = ServiceInfo(
                        authmechanism = None,
                        protocol = ProtocolInfo(
                            cn = u'RDP'
                        ),
                        cn = u'RDP',
                        port = u'3389'
                    )
                ),
                auth_mode = None,
                authorization = None,
                service_login = None,
                subprotocols = [],
                deconnection_time = None
            ),
        'NOTEPAD_admin@10.10.46.78:APP':
            RightInfo(
                account = AccountInfo(
                    isAgentForwardable = u'False',
                    login = u'admin',
                    password = PASSWORD2,
                    pubkey = None,
                    isKeyAuth = None
                ),
                group_targets = [],
                target_groups = u'',
                resource = ResourceInfo(
                    device = DeviceInfo(
                        cn = u'10.10.46.78',
                        uid = u'140ee23607907e970800279eed97',
                        deviceAlias = u'',
                        host = u'10.10.46.78',
                        isKeyAuth = None
                    ),
                    application = None,
                    service = ServiceInfo(
                        authmechanism = None,
                        protocol = ProtocolInfo(
                            cn = u'RDP'
                        ),
                        cn = u'RDP',
                        port = u'3389'
                    )
                ),
                auth_mode = None,
                authorization = None,
                service_login = None,
                subprotocols = [],
                deconnection_time = None
            ),
        'NOTEPAD_qa\\administrateur@10.10.46.78:APP':
            RightInfo(
                account = AccountInfo(
                    isAgentForwardable = u'False',
                    login = u'qa\\administrateur',
                    password = PASSWORD1,
                    pubkey = None,
                    isKeyAuth = None
                ),
                group_targets = [],
                target_groups = u'',
                resource = ResourceInfo(
                    device = DeviceInfo(
                        cn = u'10.10.46.78',
                        uid = u'140ee23607907e970800279eed97',
                        deviceAlias = u'',
                        host = u'10.10.46.78',
                        isKeyAuth = None
                    ),
                    application = None,
                    service = ServiceInfo(
                        authmechanism = None,
                        protocol = ProtocolInfo(
                            cn = u'RDP'
                        ),
                        cn = u'RDP',
                        port = u'3389'
                    )
                ),
                auth_mode = None,
                authorization = None,
                service_login = None,
                subprotocols = [],
                deconnection_time = None
            ),
        'WABLNCH_administrateur@10.10.47.228:APP':
            RightInfo(
                account = AccountInfo(
                    isAgentForwardable = u'False',
                    login = u'administrateur',
                    password = PASSWORD1,
                    pubkey = None,
                    isKeyAuth = None
                ),
                group_targets = [],
                target_groups = u'',
                resource = ResourceInfo(
                    device = DeviceInfo(
                        cn = u'10.10.47.228',
                        uid = u'140ee23607907e970800279eed97',
                        deviceAlias = u'',
                        host = u'10.10.47.228',
                        isKeyAuth = None
                    ),
                    application = None,
                    service = ServiceInfo(
                        authmechanism = None,
                        protocol = ProtocolInfo(
                            cn = u'RDP'
                        ),
                        cn = u'RDP',
                        port = u'3389'
                    )
                ),
                auth_mode = None,
                authorization = None,
                service_login = None,
                subprotocols = [],
                deconnection_time = None
            )
    }   # config_effective_targets

    config_rights = {
        'itnl_internal@bouncer2:INTERNAL': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'internal',
                password = u'internal',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'bouncer'
                )
            ],
            target_groups = u'bouncer',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'bouncer2',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'bouncer2',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'INTERNAL'
                    ),
                    cn = u'INTERNAL',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'internal@bouncer2:INTERNAL',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_internal@test_card:INTERNAL': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'internal',
                password = u'internal',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'card'
                )
            ],
            target_groups = u'card',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'test_card',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'test_card',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'INTERNAL'
                    ),
                    cn = u'INTERNAL',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'internal@test_card:INTERNAL',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_internal@widget2_message:INTERNAL': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'internal',
                password = u'internal',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'message'
                )
            ],
            target_groups = u'message',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'widget2_message',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'widget2_message',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'INTERNAL'
                    ),
                    cn = u'INTERNAL',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'internal@widget2_message:INTERNAL',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_replay@autotest:INTERNAL': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'replay',
                password = u'password',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'replay'
                )
            ],
            target_groups = u'replay',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'autotest',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'autotest',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'INTERNAL'
                    ),
                    cn = u'INTERNAL',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'replay@autotest:INTERNAL',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'notepaduser@NOTEPAD:APP': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'notepaduser',
                password = u'notepaduserpass',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win2',
            resource = ResourceInfo(
                device = None,
                application = ApplicationInfo(
                    cn=u'NOTEPAD',
                    uid=u'1412c8ff638694ee0800279eed97'
                ),
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'notepaduser@NOTEPAD',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'wablnchuser@WABLNCH:APP': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'wablnchuser',
                password = u'wablnchuserpass',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win2',
            resource = ResourceInfo(
                device = None,
                application = ApplicationInfo(
                    cn=u'WABLNCH',
                    uid=u'1412c8ff638694ee0800279eed97'
                ),
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'wablnchuser@WABLNCH',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k_administrateur@10.10.46.64:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD3,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.64',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.64',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'administrateur@10.10.46.64:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k_administrateur@10.10.47.115:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.115',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.115',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'administrateur@10.10.47.115:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k3_administrateur@10.10.47.205:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.205',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.205',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'administrateur@10.10.47.205:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k3_any@10.10.46.70:VNC': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'any',
                password = PASSWORD4,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'vnc1'
                )
            ],
            target_groups = u'vnc1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.70',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.70',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'VNC'
                    ),
                    cn = u'VNC',
                    port = u'5900'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'any@10.10.46.70:VNC',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'VNC',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k3_qa\\administrateur@10.10.46.70:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'qa\\administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2;',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.70',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.70',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'qa\\administrateur@10.10.46.70:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_w2k8_qa\\administrateur@10.10.46.78:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'qa\\administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2;',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.78',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.78',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'qa\\administrateur@10.10.46.78:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_wxp_administrateur@10.10.47.175:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.175',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.175',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = True,
                isRecorded = True
            ),
            service_login = u'administrateur@10.10.47.175:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k_administrateur@10.10.46.64:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD3,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.64',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.64',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@10.10.46.64:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k_administrateur@10.10.47.115:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.115',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.115',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@10.10.47.115:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k3_administrateur@10.10.47.205:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.205',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.205',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@10.10.47.205:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k3_any@10.10.46.70:VNC': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'any',
                password = PASSWORD4,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'vnc1'
                )
            ],
            target_groups = u'vnc1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.70',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.70',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'VNC'
                    ),
                    cn = u'VNC',
                    port = u'5900'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'any@10.10.46.70:VNC',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'VNC',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_administrateur@qa@10.10.146.78:RDP': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur@qa',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.146.78',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.146.78',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@qa@10.10.146.78:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_admin@10.10.46.78:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'admin',
                password = u'BadPassword',
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.78',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.78',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'admin@10.10.46.78:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k3_qa\\administrateur@10.10.46.70:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'qa\\administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.70',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.70',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'qa\\administrateur@10.10.46.70:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_administrateur@10.10.47.228:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.228',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.228',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@10.10.47.228:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_qa\\administrateur@10.10.46.78:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'qa\\administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.78',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.78',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'qa\\administrateur@10.10.46.78:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_qa\\administrateur@10.10.46.88:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'qa\\administrateur',
                password = PASSWORD1,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                ),
                GroupTargetInfo(
                    cn = u'win2'
                )
            ],
            target_groups = u'win1;win2',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.46.88',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.88',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'qa\\administrateur@10.10.46.88:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'wxp_administrateur@10.10.47.175:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'administrateur',
                password = PASSWORD2,
                pubkey = None,
                isKeyAuth = None
            ),
            group_targets =
            [
                GroupTargetInfo(
                    cn = u'win1'
                )
            ],
            target_groups = u'win1',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.175',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.175',
                    isKeyAuth = None
                ),
                application = None,
                service = ServiceInfo(
                    authmechanism = BlobInfo(
                        data = u''
                    ),
                    protocol = ProtocolInfo(
                        cn = u'RDP'
                    ),
                    cn = u'RDP',
                    port = u'3389'
                )
            ),
            auth_mode = u'NAM',
            authorization = AuthorizationInfo(
                isCritical = False,
                isRecorded = False
            ),
            service_login = u'administrateur@10.10.47.175:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        )
    }   # config_rights

    config_service_logins = {
        'notepaduser@NOTEPAD':
        {
            'rights':
            [
                'NOTEPAD_administrateur@qa@10.10.146.78:APP',
                'NOTEPAD_admin@10.10.46.78:APP',
                'NOTEPAD_qa\\administrateur@10.10.46.78:APP'
            ]
        },
        'wablnchuser@WABLNCH':
        {
            'rights':
            [
                'WABLNCH_administrateur@10.10.47.228:APP'
            ]
        }
    }

    config_target_password = [
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.46.64',
            protocol = u'RDP',
            password = PASSWORD3
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.115',
            protocol = u'RDP',
            password = PASSWORD2
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.175',
            protocol = u'RDP',
            password = PASSWORD2
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.205',
            protocol = u'RDP',
            password = PASSWORD2
        ),
        TargetPasswordInfo(
            account = u'admin',
            resource = u'10.10.46.78',
            protocol = u'RDP',
            password = u'BadPassword'
        ),
        TargetPasswordInfo(
            account = u'administrateur@qa',
            resource = u'10.10.146.78',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'qa\\administrateur',
            resource = u'10.10.46.70',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'any',
            resource = u'10.10.46.70',
            protocol = u'VNC',
            password = PASSWORD4
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.228',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'bouncer2',
            protocol = u'INTERNAL',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'test_card',
            protocol = u'INTERNAL',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'widget2_message',
            protocol = u'INTERNAL',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'replay',
            resource = u'autotest',
            protocol = u'INTERNAL',
            password = u'password'
        ),
        TargetPasswordInfo(
            account = u'qa\\administrateur',
            resource = u'10.10.46.78',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'qa\\administrateur',
            resource = u'10.10.46.88',
            protocol = u'RDP',
            password = PASSWORD1
        ),
    ]

    config_users = {
        'a':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'apass',
            'preferredLanguage': u'en',
            'rights':
            [
                'notepaduser@NOTEPAD:APP',
                'wablnchuser@WABLNCH:APP'
            ]
        },
        'bad':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'bad',
            'preferredLanguage': u'en',
            'rights':
            [
                'w2k8_administrateur@qa@10.10.146.78:RDP',
                'w2k8_admin@10.10.46.78:RDP',
            ]
        },
        'internal':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'internalpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'itnl_internal@bouncer2:INTERNAL',
                'itnl_internal@test_card:INTERNAL',
                'itnl_internal@widget2_message:INTERNAL',
                'itnl_replay@autotest:INTERNAL',
            ]
        },
        'rec':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'recpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'rec_w2k_administrateur@10.10.46.64:RDP',
                'rec_w2k_administrateur@10.10.47.115:RDP',
                'rec_w2k3_administrateur@10.10.47.205:RDP',
                'rec_w2k3_any@10.10.46.70:VNC',
                'rec_w2k3_qa\\administrateur@10.10.46.70:RDP',
                'rec_w2k8_qa\\administrateur@10.10.46.78:RDP',
                'rec_wxp_administrateur@10.10.47.175:RDP'
            ]
        },
        'x':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'xpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'w2k_administrateur@10.10.46.64:RDP',
                'w2k_administrateur@10.10.47.115:RDP',
                'w2k3_administrateur@10.10.47.205:RDP',
                'w2k3_any@10.10.46.70:VNC',
                'w2k3_qa\\administrateur@10.10.46.70:RDP',
                'w2k8_administrateur@10.10.47.228:RDP',
                'w2k8_qa\\administrateur@10.10.46.78:RDP',
                'w2k8_qa\\administrateur@10.10.46.88:RDP',
                'wxp_administrateur@10.10.47.175:RDP'
            ]
        },
        'fr':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'password': 'frpass',
            'preferredLanguage': u'fr',
            'rights':
            [
                'w2k_administrateur@10.10.46.64:RDP',
                'w2k3_any@10.10.46.70:VNC',
                'w2k8_qa\\administrateur@10.10.46.78:RDP',
                'w2k8_qa\\administrateur@10.10.46.88:RDP'
            ]
        },
        'x509':
        {
            'is_x509_connected': True,
            'x509_authenticate': True,
            'password': 'x509pass',
            'preferredLanguage': u'en',
            'rights':
            [
                'w2k8_administrateur@qa@10.10.146.78:RDP',
                'w2k8_qa\\administrateur@10.10.46.78:RDP'
            ]
        }
    }

    def __init__(self):
        class User:
            def __init__(self, preferredLanguage = u'en'):
                self.preferredLanguage = preferredLanguage
        Logger().info("Engine constructor")
        self.wab_login = None
        self.user = User()
        self._trace_encryption = False

    def get_trace_encryption(self):
        return self._trace_encryption

    def password_expiration_date(self):
        #return false or number of days
        return False, 0

    def is_x509_connected(self, wab_login, ip_client, proxy_type):
        """
        Ask if we are authentifying using x509
        (and ask user by opening confirmation popup if we are,
        session ticket will be asked later in x509_authenticate)
        """
        print ('is_x509_connected(%s %s %s)' % (wab_login, ip_client, proxy_type))
        res = False
        try:
            res = self.config_users[wab_login]['is_x509_connected']
            if res:
                self.wab_login = wab_login
        except:
            pass
        return res

    def x509_authenticate(self):
        print ('x509_authenticate(%s)' % self.wab_login)
        try:
            if self.config_users[self.wab_login]['x509_authenticate']:
                self.user.preferredLanguage = self.config_users[self.wab_login][u'preferredLanguage']
                return True
            return False
        except:
            self.wab_login = None
        return False

    def password_authenticate(self, wab_login, ip_client, password):
        print ('password_authenticate(%s %s %s)' % (wab_login, ip_client, password))
        res = False
        try:
            print ('::::: %s' % (self.config_users[wab_login]))
            res = self.config_users[wab_login]['password'] == password
            if res:
                self.wab_login = wab_login
                self.user.preferredLanguage = self.config_users[wab_login][u'preferredLanguage']
        except:
            pass
        return res

    def get_license_status(self):
        return True

    def NotifyConnectionToCriticalEquipment(self, protocol, user, source,
            ip_source, login, device, ip, time, url):
        notif_data = {
               u'protocol' : protocol
             , u'user'     : user
             , u'source'   : source
             , u'ip_source': ip_source
             , u'login'    : login
             , u'device'   : device
             , u'ip'       : ip
             , u'time'     : time
         }

        if not (url is None):
            notif_data[u'url'] = url

        Logger().info(u"NotifyConnectionToCriticalEquipment: %r" % notif_data)

    def NotifyPrimaryConnectionFailed(self, user, ip):
        notif_data = {
               u'user' : user
             , u'ip'   : ip
         }

        Logger().info(u"NotifyPrimaryConnectionFailed: %r" % notif_data)

    def NotifySecondaryConnectionFailed(self, user, device):
        notif_data = {
               u'user'   : user
             , u'device' : device
         }

        Logger().info(u"NotifySecondaryConnectionFailed: %r" % notif_data)

    def NotifyFilesystemIsFullOrUsedAtXPercent(self, filesystem, used):
        notif_data = {
               u'filesystem' : filesystem
             , u'used'       : used
         }

        Logger().info(u"NotifyFilesystemIsFullOrUsedAtXPercent: %r" % notif_data)

    def NotifyFindPatternInRDPFlow(self, regexp, string, user_login, user, host):
        notif_data = {
               u'regexp'     : regexp
             , u'string'     : string
             , u'user_login' : user_login
             , u'user'       : user
             , u'host'       : host
         }

        Logger().info(u"NotifyFindPatternInRDPFlow: %r" % notif_data)

    def get_proxy_rights(self, protocols):
        self.rights = [ self.config_rights[r] for r in self.config_users[self.wab_login]['rights']]
        return self.rights

    def get_effective_target(self, service_login):
#        Logger().info("Effective_target %s" % service_login)
        try:
            res = [ self.config_effective_targets[r] for r in self.config_service_logins[service_login]['rights']]
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
#        Logger().info("Effective_target ok %r" % res)
        return res

    def get_app_params(self, service_login, effective_target):
        res = None
#        Logger().info("get_app_params %s" % service_login)
        try:
            for key, value in self.config_effective_targets.items():
#                Logger().info('key=%s value=%s' % (key, value))
                if value  is effective_target:
                    res = self.config_app_params[service_login][key]
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
#        Logger().info("get_app_params done = %s" % res)
        return res

    def get_target_password(self, target_device):
        res = None
        Logger().info("get_target_password ...")
        Logger().info("account =%s" % target_device.account.login)
        Logger().info("resource=%s" % target_device.resource.device.cn)
        Logger().info("protocol=%s" % target_device.resource.service.protocol.cn)
        try:
            for p in self.config_target_password:
                if (p.account == target_device.account.login and
                   p.resource == target_device.resource.device.cn and
                   p.protocol == target_device.resource.service.protocol.cn):
                    res = p.password
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
        Logger().info("get_target_password done = %s" % res)
        return res

    def release_target_password(self, target_device, reason, target_application = None):
        Logger().info("release_target_password done: target_device=\"%s\" reason=\"%s\"" %
            (target_device, reason))

    def start_session(self, target, pid):
        return "SESSIONID-0000"

    def get_restrictions(self, target):
        self.pattern_kill = u""
        self.pattern_notify = u""
        return

    def update_session(self, target31):
        pass

    def stop_session(self, result=True, diag=u"success", title=u"End session"):
        pass

    def write_trace(self, video_path):
        return True, ""
