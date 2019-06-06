#!/usr/bin/env python3

import websockets
import asyncio
import os
import ctype
from struct import unpack_from, pack

from scytale.decrypter import CryptoReader
from scytale.meta import MetaReader

e_version = 0
e_open = 1
e_ifile = 2
e_next_data = 3

class Server:
    def __init__(self, websocket):
        self.socket = websocket
        self.filenames = []
        self.reader = None
        buflen = 65536
        self.buffer = (ctype.c_char * buflen)()
        self.buffer[0] = e_next_data
        self.bufdata = (ctype.c_char * (buflen - 1)).from_buffer(self.buffer, 1)

    def receive(self, message):
        msg_type = int(unpack_from('<B', message)[0])
        if msg_type == e_next_data:
            self.process_next_data()
        elif msg_type == e_open:
            self.process_open(message)
        elif msg_type == e_ifile:
            self.process_ifile(message)
        elif msg_type == e_version:
            self.process_version(message)

    def process_version(self, message):
        n = (len(message) - 1) / 4
        if n:
            # send last version
            self.socket.send(message[1+(n-1)*4:])

    def process_open(self, message):
        times_and_sizes = [e_open]
        with CryptoReader(filename) as cr:
            with MetaReader(cr) as meta:
                for line in meta.tier_mwrm_line():
                    self.filenames.append(line.filename)
                    times_and_sizes.append(line.stop_time - line.start_time)
                    times_and_sizes.append(line.size or os.path.getsize(line.filename))
        print(self.filenames)
        print(times_and_sizes)
        self.socket.send(pack('<B'+'QQ'*len(times_and_sizes)*2, **times_and_sizes))

    def process_seek(self, message):
        pass

    def process_ifile(self, message):
        if self.reader:
            self.reader.close()

        ifile = unpack_from('<I', message, 1)[0]
        self.reader = CryptoReader(self.filenames[ifile])
        self.reader.open()
        self.process_next(message)

    def process_next_data(self):
        n = self.reader.readbuffer(self.bufdata, 65536-1)
        self.socket.send((ctype.c_char * (n + 1)).from_buffer(self.buffer))
        pass

async def hello(websocket, path):
    server = Server(websocket)
    async for message in websocket:
        server.receive(message)
        msg_type = unpack_from('<B', message)[0]
        print(f"< {message}")

        greeting = f"Hello {message}!"

        await websocket.send(greeting)
        print(f"> {greeting}")

        unpack("<Q")

start_server = websockets.serve(hello, 'localhost', 8765)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
