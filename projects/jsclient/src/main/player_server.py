#!/usr/bin/env python3

import websockets
import asyncio
import os
import ctypes
from struct import unpack_from, unpack, pack

from scytale.decrypter import CryptoReader
from scytale.meta import MetaReader

e_error = 0
e_ok = 1
e_version = 2
e_open = 3
e_file_infos = 4
e_open_file_index = 5
e_next_data = 6
e_eof = 7

class Server:
    def __init__(self, websocket):
        self.socket = websocket
        self.filenames = []
        self.reader = None
        buflen = 65536
        self.bufdatalen = buflen - 16
        self.buffer = (ctypes.c_char * buflen)()
        self.buffer[0] = e_next_data
        self.bufdata = (ctypes.c_char * self.bufdatalen).from_buffer(self.buffer, 1)

    async def receive(self, message):
        msg_type = int(unpack_from('>B', message)[0])
        print('type:', msg_type)
        try:
            if msg_type == e_next_data:
                await self.process_next_data()
            elif msg_type == e_open:
                await self.process_open(message)
            elif msg_type == e_open_file_index:
                await self.process_open_file_index(message)
            elif msg_type == e_version:
                await self.process_version(message)
        except IOError as e:
            print(e)
            await self.socket.send(pack('>B', e_error))

    async def process_version(self, message):
        n = (len(message) - 1) // 4
        # send last version
        vers = unpack_from('>I', message, 1+(n-1)*4)[0]
        print('vers:', vers)
        await self.socket.send(pack('>BI', e_version, vers))

    async def process_open(self, message):
        times_and_sizes = [e_file_infos]
        filename = message[1:]#.decode("utf-8")
        print('filename:', filename)
        with CryptoReader(filename) as cr:
            with MetaReader(cr) as meta:
                meta.read_mwrm_header()
                print('header| version:', meta.get_header().version,
                      'has_checksum:', meta.get_header().has_checksum)
                for line in meta.iter_mwrm_line():
                    self.filenames.append(line.filename)
                    times_and_sizes.append(line.stop_time - line.start_time)
                    times_and_sizes.append(line.stat.size if line.stat else os.path.getsize(line.filename))
        print(self.filenames)
        print(times_and_sizes)
        await self.socket.send(pack('>B'+'QQ'*(len(times_and_sizes)//2), *times_and_sizes))

    async def process_seek(self, message):
        pass

    async def process_open_file_index(self, message):
        if self.reader:
            self.reader.close()

        ifile = unpack_from('>I', message, 1)[0]
        print('fileindex', ifile, '/', len(self.filenames))
        self.reader = CryptoReader(self.filenames[ifile])
        self.reader.open()
        await self.process_next_data()

    async def process_next_data(self):
        n = self.reader.readbuffer(self.bufdata, self.bufdatalen)
        print('ndata:', n)
        if n == 0:
            await self.socket.send(pack('>B', e_eof))
        else:
            await self.socket.send(self.buffer[:n+1])


async def run(websocket, path):
    server = Server(websocket)
    async for message in websocket:
        await server.receive(message)
    print('Stop')

start_server = websockets.serve(run, 'localhost', 8765)

loop = asyncio.get_event_loop()
loop.run_until_complete(start_server)
loop.run_forever()
