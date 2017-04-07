#!/usr/bin/python -O
# -*- coding: iso-8859-1 -*-
#
# Copyright (c) 2014 WALLIX, SAS. All rights reserved.
#
# Licensed computer software. Property of WALLIX.

from logger import Logger
import struct
import re
from utils import mdecode
# Up packet type
SSH_FXP_INIT = 1
SSH_FXP_VERSION = 2
SSH_FXP_OPEN = 3
SSH_FXP_CLOSE = 4
SSH_FXP_READ = 5
SSH_FXP_WRITE = 6
SSH_FXP_LSTAT = 7
SSH_FXP_FSTAT = 8
SSH_FXP_SETSTAT = 9
SSH_FXP_FSETSTAT = 10
SSH_FXP_OPENDIR = 11
SSH_FXP_READDIR = 12
SSH_FXP_REMOVE = 13
SSH_FXP_MKDIR = 14
SSH_FXP_RMDIR = 15
SSH_FXP_REALPATH = 16
SSH_FXP_STAT = 17
SSH_FXP_RENAME = 18
SSH_FXP_READLINK = 19
SSH_FXP_SYMLINK = 20
SSH_FXP_LINK = 21
SSH_FXP_BLOCK = 22
SSH_FXP_UNBLOCK = 23
# Down packet type
SSH_FXP_STATUS = 101
SSH_FXP_HANDLE = 102
SSH_FXP_DATA = 103
SSH_FXP_NAME = 104
SSH_FXP_ATTRS = 105

SSH_FXP_EXTENDED = 200
SSH_FXP_EXTENDED_REPLY = 201

SSH_FXP = {
    SSH_FXP_INIT:'SSH_FXP_INIT',
    SSH_FXP_VERSION:'SSH_FXP_VERSION',
    SSH_FXP_OPEN:'SSH_FXP_OPEN',
    SSH_FXP_CLOSE:'SSH_FXP_CLOSE',
    SSH_FXP_READ:'SSH_FXP_READ',
    SSH_FXP_WRITE:'SSH_FXP_WRITE',
    SSH_FXP_LSTAT:'SSH_FXP_LSTAT',
    SSH_FXP_FSTAT:'SSH_FXP_FSTAT',
    SSH_FXP_SETSTAT:'SSH_FXP_SETSTAT',
    SSH_FXP_FSETSTAT:'SSH_FXP_FSETSTAT',
    SSH_FXP_OPENDIR:'SSH_FXP_OPENDIR',
    SSH_FXP_READDIR:'SSH_FXP_READDIR',
    SSH_FXP_REMOVE:'SSH_FXP_REMOVE',
    SSH_FXP_MKDIR:'SSH_FXP_MKDIR',
    SSH_FXP_RMDIR:'SSH_FXP_RMDIR',
    SSH_FXP_REALPATH:'SSH_FXP_REALPATH',
    SSH_FXP_STAT:'SSH_FXP_STAT',
    SSH_FXP_RENAME:'SSH_FXP_RENAME',
    SSH_FXP_READLINK:'SSH_FXP_READLINK',
    SSH_FXP_SYMLINK:'SSH_FXP_SYMLINK',
    SSH_FXP_LINK:'SSH_FXP_LINK',
    SSH_FXP_BLOCK:'SSH_FXP_BLOCK',
    SSH_FXP_UNBLOCK:'SSH_FXP_UNBLOCK',

    SSH_FXP_STATUS:'SSH_FXP_STATUS',
    SSH_FXP_HANDLE:'SSH_FXP_HANDLE',
    SSH_FXP_DATA:'SSH_FXP_DATA',
    SSH_FXP_NAME:'SSH_FXP_NAME',
    SSH_FXP_ATTRS:'SSH_FXP_ATTRS',

    SSH_FXP_EXTENDED:'SSH_FXP_EXTENDED',
    SSH_FXP_EXTENDED_REPLY:'SSH_FXP_EXTENDED_REPLY'
}

SFTP_COMMAND = {
    # SSH_FXP_INIT:'SSH_FXP_INIT',
    # SSH_FXP_VERSION:'SSH_FXP_VERSION',
    # SSH_FXP_OPEN:'open path',
    # SSH_FXP_CLOSE:'SSH_FXP_CLOSE',
    # SSH_FXP_READ:'SSH_FXP_READ',
    # SSH_FXP_WRITE:'SSH_FXP_WRITE',
    SSH_FXP_LSTAT:'lstat',
    # SSH_FXP_FSTAT:'SSH_FXP_FSTAT',
    # SSH_FXP_SETSTAT:'SSH_FXP_SETSTAT',
    # SSH_FXP_FSETSTAT:'SSH_FXP_FSETSTAT',
    SSH_FXP_OPENDIR:'opendir',
    # SSH_FXP_READDIR:'SSH_FXP_READDIR',
    SSH_FXP_REMOVE:'remove',
    SSH_FXP_MKDIR:'mkdir',
    SSH_FXP_RMDIR:'rmdir',
    # SSH_FXP_REALPATH:'SSH_FXP_REALPATH',
    SSH_FXP_STAT:'stat',
    SSH_FXP_RENAME:'rename',
    SSH_FXP_READLINK:'readlink',
    SSH_FXP_SYMLINK:'symlink',
    SSH_FXP_LINK:'link',
    # SSH_FXP_BLOCK:'SSH_FXP_BLOCK',
    # SSH_FXP_UNBLOCK:'SSH_FXP_UNBLOCK',

    SSH_FXP_STATUS:'status',
    # SSH_FXP_HANDLE:'SSH_FXP_HANDLE',
    # SSH_FXP_DATA:'SSH_FXP_DATA',
    # SSH_FXP_NAME:'SSH_FXP_NAME',
    # SSH_FXP_ATTRS:'SSH_FXP_ATTRS',

    SSH_FXP_EXTENDED:'extended',
    # SSH_FXP_EXTENDED_REPLY:'SSH_FXP_EXTENDED_REPLY'
}
FLAG_READ = 0x01
FLAG_WRITE = 0x02
FLAG_APPEND = 0x04
FLAG_CREAT = 0x08
FLAG_TRUNC = 0x10
FLAG_EXCL = 0x20
FLAG_TEXT = 0x40

SSH_FX_OK = 0
SSH_FX_EOF = 1
SSH_FX_NO_SUCH_FILE = 2
SSH_FX_PERMISSION_DENIED = 3
SSH_FX_FAILURE = 4
SSH_FX_BAD_MESSAGE = 5
SSH_FX_NO_CONNECTION = 6
SSH_FX_CONNECTION_LOST = 7
SSH_FX_OP_UNSUPPORTED = 8

SSH_STATUS = {
    SSH_FX_OK:"Success",
    SSH_FX_EOF:"End of file",
    SSH_FX_NO_SUCH_FILE:"No such file",
    SSH_FX_PERMISSION_DENIED:"Permission denied",
    SSH_FX_FAILURE:"Failure",
    SSH_FX_BAD_MESSAGE:"Bad message",
    SSH_FX_NO_CONNECTION:"No connection",
    SSH_FX_CONNECTION_LOST:"Connection lost",
    SSH_FX_OP_UNSUPPORTED:"Operation unsupported",
}
class AnalyzerKill(Exception):
    """ Analyzer kill exception """
    def __init__(self, msg="", tosend=None):
        self.msg = msg
        self.tosend = tosend

class ProtoAnalyzer(object):
    def __init__(self, cb_record=None, cb_detect=None):
        self.cb_record = cb_record or (lambda x: None)
        self.cb_detect = cb_detect or (lambda x,y: None)
    def parse_up_data(self, data):
        pass
    def parse_down_data(self, data):
        pass

class SFTPAnalyzer(ProtoAnalyzer):
    def __init__(self, cb_record=None, cb_detect=None):
        ProtoAnalyzer.__init__(self, cb_record, cb_detect)

        self.req_table = {}
        self.handle_table = {}

        self.up_payload = 0
        self.up_data = None
        self.up_incomplete_size = None

        self.down_payload = 0
        self.down_data = None
        self.down_incomplete_size = None

        self.global_length = 0

    def build_fake_write_packet(self, reqid, handle, offset, data_length):
        packet = struct.pack('>BII', SSH_FXP_WRITE, reqid, len(handle))
        packet += handle
        packet += struct.pack('>QI', offset, data_length)
        packet += '\x00' * data_length
        packet = struct.pack('>I', len(packet)) + packet
        return packet

    def parse_up_data(self, data):
        if self.up_incomplete_size is not None:
            data = self.up_incomplete_size + data
            self.up_incomplete_size = None
        if self.up_data is not None:
            data_length = len(data)
            if self.up_payload < data_length:
                # Logger().debug("]] Add payload + %s" % self.up_payload)
                self.up_data += data[:self.up_payload]
                data = data[self.up_payload:]
                self.up_payload = 0
            else:
                # Logger().debug("]] Add payload + %s" % data_length)
                self.up_data += data
                self.up_payload -= data_length
                data = None
            if self.up_payload == 0:
                # Logger().debug("]] payload complete = %s" % len(self.up_data))
                self.consume_packet_up(self.up_data)
                self.up_data = None
            else:
                # Logger().debug("]] payload incomplete need %s" % self.up_payload)
                return
        while data and len(data) >= 4:
            packet_size, = struct.unpack(">I", data[:4])
            packet_buff = data[4:]
            buff_length = len(packet_buff)
            if buff_length < packet_size:
                self.up_data = packet_buff
                self.up_payload = packet_size - buff_length
                # Logger().debug("££ PACKET incomplete packet need %s" % self.down_payload)
                return
            data = packet_buff[packet_size:]
            packet_buff = packet_buff[:packet_size]
            # Logger().debug("££ PACKET complete %s" % packet_size)
            self.consume_packet_up(packet_buff)
        if data:
            self.up_incomplete_size = data

    def parse_down_data(self, data):
        if self.down_incomplete_size is not None:
            data = self.down_incomplete_size + data
            self.down_incomplete_size = None
        if self.down_data is not None:
            data_length = len(data)
            if self.down_payload < data_length:
                # Logger().debug("[[ Add payload + %s" % self.down_payload)
                self.down_data += data[:self.down_payload]
                data = data[self.down_payload:]
                self.down_payload = 0
            else:
                # Logger().debug("[[ Add payload + %s" % data_length)
                self.down_data += data
                self.down_payload -= data_length
                data = None
            if self.down_payload == 0:
                # Logger().debug("[[ payload complete = %s" % len(self.down_data))
                self.consume_packet_down(self.down_data)
                self.down_data = None
            else:
                # Logger().debug("[[ payload incomplete need %s" % self.down_payload)
                return
        while data and len(data) >= 4:
            packet_size, = struct.unpack(">I", data[:4])
            packet_buff = data[4:]
            buff_length = len(packet_buff)
            if buff_length < packet_size:
                self.down_data = packet_buff
                self.down_payload = packet_size - buff_length
                # Logger().debug("$$ PACKET incomplete packet need %s" % self.down_payload)
                return
            data = packet_buff[packet_size:]
            packet_buff = packet_buff[:packet_size]
            # Logger().debug("$$ PACKET complete %s" % packet_size)
            self.consume_packet_down(packet_buff)
        if data:
            self.down_incomplete_size = data

    def get_packet_type(self, packet):
        packet_type, = struct.unpack(">B", packet[0:1])
        return packet_type

    def consume_packet_up(self, packet):
        packet_length = len(packet)
        if not packet_length:
            return
        # Logger().debug(">>>>>>>>> FRONT = %s" % packet_length)
        packet_type = self.get_packet_type(packet)
        # Logger().debug("Packet type %s" % SSH_FXP.get(packet_type, packet_type))
        if packet_type == SSH_FXP_OPEN:
            self.parse_open_packet(packet[1:])
        if packet_type == SSH_FXP_CLOSE:
            self.parse_close_packet(packet[1:])
        if packet_type == SSH_FXP_READ:
            self.parse_read_packet(packet[1:])
        if packet_type == SSH_FXP_WRITE:
            self.parse_write_packet(packet[1:])
        if packet_type in [ SSH_FXP_LSTAT, SSH_FXP_STAT, SSH_FXP_OPENDIR,
                            SSH_FXP_REMOVE, SSH_FXP_RMDIR, SSH_FXP_MKDIR ]:
            self.parse_get_path_packet(packet[1:], packet_type)
        if packet_type in [ SSH_FXP_LINK, SSH_FXP_SYMLINK, SSH_FXP_RENAME ]:
            self.parse_get_twopath_packet(packet[1:], packet_type)

    def consume_packet_down(self, packet):
        packet_length = len(packet)
        # Logger().debug("<<<<<<<<< BACK length = %s" % packet_length)
        packet_type = self.get_packet_type(packet)
        # Logger().debug("Packet type %s" % SSH_FXP.get(packet_type))
        if packet_type == SSH_FXP_HANDLE:
            self.parse_handle_packet(packet[1:])
        if packet_type == SSH_FXP_STATUS:
            self.parse_status_packet(packet[1:])
        if packet_type == SSH_FXP_DATA:
            self.parse_data_packet(packet[1:])



    def parse_open_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) <= packet_length:
            reqid, path_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, pathlength %s" % (reqid, path_length))
            if offset + path_length <= packet_length:
                path = packet[offset:offset+path_length]
                # Logger().debug("path = %s" % path)
                # TODO: UTF8 conversion
                offset += path_length
                path = path
                if offset + 8 <= packet_length:
                    access, flags = struct.unpack(">II", packet[offset:offset+8])
                    # Logger().debug("ACCESS = %s FLAGS = %s" % (access, flags))
                    action = "get" if access == FLAG_READ else "put"
                    self.cb_record("%s '%s' begin\r\n" % (action, path))
                self.req_table[reqid] = path


    def parse_close_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) <= packet_length:
            reqid, handle_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, pathlength %s" % (reqid, path_length))
            if offset + handle_length <= packet_length:
                handle = packet[offset:offset+handle_length]
                ft = self.handle_table.pop(handle, None)
                offset += handle_length
                if ft is not None:
                    self.cb_record("%s '%s' done, length = %s\r\n" %
                                  (ft.action, ft.filename, ft.filesize))
                #     Logger().info("done %s '%s', length = %s" %
                #                   (action, path, length))
                # Logger().info("req table = '%s'" % self.req_table)
                # Logger().info("handle table = '%s'" % self.handle_table)

    def parse_read_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 20) <= packet_length:
            reqid, handle_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, handlelength %s" % (reqid, handle_length))
            if offset + handle_length <= packet_length:
                handle = packet[offset:offset+handle_length]
                # dump = ":".join("{:02x}".format(ord(c)) for c in handle)
                # Logger().debug("handle dump = '%s'" % dump)
                offset += handle_length
                self.req_table[reqid] = handle

    def parse_write_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) <= packet_length:
            reqid, handle_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, handlelength %s" % (reqid, handle_length))
            if offset + handle_length <= packet_length:
                handle = packet[offset:offset+handle_length]
                # dump = ":".join("{:02x}".format(ord(c)) for c in handle)
                # Logger().debug("handle dump = '%s'" % dump)
                offset += handle_length
                if offset + 12 <= packet_length:
                    data_offset, data_length = struct.unpack(
                        ">QI", packet[offset:offset+12])
                    # Logger().debug("data_offset %s, datalength %s" %
                    #               (data_offset, data_length))
                    ft = self.handle_table.get(handle, None)
                    if ft is not None:
                        ft.filesize += data_length
                        if ft.action is None:
                            ft.set_action("put")
                        if ft.remaining is None:
                            res = self.cb_detect(ft.filesize, mdecode(ft.filename))
                            # Notify => False, Kill => True, no detection => None
                            if res in [ False, True ]:
                                ft.remaining = True
                                if res:
                                    msg = "Restriction: '%s' file too big" % ft.filename
                                    self.cb_record("Kill %s\r\n" % msg)
                                    tosend = self.build_fake_write_packet(
                                        reqid, handle, offset, data_length)
                                    raise AnalyzerKill(msg, tosend)
                        self.handle_table[handle] = ft
                        # Logger().debug(
                        #     "set path = '%s', acc length = %s, inc length = %s" %
                        #     (path, length, data_length))
                    else:
                        Logger().debug("!!!!! WARNING PATH NOT FOUND in handle_table!!!!")

    def parse_handle_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) <= packet_length:
            reqid, handle_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, handlelength %s" % (reqid, handle_length))
            if offset + handle_length <= packet_length:
                handle = packet[offset:offset+handle_length]
                # dump = ":".join("{:02x}".format(ord(c)) for c in handle)
                # Logger().debug("handle dump = '%s'" % dump)
                offset += handle_length
                path = self.req_table.pop(reqid, None)
                if path is not None:
                    # Logger().debug("got path = '%s'" % path)
                    ft = FileTransfer(path, 0)
                    self.handle_table[handle] = ft
                # else:
                #     Logger().debug("!!!!!! WARINING PATH NOT FOUND in req_table !!!!!!!")

    def parse_data_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) <= packet_length:
            reqid, data_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            # Logger().debug("request id %s, datalength %s" % (reqid, data_length))
            handle = self.req_table.pop(reqid, None)
            if handle is not None:
                # dump = ":".join("{:02x}".format(ord(c)) for c in handle)
                # Logger().debug("got handle dump = '%s'" % dump)
                ft = self.handle_table.get(handle, None)
                if ft is not None:
                    ft.filesize += data_length
                    if ft.action is None:
                        ft.set_action("get")
                    if ft.remaining is None:
                        res = self.cb_detect(ft.filesize, mdecode(ft.filename))
                        # Notify => False, Kill => True, no detection => None
                        if res in [ False, True ]:
                            ft.remaining = True
                            if res:
                                msg = "Restriction: '%s' file too big" % ft.filename
                                self.cb_record("Kill %s\r\n" % msg)
                                raise AnalyzerKill(msg)
                    self.handle_table[handle] = ft
                    # Logger().debug(
                    #     "set path = '%s', acc length = %s, inc length = %s" %
                    #     (path, length, data_length))
                else:
                    Logger().debug("!!!!! WARNING PATH NOT FOUND in handle_table!!!!")
            else:
                self.global_length += data_length
                # Logger().debug(
                #     "global acc length = %s, inc length = %s" %
                #     (self.global_length, data_length))

    def parse_status_packet(self, packet):
        packet_length = len(packet)
        offset = 0
        if (offset + 12) <= packet_length:
            reqid, status, msg_length = struct.unpack(">III",
                                                      packet[offset:offset+12])
            offset += 12
            # Logger().debug("request id %s" % reqid)
            command = self.req_table.pop(reqid, None)
            # TODO check status if got associated command ?
            if command is not None and (offset + msg_length) <= packet_length:
                message = packet[offset:offset + msg_length]
                # Logger().debug("status %s message '%s'" % (status, message))

    def parse_get_path_packet(self, packet, packet_type):
        packet_length = len(packet)
        offset = 0
        path = None
        if (offset + 8) <= packet_length:
            reqid, path_length = struct.unpack(">II", packet[offset:offset+8])
            offset += 8
            if (offset + path_length) <= packet_length:
                path = packet[offset:offset+path_length]
                offset += path_length
        if path is not None:
            path = path
            cmd = SFTP_COMMAND.get(packet_type, "*unknown command*")
            self.cb_record("%s '%s'\r\n" % (cmd, path))
            # Logger().debug("%s path = '%s'" % (cmd, path))

    def parse_get_twopath_packet(self, packet, packet_type):
        packet_length = len(packet)
        offset = 0
        if (offset + 8) > packet_length:
            return
        reqid, firstpath_length = struct.unpack(">II", packet[offset:offset+8])
        offset += 8
        if (offset + firstpath_length) > packet_length:
            return
        firstpath = packet[offset:offset+firstpath_length]
        offset += firstpath_length
        if (offset + 4) > packet_length:
            return
        secondpath_length, = struct.unpack(">I", packet[offset:offset+4])
        offset += 4
        if (offset + secondpath_length) > packet_length:
            return
        secondpath = packet[offset:offset+secondpath_length]
        offset += secondpath_length
        cmd = SFTP_COMMAND.get(packet_type, "*unknown command*")
        self.cb_record("%s '%s' '%s'\r\n" % (cmd, firstpath, secondpath))
        # Logger().debug("%s '%s' '%s'" % (cmd, firstpath, secondpath))


class FileTransfer(object):
    def __init__(self, filename=None, filesize=None, action=None):
        self.filename = filename
        self.filesize = filesize
        self.remaining = None
        self.action = action
    def reset(self):
        self.filename = None
        self.filesize = None
        self.remaining = None
        self.action = None

    def is_transfering(self, length):
        if self.remaining is not None:
            self.remaining -= length
            return True
        return False
    def set_filename(self, name):
        self.filename = name
    def set_filesize(self, size):
        self.filesize = size
        self.remaining = size
    def set_action(self, action):
        self.action = action

class SCPAnalyzer(ProtoAnalyzer):
    def __init__(self, cb_record=None, cb_detect=None):
        ProtoAnalyzer.__init__(self, cb_record, cb_detect)
        self.copy_regex = re.compile("^C\d{4}\s(\d+)\s(.*)")
        self.up_data_left = None
        self.down_data_left = None

        self.up_data = FileTransfer()
        self.down_data = FileTransfer()

    def parse_up_data(self, data):
        if self.up_data.is_transfering(len(data)):
            if self.up_data.remaining <= 0:
                self.cb_record("scp upload '%s' done, length = %s\r\n" %
                               (self.up_data.filename, self.up_data.filesize))
                self.up_data.reset()
            return
        # Logger().info("SCP UP data = '%s'" % data)
        res = self.copy_regex.search(data)
        if res:
            # self.cb_record("scp up '%s'\n" % res.group(0))
            self.up_data.set_filesize(int(res.group(1)))
            self.up_data.set_filename(res.group(2))
            self.up_data.set_action("up")
            self.cb_record("scp upload '%s' begin\r\n" % self.up_data.filename)
            res = self.cb_detect(self.up_data.filesize,
                                 mdecode(self.up_data.filename))
            if res is True:
                msg = "Restriction: '%s' file too big" % self.up_data.filename
                self.cb_record("Kill %s\r\n" % msg)
                raise AnalyzerKill(msg)
            # Logger().info("SCP File name = %s" % self.up_data.filename)
            # Logger().info("SCP File size = %s" % self.up_data.filesize)

    def parse_down_data(self, data):
        if self.down_data.is_transfering(len(data)):
            if self.down_data.remaining <= 0:
                self.cb_record("scp download '%s' done, length = %s\r\n" %
                               (self.down_data.filename, self.down_data.filesize))
                self.down_data.reset()
            return
        # Logger().info("SCP DOWN data = '%s'" % data)
        res = self.copy_regex.search(data)
        if res:
            # self.cb_record("scp down '%s'\n" % res.group(0))
            self.down_data.set_filesize(int(res.group(1)))
            self.down_data.set_filename(res.group(2))
            self.down_data.set_action("down")
            self.cb_record("scp download '%s' begin\r\n" % self.down_data.filename)
            res = self.cb_detect(self.down_data.filesize,
                                 mdecode(self.down_data.filename))
            if res is True:
                msg = "Restriction: '%s' file too big" % self.down_data.filename
                self.cb_record("Kill %s\r\n" % msg)
                raise AnalyzerKill(msg)
            # Logger().info("SCP File name = %s" % self.down_data.filename)
            # Logger().info("SCP File size = %s" % self.down_data.filesize)
