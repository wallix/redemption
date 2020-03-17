// const maxPacketSize = 1600;
// const headerSize = 8;

const CF = Object.freeze({
    // TEXT            : 1,
    BITMAP          : 2,
    // METAFILEPICT    : 3,
    // SYLK            : 4,
    // DIF             : 5,
    // TIFF            : 6,
    // OEMTEXT         : 7,
    // DIB             : 8,
    // PALETTE         : 9,
    // PENDATA         : 10,
    // RIFF            : 11,
    // WAVE            : 12,
    UNICODETEXT        : 13,
    // ENHMETAFILE     : 14,
    // HDROP           : 15,
    // LOCALE          : 16,
    // DIBV5           : 17,
    // OWNERDISPLAY    : 128,
    // DSPTEXT         : 129,
    // DSPBITMAP       : 130,
    // DSPMETAFILEPICT : 131,
    // DSPENHMETAFILE  : 142,
    // PRIVATEFIRST    : 512,
    // PRIVATELAST     : 767,
    // GDIOBJFIRST     : 768,
    // GDIOBJLAST      : 1023,
});

const CustomCF = Object.freeze({
    None: 0,
    FileGroupDescriptorW: 33333,
});

const FileContentsOp = Object.freeze({
    Size: 1,
    Range: 2,
});

const FileAttributes = Object.freeze({
    Readonly: 0x1,
    Hidden: 0x2,
    System: 0x4,
    Directory: 0x10,
    Archive: 0x20,
    Normal: 0x80,
});

const FileFlags = Object.freeze({
    Attributes: 0X4,
    FileSize: 0x40,
    WriteTime: 0x20,
    ShowProgressUI: 0x4000,
});

const ChannelFlags = Object.freeze({
    First: 1,
    Last: 2,
});

const MsgFlags = Object.freeze({
    None: 0,
    Ok: 1,
    Fail: 2,
    AsciiName: 4,
});

const CbType = Object.freeze({
    MonitorReady: 0x0001,
    FormatList: 0x0002,
    ListResponse: 0x0003,
    DataRequest: 0x0004,
    DataResponse: 0x0005,
    TempDirectory: 0x0006,
    Capabilities: 0x0007,
    FileContentsRequest: 0x0008,
    FileContentsResponse: 0x0009,
    Lock: 0x000A,
    Unlock: 0x000B,
});

const CbGeneralFlags = Object.freeze({
    UseLongFormatNames:     0x00000002,
    StreamFileclipEnabled:  0x00000004,
    FileclipNoFilePaths:    0x00000008,
    CanLockClipData:        0x00000010,
    HugeFileSupportEnabled: 0x00000020
})


class Cliprdr
{
    constructor(DOMBox, syncData, emccModule) {
        // TODO DOMBox, syncData -> {addFormats, setFileGroupId, ...}
        this.emccModule = emccModule;
        this.syncData = syncData;
        this.clipboard = null;
        this.locks = {};
        this.formats = [];
        this.expectedFormatId = null;
        this.fileGroupId = null;
        this.dataDecoder = null;
        this.countFile = 0;
        this.ifile = 0;
        this.streamId = 0;

        const buflen = 1600;
        const bufp = emccModule._malloc(buflen);
        this.buffer = {
            capacity: buflen,
            i: bufp,
            array: emccModule.HEAPU8.subarray(bufp, bufp + buflen)
        }

        this.DOMBox = DOMBox;
        this.DOMFormats = this.DOMBox.appendChild(document.createElement('div'));
        this.DOMFiles = this.DOMBox.appendChild(document.createElement('div'));

        this.DOMFormats.onclick = (e) => {
            e.preventDefault();
            let formatId = e.originalTarget.dataset.formatId;
            if (formatId) {
                formatId = Number(formatId);
                console.log('DOMFormats.onclick:', formatId);
                this.expectedFormatId = formatId;
                const customCf = (formatId === this.fileGroupId)
                    ? CustomCF.FileGroupDescriptorW
                    : 0;
                this.clipboard.sendRequestFormat(formatId, customCf);
                this.syncData();
            }
        };

        this.DOMFiles.onclick = (e) => {
            e.preventDefault();
            let ifile = e.originalTarget.dataset.ifile;
            if (ifile) {
                ifile = Number(ifile);
                console.log('DOMFiles.onclick:', ifile);
                console.log('streamId:', this.streamId);
                // TODO lock + pos + hugeFileSupport
                this.clipboard.sendFileContentsRequest(
                    FileContentsOp.Range, this.streamId, ifile, 0, 0, 0x7ffff, 0, 0);
                ++this.streamId;
                this.syncData();
            }
        };
    }

    delete() {
        this.emccModule._free(this.buffer.i);
        this.DOMBox.removeChild(this.DOMFormats);
        this.DOMBox.removeChild(this.DOMFiles);
    }

    _processWithBuffer(bufLen, f) {
        const reallocBuf = (this.buffer.capacity < bufLen);
        if (this.buffer.capacity < bufLen) {
            const ibuf = this.emccModule._malloc(bufLen);
            if (!ibuf) {
                return false;
            }
            // TODO first parameter not used
            f(this.emccModule.HEAPU8.subarray(ibuf, ibuf + bufLen), ibuf);
            this.emccModule._free(ibuf);
        }
        else {
            // TODO first parameter not used
            f(this.buffer.array.subarray(0, bufLen), this.buffer.i)
        }

        return true;
    }

    setEmcChannel(chann) {
        this.clipboard = chann;
    }

    setGeneralCapability(generalFlags) {
        console.log('setGeneralCapability:', generalFlags);
        this.lockSupport = !!(generalFlags & CbGeneralFlags.CanLockClipData);
        // this.fileSupport = !!(generalFlags & CbGeneralFlags.StreamFileclipEnabled);
        // this.hugeFileSupport = !!(generalFlags & CbGeneralFlags.HugeFileSupportEnabled);
        return generalFlags;
    }

    formatListStart() {
        this.DOMFiles.innerText = '';
        this.DOMFormats.innerText = '';
        this.fileGroupId = null;
    }

    formatListFormat(dataName, formatId, customFormatId, isUTF8) {
        console.log('formatList:', formatId, customFormatId, isUTF8);

        const button = document.createElement('button');
        button.dataset.formatId = formatId;

        switch (formatId) {
            case CF.UNICODETEXT: {
                button.appendChild(new Text("UNICODETEXT"));
                break;
            }

            case CF.BITMAP: {
                button.appendChild(new Text("BITMAP"));
                break;
            }

            default: {
                switch (customFormatId) {
                    case CustomCF.FileGroupDescriptorW:
                        button.appendChild(new Text(`${formatId}: FileGroupDescriptorW`));
                        this.fileGroupId = formatId;
                        break;

                    case CustomCF.None:
                        const name = (isUTF8 ? UTF8Decoder : UTF16Decoder).decode(dataName);
                        button.appendChild(new Text(`${formatId}: ${name}`));
                        break;
                }
                break;
            }
        }

        this.DOMFormats.appendChild(button);
    }

    formatListStop() {
        console.log('formatListStop');
    }

    formatDataResponse(data, remainingDataLen, formatId, channelFlags) {
        console.log('formatDataResponse:', remainingDataLen, formatId, channelFlags);

        switch (formatId) {
            case CF.UNICODETEXT: {
                if (channelFlags & ChannelFlags.First) {
                    this.dataDecoder = new TextDecoder("utf-16");
                }

                if (channelFlags & ChannelFlags.Last) {
                    const text = this.dataDecoder.decode(data)
                    console.log(text);
                }
                else {
                    const chunckedText = this.dataDecoder.decode(data, {stream: true});
                    console.log(chunckedText)
                }
                break;
            }

            // TODO
            // case CF.BITMAP:
            //     break;

            default:
                console.log('Unknown data');
        }
    }

    formatDataResponseNbFileName(countFile) {
        console.log('formatDataResponseNbFileName:', countFile);
        this.countFile = countFile;
        this.ifile = 0;
        this.DOMFiles.innerText = '';
    }

    formatDataResponseFile(utf16Name, attr, flags, sizeLow, sizeHigh, lastWriteTimeLow, lastWriteTimeHigh) {
        const filename = UTF16Decoder.decode(utf16Name);
        console.log('formatDataResponseFile:', filename, attr, flags, sizeLow, sizeHigh, lastWriteTimeLow, lastWriteTimeHigh);
        const button = this.DOMFiles.appendChild(document.createElement('button'));
        button.appendChild(new Text(`${attr}, ${flags}, ${(sizeHigh << 32) | sizeLow}, ${(lastWriteTimeHigh << 32) | lastWriteTimeLow}  ${filename}`));
        button.dataset.ifile = this.ifile;
        ++this.ifile;
    }

    fileContentsResponse(data, streamId, remainingDataLen, channelFlags) {
        console.log('fileContentsResponse:', streamId, remainingDataLen, channelFlags);
        // TODO streamId

        if (channelFlags & ChannelFlags.First) {
            this.dataDecoder = [data]
        }
        else {
            this.dataDecoder.push(data)
        }

        if (channelFlags & ChannelFlags.Last) {
            const blob = new Blob(this.dataDecoder, {type: "application/octet-stream"});
            cbDownload.href = URL.createObjectURL(blob);
            // TODO filename
            cbDownload.download = 'filename';
            cbDownload.click();
        }
    }

    formatDataRequest(formatId) {
        console.log('formatDataRequest:', formatId);

        switch (formatId) {
            case CF.UNICODETEXT: {
                const data = sendCbUtf8_data.value;
                const capacity = data.length * 2 + 2 /*"\0\0"*/ + 8 /* header size */;
                if (!this._processWithBuffer(capacity, (arr, ibuffer) => {
                    let len = this.emccModule.stringToUTF16(data, ibuffer + 8, capacity - 8) + 2;

                    const stream = new OutStream(ibuffer, this.emccModule)
                    stream.u16le(CbType.DataResponse);
                    stream.u16le(MsgFlags.Ok);
                    stream.u32le(len);

                    if (capacity <= 1600) {
                        this.clipboard.sendRawData(
                            ibuffer, capacity, capacity, ChannelFlags.First | ChannelFlags.Last);
                    }
                    else {
                        this.clipboard.sendRawData(ibuffer, 1600, capacity, ChannelFlags.First);
                        let i = 1600;
                        while (capacity - i > 1600) {
                            this.clipboard.sendRawData(ibuffer + i, 1600, capacity, 0);
                            i += 1600;
                        }
                        this.clipboard.sendRawData(
                            ibuffer + i, capacity - i, capacity, ChannelFlags.Last);
                    }
                })) {
                    // TODO malloc error
                }
                break;
            }

            case CustomCF.FileGroupDescriptorW: {
                const file = sendCbFile_data.files[0]

                const flags = FileFlags.FileSize | FileFlags.ShowProgressUI /*| FileFlags.WriteTime*/;
                const attrs = FileAttributes.Normal;
                const stream = new OutStream(this.channel_ibuffer, this.emccModule);

                stream.u16le(CbType.DataResponse);
                stream.u16le(MsgFlags.Ok);
                const headerSizePos = stream.i;
                stream.skip(4);

                stream.u32le(1/*files.length*/);
                stream.u32le(flags);
                stream.bzero(32);
                stream.u32le(attrs);
                stream.bzero(16);
                // lastWriteTime: specifies the number of 100-nanoseconds intervals that have elapsed since 1 January 1601.
                stream.u64le((file.lastModified + 11644473600) * 10000000);
                stream.u64lem(file.size);
                stream.copyStringAsAlignedUTF16(file.name);
                stream.bzero(520 - file.name.length * 2);

                const totalLen = stream.i - this.channel_ibuffer;
                stream.i = headerSizePos;
                stream.u32le(totalLen);

                console.log(totalLen);

                this.clipboard.sendRawData(this.channel_ibuffer, totalLen, totalLen, ChannelFlags.First | ChannelFlags.Last);
                break;
            }
        }
    }

    fileContentsRequest(streamId, type, lindex, nposLow, nposHigh, szRequested) {
        console.log("fileContentsRequest:", ...arguments);

        const file = sendCbFile_data.files[lindex];
        console.log(file.size, file.name);

        switch (type)
        {
        case FileContentsOp.Size: {
            const stream = new OutStream(this.channel_ibuffer, this.emccModule);

            stream.u16le(CbType.FileContentsResponse);
            stream.u16le(MsgFlags.Ok);
            const headerSizePos = stream.i;
            stream.skip(4);

            stream.u32le(streamId);
            stream.u64le(file.size);

            const totalLen = stream.i - this.channel_ibuffer;
            stream.i = headerSizePos;
            stream.u32le(totalLen);

            console.log(totalLen);

            clipboard.sendRawData(this.channel_ibuffer, totalLen, totalLen, ChannelFlags.First | ChannelFlags.Last);

            rdpclient.sendBufferedData();
            break;
        }

        case FileContentsOp.Range: {
            // TODO chunk
            const reader = new FileReader();

            // Closure to capture the file information.
            reader.onload = function(e) {
                const contents = new Uint8Array(e.target.result);
                console.log(contents.length);
                const stream = new OutStream(this.channel_ibuffer, this.emccModule);

                stream.u16le(CbType.FileContentsResponse);
                stream.u16le(MsgFlags.Ok);
                const headerSizePos = stream.i;
                stream.skip(4);

                stream.u32le(streamId);
                stream.copyAsArray(contents);

                const totalLen = stream.i - this.channel_ibuffer;
                stream.i = headerSizePos;
                stream.u32le(totalLen);

                console.log(totalLen);

                clipboard.sendRawData(this.channel_ibuffer, totalLen, totalLen, ChannelFlags.First | ChannelFlags.Last);

                rdpclient.sendBufferedData();
            };

            reader.readAsArrayBuffer(file);
            break;
        }
        }
    }

    lock(lockId) {
        console.log("lock:", lockId);
    }

    unlock(lockId) {
        console.log("unlock:", lockId);
    }

    receiveResponseFail(msgType) {
        console.log("receiveResponseFail", msgType)
    }
}
