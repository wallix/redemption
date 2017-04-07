import os
import pty
import select

import traceback
from logger import Logger
import select
from sshng import pysshct
from sshng.pysshct import (
    lib,
    event_cb,
    CB_EVENT_FUNCS
)

class WABConsoleFD(object):
    def __init__(self, fd, console):
        self.fd = fd
        self.console = console

    def fileno(self):
        return self.fd

    def read(self):
        res = None
        try:
            res = pty._read(self.fd)
            if res.strip() in [ "exit", "quit"]:
                self.console.close_linked_channel()
#                self.fd.close()
        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
        return res

    def write(self, data):
        return pty._writen(self.fd, data)

    def close(self):
        pty._writen(self.fd, "exit\n")
        self.console.close_linked_channel()


class WABConsole(object):
    def __init__(self, session, client_channel, user, password, ip_source, ip_local):
        try:
            self.libssh_channel = None # used to check if we can use ssh library calls on that object
            self.session = session
            self.language = self.session.wabengine_proxy.get_language()
            self.closed = False
            self.pid, fd = pty.fork()
            self.fd = WABConsoleFD(fd, self)
            if not self.pid:
                os.execvp('WABConsole',
                          ['WABConsole', '-u', user,
                           '-p', password, '-i', ip_source,
                           '-s', ip_local
                           ])
                self.session.server_channels_to_close.append((client_channel, "OK"))
                return
            self.linkedchannel = client_channel

            self.callback = event_cb(CB_EVENT_FUNCS['event_cb'])
            lib.ssh_event_set_fd_client(self.session.libssh_mainloop,
                                     self.fd.fileno(),
                                     self.callback,
                                     self)

            Logger().info("sending Wallix banner")
            self.send_wallix_banner(client_channel)

        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
            client_channel.write_stderr_server("Unable to open a shell, please contact your WAB admin\n")
            client_channel.write_stderr_server("Diagnostic : %s" % exc)

    def __del__(self):
        pass

    def close_linked_channel(self):
        self.session.server_channels_to_close.append((self.linkedchannel, "OK"))

    def pollEvent(self, fd, revent):
        try:
            if(self.fd.fileno() != fd):
                raise RuntimeError("Different fd in callbacks")
            if (revent & select.POLLIN):
                self.pollInEvent()
            if (revent & select.POLLERR):
                self.pollErrEvent()
            if (revent & select.POLLOUT):
                self.pollOutEvent()
            if (revent & select.POLLHUP):
                self.close()
        except Exception, e:
            import traceback
            Logger().info("%s" % traceback.format_exc(e))
        return 0

    def pollInEvent(self):
        try:
            if self.linkedchannel is None:
                return
            try:
                data = self.fd.read()
                written = self.linkedchannel.write_stdout_server(data)
                return written
            except EOFError:
                self.session.server_channels_to_close.append((self.linkedchannel, "OK"))
                pty._writen(self.fd, "exit\n")
                self.fd.close()
        except Exception, e:
            Logger().debug("WABConsole.pollInEvent :: %s" % traceback.format_exc(e))

    def pollOutEvent(self):
        pass

    def pollErrEvent(self):
        self.session.server_channels_to_close.append((self.linkedchannel, "OK"))
        pty._writen(self.fd, "exit\n")
        self.fd.close()

    def write_stdout_client(self, data):
        data_len = 0
        try:
            data_len = len(data)
            self.fd.write(data)
        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
        return data_len

    def write_stderr_client(self, data):
        data_len = 0
        try:
            data_len = len(data)
            self.fd.write(data)
        except Exception, exc:
            Logger().info("Exception caught %s %s\n%s" % (type(exc), repr(exc), traceback.format_exc()))
        return data_len

    def close(self):
        self.fd.close()
        if self.linkedchannel and self.linkedchannel.isOpen_server():
            self.linkedchannel.close()
        self.closed = True

    def isOpen_client(self):
        return not self.closed

    def isEof(self):
        return self.closed
 
    def sendEof(self):
        self.fd.write("exit\n")
        self.fd.close()
        if self.linkedchannel and self.linkedchannel.isOpen_server():
            self.linkedchannel.close()

    def send_wallix_banner(self, channel, banner_path=None, disconnection_date=None):
        """
        Send a Wallix ASCII Art banner to the client.
        You need to use just when an shell is requested.

        @param banner_path: absolute path to wallix banner file
        @type banner_path: str or None for default banner

        @param disconnection_date: UTF formated datetime of the next automatic disconnection
        @type disconnection_date: str
        """
        
        if banner_path is None:
            banner = self.session.wabengine_proxy.get_ssh_banner(self.language)
            channel.write_stdout_server(banner)
        else:
            import os
            if os.path.exists(banner_path):
                banner = open(banner_path)
                for line in banner.readlines():
                    channel.write_stdout_server(line+'\r')
                banner.close()

        if disconnection_date is not None:
            channel.write_stdout_server(
                'Warning:    This connection will be closed at %s\r\n' % str(disconnection_date)
            )

    def wakeup(self):
        pass
