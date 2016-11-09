from logger import Logger

from ..common.utils import mdecode
import traceback

from sshng.wab.engine import APPROVAL_ACCEPTED, APPROVAL_REJECTED, APPROVAL_PENDING, APPROVAL_NONE, APPREQ_REQUIRED

from os import getpid

SELECTOR_ZERO = 0
SELECTOR_ONE = 1
SELECTOR_TWO = 2
SELECTOR_NO_TARGET = 3
SELECTOR_APPROVAL = 4

class SSHSelector:
    def __init__(self, chan, targets_info, targetok, page_size=25, is_direct=False):
        self.libssh_channel = None # used to check if we can use ssh library calls on that object
        self.chan = chan
        self.targets_info = targets_info
        self.target_rights = None
        self.page_size = page_size
        self.targetok = targetok
        self.linkedchannel = None
        self.approval_status = None
        self.approval_info = None
        self.last_approval_info = None
        self.site = None
        self.form = ApprovalForm()

        self.ticket = None
        self.pid = getpid()
        self.direct = is_direct

        self.pos = 0
        self.filter_pattern = u""
        self.filter_list()
        if not len(self.targets_nr):
            self.status = 3
            # self.chan.write_stdout_server("\r\nNo reachable target\r\n")
            # self.chan.write_stdout_server("Enter h for help, ctrl-D to quit\r\n > ")
            return None

        if self.direct is True:
            self.status = 4
        elif len(self.targets_nr) == 1:
            self.direct_approval(self.targets_nr[0])
        else:
            self.show_target_list(self.filter_pattern)
        self.status = 0

    def close(self):
        return 0

    def isOpen_client(self):
        return True

    def isEof(self):
        return False

    def sendEof(self):
        pass

    def wakeup(self):
        pass

    def filter_list(self, pattern=None):
        self.targets_nr = []
        if not pattern:
            pattern = ''
        for target_tuple in self.targets_info:
            if ((u"SSH" == target_tuple.protocol)
                and not ((u"SSH_SHELL_SESSION" in target_tuple.subprotocols)
                        or (u"SSH_X11_SESSION" in target_tuple.subprotocols))):
                continue
            target_login = target_tuple.target_login
            target_device = target_tuple.target_name
            target_service = target_tuple.service_name
            # target_login, target_device, target_service = target_tuple
            target_tuple_utf8 = (target_login.encode('utf8'),
                                 target_device.encode('utf8'),
                                 target_service.encode('utf8'))
            # TODO Filter subnets
            if pattern in ("%s@%s:%s" % target_tuple_utf8):
                self.targets_nr.append(target_tuple_utf8)
                self.targets_nr = sorted(self.targets_nr, cmp=lambda a, b: cmp((a[1], a[0], a[2]), (b[1], b[0], b[2])))

    def show_target_list(self, filter_pattern):
        pattern_desc = u"" if not filter_pattern else u" [filter: '%s']" % filter_pattern
        self.chan.write_stdout_server("| ID | Site (page %d/%d)%s\r\n" % (
            self.pos + 1,
            ((len(self.targets_nr) - 1) / self.page_size) + 1,
            pattern_desc))
        self.chan.write_stdout_server("|----|-----------------------------------\r\n")
        for idx in xrange(self.pos * self.page_size,
                          min((self.pos + 1) * self.page_size, len(self.targets_nr))):
            self.chan.write_stdout_server("|%03s | %s\r\n" % (idx, ("%s@%s:%s" % self.targets_nr[idx])))
        self.chan.write_stdout_server("Enter h for help, ctrl-D to quit\r\n > ")

    def write_stderr_client(self, data):
        return None

    def write_stdout_client(self, data):
        Logger().info("@@SSHSelector.write_stdout_client")
        try:
            data_len = len(data)
            data = data.replace('\r\n', '\n').replace('\n\r', '\n').replace('\r', '\n')
            for n, c in enumerate(data):
                if c == '\x04':
                    self.chan.session.server_channels_to_close.append((self.chan, "OK"))
                    return None

                if self.status == 4:
                    ret = self.handle_approval(c, data_len)
                    if ret is not None:
                        return ret

                if self.status == 0:
                    self.inputs = ""
                    if c in ['h', '?']:
                        self.chan.write_stdout_server("\r\nPress 'n' for next page\r\n"
                                        "Press 'p' for previous page\r\n"
                                        "Press 'f <pattern>' to filter targets list\r\n"
                                        "Press 'f' to remove a filter\r\n"
                                        "Press 'q' to quit\r\n"
                                        "Press '<site_id>' to connect to a site\r\n"
                                        "Enter h for help, ctrl-D to quit\r\n"
                                        " > ")
                        break
                    elif c in ['q']:
                        self.chan.session.server_channels_to_close.append((self.chan, "OK"))
                        return None
                    elif c in ['n', '\n']:
                        self.chan.write_stdout_server("n\r\n")
                        self.pos += 1
                        if self.pos * self.page_size >= len(self.targets_nr):
                            self.pos = 0
                        self.show_target_list(self.filter_pattern)
                        break
                    elif c in ['p']:
                        self.chan.write_stdout_server("p\r\n")
                        self.pos -= 1
                        if self.pos * self.page_size < 0:
                            self.pos = (len(self.targets_nr) - 1) / self.page_size
                        self.show_target_list(self.filter_pattern)
                        break
                    elif c in ['f']:
                        self.status = 1
                    elif c in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']:
                        self.status = 2
                    else:
                        self.chan.write_stdout_server("Unknown command\r\n"
                                               "Enter h for help, ctrl-D to quit\r\n"
                                               " > ")
                        break
                # Filter or num : Lazy backspace support
                if self.status in [1, 2]:
                    if c == "\x7f":
                        self.inputs = self.inputs[:-1]
                        self.chan.write_stdout_server('\x08 \x08')
                        if not self.inputs:
                            self.status = 0

                # Entering filter
                if self.status == 1:
                    if c == '\n':
                        self.status = 0
                        try:
                            self.filter_pattern = mdecode(self.inputs[1:].strip())
                            self.pos = 0
                            self.filter_list(self.filter_pattern)
                            if self.filter_pattern:
                                if len(self.targets_nr):
                                    self.chan.write_stdout_server("\r\nFiltering result with '%s'\r\n"
                                                           % self.filter_pattern.encode('utf-8'))

                                else:
                                    self.chan.write_stdout_server(
                                        "\r\nNo device found, removing filter\r\n")
                                    self.filter_pattern = u""
                                    self.filter_list()
                            else:
                                self.chan.write_stdout_server("\r\nFilter removed\r\n")
                            self.show_target_list(self.filter_pattern)
                        except Exception, e:
                            Logger().info("Exception in filter pattern=%s" %
                                          traceback.format_exc(e))

                        break
                    else:
                        self.chan.write_stdout_server(c)
                        self.inputs += c

                if self.status == 2:
                    if c == '\n':
                        try:
                            self.site = self.targets_nr[int(self.inputs)]
                        except Exception, e:
                            self.chan.write_stdout_server("\r\nInvalid Device ID: %s\r\n" %
                                                   self.inputs)
                            self.show_target_list(self.filter_pattern)
                            self.status = 0
                            break

                        if not self.site:
                            self.chan.close()
                            return None

                        self.status = 4

                        target_login, target_device, target_service = self.site
                        self.target_rights, message = \
                            self.chan.session.wabengine_proxy.get_target_rights(
                            mdecode(target_login),
                            mdecode(target_device),
                            mdecode(target_service))
                        self.last_approval_info = None
                        self.approval_status, self.approval_info = self.check_target(
                            self.target_rights)
                        target_info = self.approval_info.get('target')
                        if not target_info:
                            target_info = "%s@%s:%s" % self.site
                        target_info = "Selected target: %s" % target_info
                        message_info = self.approval_info.get('message', '')
                        if message_info:
                            message_info = "\n%s" % message_info
                        message = "%s%s" % (target_info, message_info)
                        message = message.replace('\n', '\r\n').encode('utf-8')
                        self.chan.write_stdout_server("\r\n%s\r\n" % message)
                        if self.approval_status == APPROVAL_ACCEPTED:
                            if self.targetok(self.site):
                                return data_len
                            else:
                                self.site = None
                                self.status = 0
                                return None
                        self.chan.session.approvals.append(self)
                        self.handle_approval()
                    elif c in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']:
                        self.chan.write_stdout_server(c)
                        self.inputs += c
        except Exception, e:
            import traceback
            Logger().info("@@SSHSelector.write_stdout_client <<<%s>>>" % traceback.format_exc(e))

        Logger().info("@@SSHSelector.write_stdout_client done")
        return data_len

    def direct_approval(self, site):
        self.direct = True
        self.site = site
        self.status = 4

        target_login, target_device, target_service = self.site
        self.target_rights, message = self.chan.session.wabengine_proxy.get_target_rights(
                mdecode(target_login),
                mdecode(target_device),
                mdecode(target_service))
        if not self.target_rights:
#            return False, message
            return APPROVAL_REJECTED, message

        self.chan.session.approvals.append(self)
        self.last_approval_info = None
        self.approval_status, self.approval_info = self.check_target(self.target_rights)
        message = self.approval_info.get('message', '')
        if message:
            message = message.replace('\n', '\r\n').encode('utf-8')
            self.chan.write_stdout_server("\r\n%s\r\n" % message)
        self.handle_approval()
        return self.approval_status, message

    def handle_approval(self, c=None, data_len=0):
        if self.approval_status == APPROVAL_NONE:
            self.approval_form(c)
        else:
            self.form.reset()
            self.last_approval_info = self.approval_info
            if self.target_rights is not None:
                self.approval_status, self.approval_info = self.check_target(
                    self.target_rights)
            else:
                self.approval_status = APPROVAL_REJECTED
            if self.last_approval_info is not None and self.approval_info is not None:
                message = self.approval_info.get('message', '')
                if self.last_approval_info.get('message', '') != message:
                    message = message.replace('\n', '\r\n').encode('utf-8')
                    self.chan.write_stdout_server("\r\n%s\r\n" % message)
            if self.approval_status == APPROVAL_REJECTED:
                if self.direct:
                    self.chan.session.approvals.remove(self)
                    self.chan.session.server_channels_to_close.append((self.chan, "OK"))
                    return None
                self.chan.session.wabengine_proxy.get_proxy_rights(
                    [u'SSH', u'TELNET', u'RLOGIN'],
                    target_device=None,
                    check_timeframes=False,
                    target_context=self.chan.session.target_context)
                self.show_target_list(self.filter_pattern)
                self.status = 0
                self.pos = 0
                self.form.reset()
                self.chan.session.approvals.remove(self)
                self.site = None
                return None
            elif self.approval_status == APPROVAL_ACCEPTED:
                self.chan.session.approvals.remove(self)
                if self.chan.session.passthrough_mode is not True:
                    t_login, t_device, t_service = self.site
                    t_label = "%s@%s:%s" % (t_login, t_device, t_service or '')
                    self.chan.write_stdout_server("\r\nConnecting to %s...\r\n" % t_label)
                if self.targetok(self.site):
                    return data_len
                else:
                    self.site = None
                    self.status = 0
                    return None
            elif self.approval_status == APPROVAL_PENDING:
                if c == "\x03":
                    self.chan.session.approvals.remove(self)
                    self.chan.session.server_channels_to_close.append((self.chan, "OK"))
                    return None
        return None

    def approval_form(self, c):
        if not self.form.check():
            self.form.set_fields(self.approval_info.get("ticket_fields"))
            if not self.form.check():
                return
            self.chan.write_stdout_server("\r\n= Approval request =\r\n")
            self.chan.write_stdout_server("%s: " % self.form.get_title())
            return
        if self.form.check():
            if c == '\n':
                if self.form.is_mandatory() and (not self.form.get_param()):
                    return
                self.chan.write_stdout_server("\r\n")
                if self.form.next_key():
                    self.chan.write_stdout_server(
                        "%s: " %
                        self.form.get_title())
                else:
                    def fparam(key, value):
                        if key == "duration":
                            return self.parse_duration(value)
                        return mdecode(value)
                    ticket = dict((k, fparam(k,v)) for k, v in self.form.params.items())
                    self.last_approval_info = self.approval_info
                    self.approval_status, self.approval_info = \
                        self.check_target(self.target_rights, ticket)
                    if ((self.last_approval_info is not None) and
                        (self.approval_info is not None)):
                        message = self.approval_info.get('message', '')
                        if self.last_approval_info.get('message', '') != message:
                            message = message.replace('\n', '\r\n').encode('utf-8')
                            self.chan.write_stdout_server("\r\n%s\r\n" % message)
            elif c == "\x03":
                self.chan.session.approvals.remove(self)
                self.chan.session.server_channels_to_close.append((self.chan, "OK"))
            elif c == "\x7f":
                if self.form.del_char():
                    self.chan.write_stdout_server('\x08 \x08')
            else:
                if c is not None:
                    # Logger().info("input char : '%s'" % c.encode('hex'))
                    if self.form.set_char(c):
                        self.chan.write_stdout_server(c)

    def parse_duration(self, duration):
        if duration is not None:
            try:
                import re
                mpat = re.compile("(\d+)m")
                hpat = re.compile("(\d+)h")
                hres = hpat.search(duration)
                mres = mpat.search(duration)
                duration = 0
                if mres:
                    duration += 60 * int(mres.group(1))
                if hres:
                    duration += 60 * 60 * int(hres.group(1))
                if duration == 0:
                    duration = 3600
            except Exception:
                duration = 3600
        else:
            duration = 3600
        return duration

    def check_target(self, target, ticket=None):
        return self.chan.session.wabengine_proxy.check_target(target,
                                                              self.pid,
                                                              ticket)


class ApprovalForm(object):
    def __init__(self):
        self.titles = {'ticket':u'Ticket Ref.',
                       'duration':u'Duration',
                       'description':u'Comment'}
        self.notes = {'duration': u'([hours]h[mins]m)'}
        self.warnings = {'invalid_format':u"invalid format",
                         'toohigh':u"too high value (must be < 10000 h)"}

        self.warn = None
        self.params = {}
        self.step = -1
        self.fields = None
        self.current_key = None
        self.length = 0
        self.skip = 0
    def reset(self):
        self.params = {}
        self.step = -1
        self.fields = None
        self.current_key = None
        self.length = 0
        self.warn = None
    def set_fields(self, ticket_fields):
        if ticket_fields is not None:
            self.step = 0
            self.fields = ticket_fields
            self.length = len(self.fields)
            if self.step < self.length:
                self.current_key = self.fields.keys()[self.step]
                self.params[self.current_key] = ''
            else:
                self.reset()
        else:
            self.reset()
    def check(self):
        return (self.fields is not None) and (self.step < self.length)
    def get_title(self):
        if self.check():
            title = self.titles.get(self.current_key, "UNKNOWN FIELD")
            note = self.notes.get(self.current_key)
            if note:
                title = '%s %s' % (title, note)
            if not self.is_mandatory():
                title = "%s (optional)" % title
            if self.warn:
                warning = self.warnings.get(self.warn)
                if warning:
                    title = 'Error: %s\r\n%s' % (warning, title)
            return title
        return None
    def set_char(self, c):
        if self.skip > 0:
            self.skip -= 1
            return False
        if self.check():
            if len(self.params.get(self.current_key, '')) > 255:
                return False
            elif c == "\x1b":
                # begining of an arrow key on 3 bytes.
                self.skip = 2
                return False
            elif ord(c) < 0x20:
                # non-printable
                return False
            self.params[self.current_key] += c
            return True
        return False
    def del_unichar(self, word):
        while word and ((ord(word[-1]) & 0xC0) == 0x80):
            # byte from multibyte utf8
            word = word[:-1]
        if word and len(word) > 0:
            return word[:-1]
        return ''
    def del_char(self):
        if self.check():
            param = self.params.get(self.current_key)
            if param and len(param) > 0:
                self.params[self.current_key] = self.del_unichar(param)
                return True
        return False
    def next_key(self):
        if self.check():
            if self.current_key == "duration":
                minutes = self.check_duration(self.params[self.current_key])
                if minutes <= 0:
                    self.warn = 'invalid_format'
                    self.params[self.current_key] = ''
                    return True
                if minutes >= 600000:
                    self.warn = 'toohigh'
                    self.params[self.current_key] = ''
                    return True
            self.warn = None
            self.step += 1
            if self.step < self.length:
                self.current_key = self.fields.keys()[self.step]
                self.params[self.current_key] = ''
                return True
        return False
    def is_mandatory(self):
        if self.check():
            return (self.fields.get(self.current_key) == APPREQ_REQUIRED)
        return False
    def get_param(self):
        if self.check():
            return self.params.get(self.current_key)
        return None


    def check_duration(self, duration):
        minutes = 0
        if duration is not None:
            try:
                import re
                mpat = re.compile("(\d+)m")
                hpat = re.compile("(\d+)h")
                hres = hpat.search(duration)
                mres = mpat.search(duration)
                if mres:
                    minutes += int(mres.group(1))
                    # Logger().info("mins = %s" % int(mres.group(1)))
                if hres:
                    # Logger().info("hours = %s" % int(hres.group(1)))
                    minutes += int(hres.group(1)) * 60
            except Exception:
                minutes = 0
        # Logger().info("minutes total = %s" % minutes)
        return minutes
