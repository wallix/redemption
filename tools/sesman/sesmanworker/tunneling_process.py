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
import pexpect
import tempfile
from time import (
    monotonic as get_time,
    sleep,
)
from logger import Logger
from subprocess import Popen, PIPE

from Crypto.PublicKey import RSA
from Crypto.Random import get_random_bytes

from wallixconst.misc import VOLATILE_FOLDER

try:
    from wabsshkeys.utils import openssh_pkcs1_passphrase_private
except Exception:
    class OpenSSLException(Exception):
        pass

    SSH_KEYGEN = "/usr/bin/ssh-keygen"

    def openssh_pkcs1_passphrase_private(key, passphrase=None):
        """
        transform an internal representation (pem) of a private key to an openssh
        pkcs1 format with/without passphrase encryption
        :param key: a string representing the key in pkcs1 pem format
        :param passphrase: a string representing the passphrase used to cypher the
            exported key
        :return: a string representing the key in openssh pkcs1 pem format
        """
        if not passphrase:
            return key
        prk_fd, prk_path = tempfile.mkstemp(dir=VOLATILE_FOLDER)
        if not isinstance(key, bytes):
            key = key.encode('utf-8')
        os.write(prk_fd, key)
        os.close(prk_fd)
        command = "{} -f {} -p".format(SSH_KEYGEN, prk_path)
        new_passphrase = ".*Enter new passphrase.*"
        confirm_passphrase = ".*Enter same passphrase again:"
        ssh_keygen = pexpect.spawn(command)
        i = ssh_keygen.expect([new_passphrase, pexpect.EOF])
        if i == 0:
            ssh_keygen.sendline(passphrase)
        else:
            os.remove(prk_path)
            raise OpenSSLException("Error setting the passphrase for the exported private key")
        i = ssh_keygen.expect([confirm_passphrase, pexpect.EOF])
        if i == 0:
            ssh_keygen.sendline(passphrase)
        else:
            os.remove(prk_path)
            raise OpenSSLException("Error setting the passphrase for the exported private key")
        ssh_keygen.expect([pexpect.EOF])
        if ssh_keygen.isalive():
            ssh_keygen.wait()
        with open(prk_path, 'r') as f:
            pem_key = f.read()
        os.remove(prk_path)
        return pem_key


RANDOM_NAME_SIZE = 10
CONNECTION_TIMEOUT = 5
SSHPASS_COPYABLE_VAR_ENV = ('LANG', 'PATH')


def crypto_random_str_by_bytes_size(bytes_size):
    return binascii.hexlify(get_random_bytes(bytes_size)).decode("ascii")


# subprocess popen
def set_non_blocking_fd(fd):
    """
    Set the file description of the given file descriptor to non-blocking.
    """
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    flags = flags | os.O_NONBLOCK
    fcntl.fcntl(fd, fcntl.F_SETFL, flags)


def popen_command(command, env):
    args = shlex.split(command)
    process = Popen(args, stdin=PIPE, stdout=PIPE, stderr=PIPE,
                    universal_newlines=True, bufsize=1, env=env)
    set_non_blocking_fd(process.stdout)
    set_non_blocking_fd(process.stderr)
    return process


def popen_sshpass_ssh(command, ssh_password, ssh_private_key_passphrase):
    env = {k: os.environ[k] for k in SSHPASS_COPYABLE_VAR_ENV if k in os.environ}
    if ssh_private_key_passphrase:
        command = f"sshpass -P passphrase -e {command}"
        env["SSHPASS"] = ssh_private_key_passphrase
    else:
        command = f"sshpass -e {command}"
        env["SSHPASS"] = ssh_password
    return popen_command(command, env)


# p = Popen("./a.out", stdin = PIPE, stdout = PIPE, stderr = PIPE, bufsize = 1)
# setNonBlocking(p.stdout)
# setNonBlocking(p.stderr)


def pexpect_prompt_ssh(command, ssh_password, ssh_private_key_passphrase):
    px = None
    try:
        from pexpect import spawn, popen_spawn
        px = spawn(command)
        # px = popen_spawn.PopenSpawn(command)
        if ssh_private_key_passphrase:
            px.expect("passphrase")
        else:
            px.expect("assword:")
        px.sendline(f"{ssh_private_key_passphrase}"
                    if ssh_private_key_passphrase
                    else f"{ssh_password}")
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


def add_passphrase_to_private_key(private_key, passphrase):
    try:
        in_key = RSA.importKey(private_key)
        return in_key.exportKey(passphrase=passphrase).decode("utf-8")
    except Exception as e:
        return openssh_pkcs1_passphrase_private(private_key, passphrase)


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

        if ssh_key:
            private_key, _, certificate = ssh_key

            passphrase = crypto_random_str_by_bytes_size(32)

            private_key_filename = (
                VOLATILE_FOLDER +
                f"/{crypto_random_str_by_bytes_size(RANDOM_NAME_SIZE)}"
            )
            certificate_filename = private_key_filename + '.pub'

            try:
                with open(private_key_filename, 'w') as f:
                    os.chmod(private_key_filename, 0o400)
                    out_key = add_passphrase_to_private_key(
                        private_key=private_key,
                        passphrase=passphrase
                    )
                    f.write(out_key)

                    if certificate:
                        with open(certificate_filename, 'w') as f2:
                            os.chmod(certificate_filename, 0o400)
                            f2.write(certificate)

                            self.ssh_key_certificate_filename = certificate_filename

                    self.ssh_key_passphrase = passphrase
                    self.ssh_key_private_key_filename = private_key_filename
            except Exception as e:
                os.remove(private_key_filename)
                os.remove(certificate_filename)

                raise e

    def _generate_sock_path(self):
        if self.sock_path is None:
            pid = os.getpid()
            generated_name = (
                f"vnc-{pid}-"
                f"{crypto_random_str_by_bytes_size(RANDOM_NAME_SIZE)}.sock"
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

        self._remove_ssh_key_files()

        return False

    def pre_connect(self):
        try:
            if not self.sock_path:
                return False
            return expect_connection_ready(self.sock_path)
        finally:
            self._remove_ssh_key_files()

    def _remove_socket_file(self):
        if self.sock_path is not None:
            remove_file(self.sock_path)
            self.sock_path = None

    def _remove_ssh_key_files(self):
        if self.ssh_key_private_key_filename:
            os.remove(self.ssh_key_private_key_filename)

        if self.ssh_key_certificate_filename:
            os.remove(self.ssh_key_certificate_filename)

        self.ssh_key_passphrase = ""
        self.ssh_key_private_key_filename = ""
        self.ssh_key_certificate_filename = ""

    def post_connect(self):
        self._remove_socket_file()
        return True

    def stop(self):
        self._remove_socket_file()
        if self.process:
            self.process.terminate()
            try:
                self.process.wait()
            except Exception:
                self.process.kill(9)
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
        self._generate_sock_path()
        self.process = pxssh_ssh_tunneling_vnc(
            self.sock_path, self.target_host, self.vnc_port,
            self.ssh_port, self.ssh_login, self.ssh_password,
            self.ssh_key_private_key_filename, self.ssh_key_passphrase
        )

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
    ssh_opts = (
        "-o UserKnownHostsFile=/dev/null "
        "-o StrictHostKeyChecking=no "
        "-N "
        f"-p {ssh_port}"
    )

    use_private_key = (
        bool(ssh_private_key_filename) and
        bool(ssh_private_key_passphrase)
    )
    if use_private_key:
        ssh_opts += f" -i {ssh_private_key_filename}"
        ssh_opts += f" -o PubkeyAcceptedKeyTypes=+ssh-dss"

    tunneling_command = (
        f"ssh {ssh_opts} "
        f"-L {local_usocket_name}:localhost:{vnc_port} "
        f"-l {ssh_login} {target_host}"
    )
    remove_file(local_usocket_name)
    Logger().debug(f"ssh_tunneling_vnc {tunneling_command}")
    try:
        process_fn = popen_sshpass_ssh
        if use_pexpect:
            process_fn = pexpect_prompt_ssh
        process = process_fn(
            tunneling_command,
            ssh_password,
            ssh_private_key_passphrase if use_private_key else ""
        )
    except Exception as e:
        Logger().info(f"Tunneling with "
                      f"{'Pexpect' if use_pexpect else 'Popen'} "
                      f"Error {e}")
        process = None
    return process


def pxssh_ssh_tunneling_vnc(local_usocket_name, target_host, vnc_port,
                            ssh_port, ssh_login, ssh_password,
                            ssh_private_key_filename,
                            ssh_private_key_passphrase):
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
    from pexpect import pxssh
    p = None
    try:
        p = pxssh.pxssh(
            ignore_sighup=False,
            options={
                "StrictHostKeyChecking": "no",
                "UserKnownHostsFile": "/dev/null",
                "PubkeyAcceptedKeyTypes": "+ssh-dss"
            }
        )
        use_private_key = (
            bool(ssh_private_key_filename) and
            bool(ssh_private_key_passphrase)
        )
        if not use_private_key:
            p.force_password = True
        remove_file(local_usocket_name)
        p.login(
            server=target_host,
            username=ssh_login,
            ssh_key=ssh_private_key_filename if use_private_key else None,
            password=ssh_private_key_passphrase if use_private_key else ssh_password,
            port=ssh_port,
            ssh_tunnels={
                'local': [f'{local_usocket_name}:localhost:{vnc_port}']
            }
        )
    except Exception as e:
        Logger().info(f"Tunneling with PXSSH Error {e}")
        p = None
    return p


def check_tunneling(engine, opts, target_host, target_port,
                    sock_path_dir=None):
    if not opts.get("enable"):
        return None
    try:
        ssh_port = opts.get("ssh_port")
        if opts.get("tunneling_credential_source") == "static_login":
            ssh_login = opts.get("ssh_login")
            ssh_password = opts.get("ssh_password")
            ssh_key = None
        else:
            param = opts.get("scenario_account_name")
            acc_infos = engine.get_scenario_account(param=param, force_device=True)
            if acc_infos:
                ssh_login = acc_infos.get('login')
                ssh_password = acc_infos.get('password')
                ssh_key = acc_infos.get('ssh_key')
            else:
                ssh_login = param
                ssh_password = param
                ssh_key = param

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
