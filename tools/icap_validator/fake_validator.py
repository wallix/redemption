#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import select
import sys
import os
import struct
import time # sleep

def read_session_socket(session_socket, size_to_read):
    """
    Read exactly size_to_read bytes from socket and
    return a tuple with True if no error is detected and received data
    """
    received_message = b""

    while size_to_read:
        received_data = session_socket.recv(size_to_read)
        if received_data:
            size_to_read -= len(received_data)
            received_message += received_data
        else:
            break

    return size_to_read == 0, received_message

def read_session_msg(client_socket):
    """
    Read session client socket received data, parse the header to return read
    message type and True if no error is detected
    """
    message_type = -1
    received_message = b""

    ok, header_received = read_session_socket(client_socket, 5)
    if ok:
        message_type, message_len = struct.unpack_from(">BI", header_received)
        ok, received_message = read_session_socket(client_socket, message_len)
        if ok:
            return True, received_message, message_type

    return False, received_message, message_type

ACCEPTED = 0x00
REJECTED = 0x01

IS_UNICODE_DATA = 0
ACCEPTED_ON_EOF = 1
REJECTED_ON_EOF = 2
ACCEPTED_WITH_DELAY = 3
REJECTED_WITH_DELAY = 4

def send_response_message(socket, file_id, result, content):
    """
    Send a Response message
    """
    print(f'send: file_id={file_id} content={content}')
    content_len = len(content)
    msg = struct.pack(">BI", 0x05, 9 + content_len)
    msg += struct.pack(">BII", result, file_id, content_len)
    msg += content
    socket.send(msg)

def process_new_data(message, client_socket, data):
    """
    Parse a NewFile message and send a response or
    wait a eof type message. If filename contains
    - norep: no response
    - virus0: send REJECTED immediately
    - ok0: send ACCEPTED immediately
    - virus1: send REJECTED on eof or abort
    - ok1: send ACCEPTED on eof or abort
    - virus2: send REJECTED 7 seconds after eof or abort
    - ok2: send ACCEPTED 7 seconds after eof or abort
    - otherwise, same as ok1
    """

    # file id, u16, target, *(u16, key, u16, data)
    file_id, = struct.unpack_from(">I", message)
    msg_data = message[6:]
    # ignore target_name_len and target_name

    print(f'file_id={file_id} msg_data={msg_data}')

    if b'virus0' in msg_data:
        send_response_message(client_socket, file_id, REJECTED, b'virus0')
    elif b'ok0' in msg_data:
        send_response_message(client_socket, file_id, ACCEPTED, b'ok0')
    elif b'virus2' in msg_data:
        data[file_id] = REJECTED_WITH_DELAY
    elif b'ok2' in msg_data:
        data[file_id] = ACCEPTED_WITH_DELAY
    # unicode text
    elif b'format_id\x00\x0213' in msg_data:
        data[file_id] = IS_UNICODE_DATA
    elif b'norep' not in msg_data:
        data[file_id] = REJECTED_ON_EOF if b'virus1' in msg_data else ACCEPTED_ON_EOF

state_to_msg_table = [
    (ACCEPTED, b'ok'),
    (ACCEPTED, b'ok1'),
    (REJECTED, b'virus1'),
    (ACCEPTED, b'ok2'),
    (REJECTED, b'virus2'),
]

def process_eol(message, client_socket, data):
    """
    Parse a Eol message and send a ACCEPTED or REJECTED response
    """

    file_id, = struct.unpack_from(">I", message)

    state = data.get(file_id)
    if state is not None:
        print(f'file_id={file_id} state={state}')
        if state >= 3:
            print('7 seconds delay')
            time.sleep(7)
        p = state_to_msg_table[state]
        send_response_message(client_socket, file_id, p[0], p[1])
        del data[file_id]

process_abort_file = process_eol


def process_file_data(message, client_socket, data):
    """
    Parse a FileData message and send response or
    wait a eof type message when file_id is a text
    data.
    """

    file_id, = struct.unpack_from(">I", message)

    is_a_virus = data.get(file_id)
    if is_a_virus is IS_UNICODE_DATA:
        msg_data = message[4:]

        print(f'file_id={file_id} message={msg_data}')

        if b'v\0i\0r\0u\0s\x000\0' in msg_data:
            send_response_message(client_socket, file_id, REJECTED, b'virus0')
            del data[file_id]
        elif b'o\0k\x000\0' in msg_data:
            send_response_message(client_socket, file_id, ACCEPTED, b'ok0')
            del data[file_id]
        elif b'v\0i\0r\0u\0s\x002\0' in msg_data:
            data[file_id] = REJECTED_WITH_DELAY
        elif b'o\0k\x002\0' in msg_data:
            data[file_id] = ACCEPTED_WITH_DELAY
        elif b'v\0i\0r\0u\0s\x001\0' in msg_data:
            data[file_id] = REJECTED_ON_EOF
        elif b'o\0k\x001\0' in msg_data:
            data[file_id] = ACCEPTED_ON_EOF
        elif b'n\0o\0r\0e\0p\0' in msg_data:
            del data[file_id]

def parse_message(type, message, client_socket, data):
    """
        Parse received data according to message type
    """
    if type == 0x01: # File Data Flag
        print('parse_message: file data')
        process_file_data(message, client_socket, data)

    elif type == 0x07: # New Data Flag
        print('parse_message: new data')
        process_new_data(message, client_socket, data)

    elif type == 0x03:  # End of File Flag
        print('parse_message: end of file')
        process_eol(message, client_socket, data)

    elif type == 0x04:  # Abort File Flag
        print('parse_message: abort')
        process_abort_file(message, client_socket, data)

    elif type == 0x06: # File Infos Flag
        print('parse_message: infos')

    elif type == 0x02:  # Disconnection Flag
        print('parse_message: disconnection')
        return False

    else:
        print(f'invalid message type {type}')

    return True

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'{sys.argv[0]} socket_path\n\n'
              'If filename or text data contain:\n'
              '- norep: never send a response\n'
              '- virus0: send REJECTED immediately\n'
              '- ok0: send ACCEPTED immediately\n'
              '- virus1: send REJECTED on eof or abort\n'
              '- ok1: send ACCEPTED on eof or abort\n'
              '- virus2: send REJECTED 7 seconds after eof or abort\n'
              '- ok2: send ACCEPTED 7 seconds after eof or abort\n'
              '- otherwise, same as ok1')
        exit(1)

    socket_path = sys.argv[1]

    try:
        os.unlink(socket_path)
    except OSError:
        if os.path.exists(socket_path):
            raise

    data_by_sockets = {}  # { front_socket: (session_id, {}) }
    last_session_id = 0

    service_socket = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    service_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    service_socket.bind(socket_path)

    in_sockets = [service_socket]
    writable_s = []
    exceptionnals_s = []

    while in_sockets:
        readable, writable, exceptionnals = select.select(
            in_sockets,
            writable_s,
            exceptionnals_s
        )

        for sock in readable:
            if sock is service_socket:
                # New Client Session connection
                try:
                    service_socket.listen(100)
                    client_socket, address = service_socket.accept()
                    in_sockets.append(client_socket)

                    last_session_id += 1
                    data_by_sockets[client_socket] = (last_session_id, {})

                    print(f"Session connection (id={last_session_id})")

                except socket.error as msg:
                    print(f"ERROR can't accept session connection ({msg})")

            else:
                # Client Session message
                data = data_by_sockets[sock]
                session_id = data[0]
                session_data = data[1]

                try:
                    ok, msg, msg_type = read_session_msg(sock)
                    print(f'session {session_id}: type={msg_type} len={len(msg)}')
                except socket.error as msg:
                    print(f"ERROR session socket ({msg})")
                    ok = False

                if ok:
                    ok = parse_message(msg_type, msg, sock, session_data)
                    if not ok:
                        print(f"Session disconnection id({session_id})")
                        sock.close()
                else:
                    print("Session connection broken")

                if not ok:
                    del data_by_sockets[sock]
                    if sock in in_sockets:
                        in_sockets.remove(sock)

    print("Files Validator closed")
