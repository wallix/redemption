#!/usr/bin/python3 -O
# -*- coding: iso-8859-1 -*-
#
# Copyright (c) 2021 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.


import os
import fcntl
import shlex
import binascii
import string
#import random
from subprocess import Popen, PIPE
from time import (
    monotonic as get_time,
    sleep,
)
try:
    from logger import Logger
except Exception:
    class Logger(object):
        def info(self, msg):
            print(msg)

        def debug(self, msg):
            print(msg)

from Crypto.PublicKey import RSA
from Crypto.Random import random

RANDOM_NAME_SIZE = 10
CONNECTION_TIMEOUT = 5


def random_name(size):
    return binascii.hexlify(os.urandom(size)).decode("ascii")


# subprocess popen
def set_non_blocking_fd(fd):
    """
    Set the file description of the given file descriptor to non-blocking.
    """
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    flags = flags | os.O_NONBLOCK
    fcntl.fcntl(fd, fcntl.F_SETFL, flags)


def popen_command(command):
    args = shlex.split(command)
    process = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE,
                    universal_newlines=True, bufsize=1)
    set_non_blocking_fd(process.stdout)
    set_non_blocking_fd(process.stderr)
    return process


def popen_sshpass_ssh(command, ssh_password_or_passphrase, use_passphrase):
    Logger().info(f"> > > > > popen_sshpass_ssh: use_passphrase={use_passphrase}")
    if use_passphrase:
        command = f"sshpass -P passphrase -e {command}"
    else:
        command = f"sshpass -e {command}"
    os.environ["SSHPASS"] = ssh_password_or_passphrase
    process = popen_command(command)
    if os.getenv("SSHPASS"):
        os.environ["SSHPASS"] = ""
        del os.environ["SSHPASS"]
    return process


# p = Popen("./a.out", stdin = PIPE, stdout = PIPE, stderr = PIPE, bufsize = 1)
# setNonBlocking(p.stdout)
# setNonBlocking(p.stderr)


def pexpect_prompt_ssh(command, ssh_password_or_passphrase, use_passphrase):
    Logger().info(f"> > > > > pexpect_prompt_ssh: use_passphrase={use_passphrase}")
    px = None
    try:
        from pexpect import spawn, popen_spawn
        px = spawn(command)
        # px = popen_spawn.PopenSpawn(command)
        if use_passphrase:
            px.expect("passphrase")
        else:
            px.expect("assword:")
        px.sendline(f"{ssh_password_or_passphrase}")
    except Exception as e:
        Logger().info(f"Error {e}")
        px = None
    return px


def expect_connection_ready(file_path):
    start_time = get_time()
    while not os.path.exists(file_path):
        sleep(0.5)
        if get_time() - start_time > CONNECTION_TIMEOUT:
            return False
    return True


def remove_file(file_path):
    try:
        os.remove(file_path)
    except Exception:
        pass


class TunnelingProcessInterface:
    def start(self):
        return False

    def pre_connect(self):
        return True

    def post_connect(self):
        return True

    def stop(self):
        pass


class TunnelingProcessSSH(TunnelingProcessInterface):
    def __init__(self, target_host, vnc_port, ssh_port,
                 ssh_login, ssh_password, ssh_key,
                 sock_path_dir=None, **kwargs):
        self.process = None
        self.sock_path = None
        self.target_host = target_host
        self.vnc_port = vnc_port
        self.ssh_port = ssh_port
        self.ssh_login = ssh_login
        self.ssh_password = ssh_password
        self.sock_path_dir = sock_path_dir or "/tmp/"
        self._use_pexpect = False
        self.kwargs = kwargs

        self.ssh_key_passphrase = ""
        self.ssh_key_private_key_filename = ""
        self.ssh_key_certificate_filename = ""

        #Logger().info(f"check_tunneling: ssh_key={ssh_key}")
        if ssh_key:
            alnum = ''.join(c for c in map(chr, range(256))
                            if c.isalnum() and c.isascii())
            passphrase = ''.join(
                random.choice(alnum) for _ in range(32))
            Logger().info(f"passphrase={passphrase}")

            private_key_filename = (
                f"/var/tmp/wab/volatile/{random_name(RANDOM_NAME_SIZE)}"
            )
            certificate_filename = private_key_filename + '.pub'

            try:
                with open(private_key_filename, 'wb') as f:
                    os.chmod(private_key_filename, 0o400)
                    Logger().info(f"> > > > > TunnelingProcessSSH: ssh_key={ssh_key}")
                    rsa_key = RSA.importKey(ssh_key[0]['private_key'])
                    pem_key = rsa_key.exportKey(passphrase=passphrase)
                    f.write(pem_key)
                    f.close()

                    with open(certificate_filename, 'wb') as f2:
                        os.chmod(certificate_filename, 0o400)
                        f2.write(ssh_key[0]['certificate'].encode())
                        f2.close()

                        self.ssh_key_passphrase = passphrase
                        self.ssh_key_private_key_filename = private_key_filename
                        self.ssh_key_certificate_filename = certificate_filename
            except Exception as e:
                os.remove(private_key_filename)
                os.remove(certificate_filename)

                raise e


    def _generate_sock_path(self):
        if self.sock_path is None:
            pid = os.getpid()
            generated_name = (
                f"vnc-{pid}-{random_name(RANDOM_NAME_SIZE)}.sock"
            )
            self.sock_path = os.path.join(self.sock_path_dir, generated_name)

    def start(self):
        self._generate_sock_path()
        self.process = ssh_tunneling_vnc(
            self.sock_path, self.target_host, self.vnc_port,
            self.ssh_port, self.ssh_login, self.ssh_password,
            self.ssh_key_private_key_filename, self.ssh_key_passphrase,
            use_pexpect=self._use_pexpect,
        )


        if self.process is not None:
            return True

        if self.ssh_key_private_key_filename:
            os.remove(self.ssh_key_private_key_filename)

        if self.ssh_key_certificate_filename:
            os.remove(self.ssh_key_certificate_filename)

        self.ssh_key_passphrase = ""
        self.ssh_key_private_key_filename = ""
        self.ssh_key_certificate_filename = ""

        return False

    def pre_connect(self):
        Logger().info("> > > > > TunnelingProcessSSH.pre_connect")
        try:
            if not self.sock_path:
                return False
            return expect_connection_ready(self.sock_path)
        finally:
            if self.ssh_key_private_key_filename:
                os.remove(self.ssh_key_private_key_filename)

            if self.ssh_key_certificate_filename:
                os.remove(self.ssh_key_certificate_filename)

            self.ssh_key_passphrase = ""
            self.ssh_key_private_key_filename = ""
            self.ssh_key_certificate_filename = ""

    def _remove_socket_file(self):
        if self.sock_path is not None:
            remove_file(self.sock_path)
            self.sock_path = None

    def post_connect(self):
        self._remove_socket_file()
        return True

    def stop(self):
        self._remove_socket_file()
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(2)
            except Exception:
                self.process.kill()
            # Logger().debug(self.process.returncode)
            self.process = None


class TunnelingProcessPEXPECTSSH(TunnelingProcessSSH):
    def __init__(self, **kwargs):
        TunnelingProcessSSH.__init__(self, **kwargs)
        self._use_pexpect = True


class TunnelingProcessPXSSH(TunnelingProcessSSH):
    def __init__(self, target_host, vnc_port, ssh_port,
                 ssh_login, ssh_password, ssh_key,
                 sock_path_dir=None, **kwargs):
        TunnelingProcessSSH.__init__(
            self, target_host, vnc_port, ssh_port,
            ssh_login, ssh_password, ssh_key, sock_path_dir,
            **kwargs
        )

    def start(self):
        Logger().info(f"> > > > > TunnelingProcessPXSSH.start: self.ssh_key_private_key_filename={self.ssh_key_private_key_filename}")
        Logger().info(f"> > > > > TunnelingProcessPXSSH.start: self.ssh_key_passphrase={self.ssh_key_passphrase}")

        self._generate_sock_path()
        self.process = pxssh_ssh_tunneling_vnc(
            self.sock_path, self.target_host, self.vnc_port,
            self.ssh_port, self.ssh_login, self.ssh_password,
            self.ssh_key_private_key_filename, self.ssh_key_passphrase
        )
        Logger().info(f"> > > > > TunnelingProcessPXSSH.start: self.process={self.process}")

        if self.process is not None:
            return True

        TunnelingProcessSSH.pre_connect(self)

        return False

    def pre_connect(self):
        # pxssh is blocking on connect
        TunnelingProcessSSH.pre_connect(self)
        return True

    def stop(self):
        self._remove_socket_file()
        if self.process:
            self.process.logout()
            self.process.terminate()
            try:
                self.process.wait()
            except Exception:
                # import traceback
                # print(traceback.format_exc())
                self.process.kill()
            # Logger().debug(self.process)
            self.process = None


def ssh_tunneling_vnc(local_usocket_name, target_host, vnc_port,
                      ssh_port, ssh_login, ssh_password,
                      ssh_private_key_filename, ssh_private_key_passphrase,
                      use_pexpect=False):
    """
    local_usocket_name :         must be absolute path
    target_host :                ssh and vnc host
    vnc_port :                   vnc port
    ssh_port :                   ssh port
    ssh_login :                  ssh login
    ssh_password :               ssh password
    ssh_private_key_filename :   ssh private key filename
    ssh_private_key_passphrase : ssh private key passphrase
    """
    Logger().info("> > > > > ssh_tunneling_vnc")
    ssh_opts = (
        "-o UserKnownHostsFile=/dev/null "
        "-o StrictHostKeyChecking=no "
        "-N "
        f"-p {ssh_port}"
    )
    use_private_key = bool(ssh_private_key_filename) and bool(ssh_private_key_passphrase)
    if use_private_key:
        tunneling_command = (
            f"ssh {ssh_opts} "
            f"-L {local_usocket_name}:localhost:{vnc_port} "
            f"-l {ssh_login} -i {ssh_private_key_filename} {target_host}"
        )
        password_or_passphrase = ssh_private_key_passphrase
    else:
        tunneling_command = (
            f"ssh {ssh_opts} "
            f"-L {local_usocket_name}:localhost:{vnc_port} "
            f"-l {ssh_login} {target_host}"
        )
        password_or_passphrase = ssh_password
    remove_file(local_usocket_name)
    Logger().debug(f"> > > > > ssh_tunneling_vnc {tunneling_command}")
    try:
        process_fn = popen_sshpass_ssh
        if use_pexpect:
            process_fn = pexpect_prompt_ssh
        process = process_fn(tunneling_command, password_or_passphrase,
            use_private_key)
    except Exception as e:
        Logger().info(f"> > > > > ssh_tunneling_vnc: Exception={e}")
        Logger().info(f"Tunneling with "
                      f"{'Pexpect' if use_pexpect else 'Popen'} "
                      f"Error {e}")
        process = None
    Logger().info("> > > > > ssh_tunneling_vnc: return")
    return process


def pxssh_ssh_tunneling_vnc(local_usocket_name, target_host, vnc_port,
                            ssh_port, ssh_login, ssh_password,
                            ssh_private_key_filename, ssh_private_key_passphrase):
    """
    local_usocket_name :         must be absolute path
    target_host :                ssh and vnc host
    vnc_port :                   vnc port
    ssh_port :                   ssh port
    ssh_login :                  ssh login
    ssh_password :               ssh password
    ssh_private_key_filename :   ssh private key filename
    ssh_private_key_passphrase : ssh private key passphrase
    """
    Logger().info("> > > > > pxssh_ssh_tunneling_vnc")
    from pexpect import pxssh
    p = None
    try:
        p = pxssh.pxssh(
            ignore_sighup=False,
            options={
                "StrictHostKeyChecking": "no",
                "UserKnownHostsFile": "/dev/null"
            }
        )
        use_private_key = bool(ssh_private_key_filename) and bool(ssh_private_key_passphrase)
        Logger().info(f"> > > > > pxssh_ssh_tunneling_vnc: use_private_key={use_private_key}")
        if not use_private_key:
            p.force_password = True
        else:
            Logger().info("> > > > > pxssh_ssh_tunneling_vnc: Do not force password")
        remove_file(local_usocket_name)
        if use_private_key:
            Logger().info(f"> > > > > pxssh_ssh_tunneling_vnc: ssh_key={ssh_private_key_filename}")
            Logger().info(f"> > > > > pxssh_ssh_tunneling_vnc: password={ssh_private_key_passphrase}")
            p.login(
                server=target_host,
                username=ssh_login,
                ssh_key=ssh_private_key_filename,
                password=ssh_private_key_passphrase,
                port=ssh_port,
                ssh_tunnels={
                    'local': [f'{local_usocket_name}:localhost:{vnc_port}']
                }
            )
        else:
            p.login(
                server=target_host,
                username=ssh_login,
                password=ssh_password,
                port=ssh_port,
                ssh_tunnels={
                    'local': [f'{local_usocket_name}:localhost:{vnc_port}']
                }
            )
    except Exception as e:
        Logger().info(f"Tunneling with PXSSH Error {e}")
        p = None
    return p


def check_tunneling(engine, opts, target_host, target_port, filebase,
                    sock_path_dir=None):
    if not opts.get("enable"):
        return None
    try:
        ssh_port = opts.get("ssh_port")
        if opts.get("tunneling_credential_source") == "static_login":
            ssh_login = opts.get("ssh_login")
            ssh_password = opts.get("ssh_password")
        else:
            ssh_login = engine.get_scenario_account_field(
                field = "login", param = opts.get("scenario_account_name"),
                force_device = True
            )
            ssh_password = engine.get_scenario_account_field(
                field = "password", param = opts.get("scenario_account_name"),
                force_device = True
            )
            ssh_key = engine.get_scenario_account_field(
                field = "ssh_key", param = opts.get("scenario_account_name"),
                force_device = True
            )

        Logger().info("> > > > > To be removed!!!")
        ssh_password = None

        tunneling_type = opts.get("tunneling_type", "pxssh")
        tunneling_class = TunnelingProcessPXSSH
        if tunneling_type == "pexpect":
            tunneling_class = TunnelingProcessPEXPECTSSH
        elif tunneling_type == "popen":
            tunneling_class = TunnelingProcessSSH
        tunneling_process = tunneling_class(
            target_host=target_host,
            vnc_port=target_port,
            ssh_port=ssh_port,
            ssh_login=ssh_login,
            ssh_password=ssh_password,
            ssh_key=ssh_key,
            sock_path_dir=sock_path_dir
        )
    except Exception:
        import traceback
        Logger().debug(traceback.format_exc())
        return None
    return tunneling_process
