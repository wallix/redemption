#!/usr/bin/python
# -*- coding: utf-8 -*-

from model import *

from password import PASSWORD1, PASSWORD2, PASSWORD3, PASSWORD4, PASSWORD5, PASSWORD6, PASSWORD7, PASSWORD8, PASSWORD9


class TargetConf(object):
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
        }
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
            )
    }   # config_effective_targets

    config_rights = {
        'itnl_internal@bouncer2:RDP': RightInfo(
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
            service_login = u'internal@bouncer2:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_internal@test_card:RDP': RightInfo(
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
            service_login = u'internal@test_card:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_internal@widget2_message:RDP': RightInfo(
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
            service_login = u'internal@widget2_message:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_replay@autotest:RDP': RightInfo(
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
            service_login = u'replay@autotest:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'itnl_internal@widgettest:RDP': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'internal',
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
            target_groups = u'internal',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'widgettest',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'widgettest',
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
            service_login = u'internal@widgettest:RDP',
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
        'rec_w2k_administrateur@10.10.47.39:RDP' : RightInfo(
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
                    cn = u'10.10.47.39',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.39',
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
            service_login = u'administrateur@10.10.47.39:RDP',
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
        'rec_w2k8_administrateur@10.10.47.89:RDP' : RightInfo(
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
                    cn = u'10.10.47.89',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.89',
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
            service_login = u'administrateur@10.10.47.89:RDP',
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
        'rec_w2k12_administrateur@10.10.47.132:RDP' : RightInfo(
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
                    cn = u'win2012'
                )
            ],
            target_groups = u'win2012',
            resource = ResourceInfo(
                device = DeviceInfo(
                    cn = u'10.10.47.132',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.132',
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
            service_login = u'administrateur@10.10.47.132:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'rec_x@10.10.47.179:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'x',
                password = u'x',
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
                    cn = u'10.10.47.179',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.179',
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
                isRecorded = True
            ),
            service_login = u'x@10.10.47.179:RDP',
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
        'w2k_administrateur@10.10.47.39:RDP' : RightInfo(
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
                    cn = u'10.10.47.39',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.39',
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
            service_login = u'administrateur@10.10.47.39:RDP',
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
        'w2k3_any@10.10.42.13:VNC': RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'any',
                password = u'linux',
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
                    cn = u'10.10.42.13',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.42.13',
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
            service_login = u'any@10.10.42.13:VNC',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'VNC',
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
        'w2k8_administrateur@10.10.47.89:RDP' : RightInfo(
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
                    cn = u'10.10.47.89',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.89',
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
            service_login = u'administrateur@10.10.47.89:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8_administrateur@10.10.47.63:RDP' : RightInfo(
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
                    cn = u'10.10.47.63',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.63',
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
            service_login = u'administrateur@10.10.47.63:RDP',
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
        ),
        'x@10.10.47.179:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'x',
                password = u'x',
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
                    cn = u'10.10.47.179',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.47.179',
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
            service_login = u'x@10.10.47.179:RDP',
            subprotocols =
            [
                SubprotocolInfo(
                    cn = u'RDP',
                    uid = u'140ed5f39235d74d0800279eed97'
                )
            ],
            deconnection_time = u'2099-12-30 23:59:59'
        ),
        'w2k8erpm\\special@10.10.46.110:RDP' : RightInfo(
            account = AccountInfo(
                isAgentForwardable = u'0',
                login = u'QA\\Trâçàbïlîtôùñÿê',
                password = PASSWORD9,
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
                    cn = u'10.10.46.110',
                    uid = u'140ee23607907e970800279eed97',
                    deviceAlias = u'',
                    host = u'10.10.46.110',
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
            service_login = u'QA\\Trâçàbïlîtôùñÿê@10.10.46.110:RDP',
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
        }
    }

    config_target_password = [
        TargetPasswordInfo(
            account = u'x',
            resource = u'10.10.47.179',
            protocol = u'RDP',
            password = u'x'
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.63',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.46.64',
            protocol = u'RDP',
            password = PASSWORD3
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.39',
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
            account = u'administrateur',
            resource = u'10.10.47.89',
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
            resource = u'10.10.42.13',
            protocol = u'VNC',
            password = u'linux'
        ),
        TargetPasswordInfo(
            account = u'any',
            resource = u'10.10.46.70',
            protocol = u'VNC',
            password = PASSWORD4
        ),
        TargetPasswordInfo(
            account = u'administrateur',
            resource = u'10.10.47.89',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'bouncer2',
            protocol = u'RDP',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'test_card',
            protocol = u'RDP',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'widget2_message',
            protocol = u'RDP',
            password = u'internal'
        ),
        TargetPasswordInfo(
            account = u'replay',
            resource = u'autotest',
            protocol = u'RDP',
            password = u'password'
        ),
        TargetPasswordInfo(
            account = u'internal',
            resource = u'widgettest',
            protocol = u'RDP',
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
        TargetPasswordInfo(
            account = u'QA\\administrateur',
            resource = u'10.10.46.110',
            protocol = u'RDP',
            password = PASSWORD1
        ),
        TargetPasswordInfo(
            account = u'QA\\Trâçàbïlîtôùñÿê',
            # account = u'QA\\noël_et_cébè_et_fran',
            resource = u'10.10.46.110',
            protocol = u'RDP',
            password = PASSWORD9
        ),
    ]

    config_users = {
        'a':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
            'password': 'apass',
            'preferredLanguage': u'en',
            'rights':
            [
                'notepaduser@NOTEPAD:APP'
            ]
        },
        'bad':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
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
            'passthrough_authenticate': False,
            'password': 'internalpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'itnl_internal@bouncer2:RDP',
                'itnl_internal@test_card:RDP',
                'itnl_internal@widget2_message:RDP',
                'itnl_replay@autotest:RDP',
                'itnl_internal@widgettest:RDP',
            ]
        },
        'rec':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
            'password': 'recpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'rec_w2k_administrateur@10.10.46.64:RDP',
                'rec_w2k_administrateur@10.10.47.39:RDP',
                'rec_w2k3_administrateur@10.10.47.205:RDP',
                'rec_w2k8_administrateur@10.10.47.89:RDP',
                'rec_w2k3_any@10.10.46.70:VNC',
                'rec_w2k3_qa\\administrateur@10.10.46.70:RDP',
                'rec_w2k8_qa\\administrateur@10.10.46.78:RDP',
                'rec_wxp_administrateur@10.10.47.175:RDP',
                'rec_w2k12_administrateur@10.10.47.132:RDP',
                'rec_x@10.10.47.179:RDP'
            ]
        },
        'x':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
            'password': 'xpass',
            'preferredLanguage': u'en',
            'rights':
            [
                'w2k_administrateur@10.10.46.64:RDP',
                'w2k_administrateur@10.10.47.39:RDP',
                'w2k3_administrateur@10.10.47.205:RDP',
                'w2k3_any@10.10.42.13:VNC',
                'w2k3_any@10.10.46.70:VNC',
                'w2k3_qa\\administrateur@10.10.46.70:RDP',
                'w2k8_administrateur@10.10.47.63:RDP',
                'w2k8_administrateur@10.10.47.89:RDP',
                'w2k8_qa\\administrateur@10.10.46.78:RDP',
                'w2k8_qa\\administrateur@10.10.46.88:RDP',
                'wxp_administrateur@10.10.47.175:RDP',
                'x@10.10.47.179:RDP',
                'w2k8erpm\\special@10.10.46.110:RDP'
            ]
        },
        'fr':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
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
        'challenge':
        {
            'is_x509_connected': False,
            'x509_authenticate': False,
            'passthrough_authenticate': False,
            'password': 'challengepass',
            'response': 'yes',
            'preferredLanguage': u'en',
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
            'passthrough_authenticate': False,
            'password': 'x509pass',
            'preferredLanguage': u'en',
            'rights':
            [
                'w2k8_administrateur@qa@10.10.146.78:RDP',
                'w2k8_qa\\administrateur@10.10.46.78:RDP'
            ]
        }
    }
