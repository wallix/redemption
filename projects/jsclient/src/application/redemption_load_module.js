const redemptionLoadModule = function(Module, window)
{
    const HEAPU8 = Module.HEAPU8;
    const HEAP16 = Module.HEAP16;

    const identity = function(cb, thisp) {
        return function(...args) {
            return cb.call(thisp, ...args);
        };
    };

    const wCb_em2js_Bitmap = function(cb, thisp) {
        return function(idata, bitsPerPixel, w, h, lineSize, ...args) {
            const data = HEAPU8.subarray(idata, idata + lineSize * h);
            return cb.call(thisp, data, bitsPerPixel, w, h, lineSize, ...args);
        };
    };

    const wCb_em2js_ImageData = function(cb, thisp) {
        return function(idata, w, h, ...args) {
            const array = new Uint8ClampedArray(HEAPU8.buffer, idata, w * h * 4);
            const image = new ImageData(array, w, h);
            return cb.call(thisp, image, ...args);
        };
    };

    const wCb_em2js_U8p = function(cb, thisp) {
        return function(idata, len, ...args) {
            const data = HEAPU8.subarray(idata, idata + len);
            return cb.call(thisp, data, ...args);
        };
    };

    const wCb_em2js_Deltas = function(cb, thisp) {
        return function(xStart, yStart, numDeltaEntries, deltaEntries, ...args) {
            const deltas = HEAP16.subarray(deltaEntries, deltaEntries + numDeltaEntries * 2);
            return cb.call(thisp, xStart, yStart, deltas, ...args);
        };
    };

    const wCb_em2js_Brush = function(cb, thisp) {
        return function(brushData, ...args) {
            cb.call(thisp, HEAPU8.subarray(brushData, brushData + 8), ...args);
        };
    };

    const noop = function(){};

    // { funcname: [wrapCreator, defaultFunction], ... }
    const wrappersGd = {
        setBmpCacheEntries: identity,
        setBmpCacheIndex: identity,
        drawMemBlt: identity,
        drawMem3Blt: wCb_em2js_Brush,

        drawImage: identity,
        drawRect: identity,
        drawScrBlt: identity,
        drawLineTo: identity,
        drawDestBlt: identity,
        drawPolyline: wCb_em2js_Deltas,
        drawPolygoneSC: wCb_em2js_Deltas,
        drawPolygoneCB: function(cb, thisp) {
            return function(xStart, yStart, numDeltaEntries, deltaEntries,
                            clipX, clipY, clipW, clipH, brushData, ...args
            ) {
                const deltas = HEAP16.subarray(deltaEntries, deltaEntries + numDeltaEntries * 2);
                return cb.call(thisp, xStart, yStart, deltas,
                               clipX, clipY, clipW, clipH,
                               HEAPU8.subarray(brushData, brushData + 8),
                               ...args);
            };
        },
        drawEllipseSC: identity,
        drawEllipseCB: identity,
        drawPatBlt: wCb_em2js_Brush,
        drawFrameMarker: identity,

        setPointer: wCb_em2js_ImageData,
        newPointer: wCb_em2js_ImageData,
        cachedPointer: identity,

        resizeCanvas: identity,
    };

    const wrappersFront = {
        random: wCb_em2js_U8p,
    };

    const wrapEvents = function(wrappedEvents, wrappers, events, defaultCb) {
        for (const eventName in wrappers) {
            const wrapCb = wrappers[eventName];
            const cb = events[eventName];
            wrappedEvents[eventName] = cb ? wrapCb(cb, events) : defaultCb;
        }
    };


    class RDPClient {
        constructor(socket, width, height, events, username, password, disabled_orders, verbosity) {
            const rdpEvents = {};
            wrapEvents(rdpEvents, wrappersGd, events, undefined);
            wrapEvents(rdpEvents, wrappersFront, events, undefined);
            rdpEvents.drawFrameMarker = rdpEvents.drawFrameMarker || noop;
            rdpEvents.random = rdpEvents.random || function(idata, len) {
                const data = HEAPU8.subarray(idata, idata + len);
                window.crypto.getRandomValues(data);
            }

            verbosity = verbosity || 0;

            this.native = new Module.RdpClient(
                rdpEvents, width, height,
                username || "", password || "",
                disabled_orders || 0,
                verbosity & 0xffffffff,
                (verbosity > 0xffffffff ? verbosity - 0xffffffff : 0)
            );
            this.socket = socket;
            this._channels = [];
        }

        close() {
            this.socket.close();
        }

        start() {
            this.native.sendFirstPacket();
            this.sendBufferedData();
        }

        delete() {
            for (const channel of this._channels) {
                channel.delete();
            }
            this.native.delete();
        }

        getCallback() {
            return this.native.getCallbackAsVoidPtr();
        }

        addChannel(channel) {
            this._channels.push(channel);
            this.native.addChannelReceiver(channel.getChannelReceiver());
        }

        sendUnicode(unicode, flags) {
            this.native.sendUnicode(unicode, flags);
            this.sendBufferedData();
        }

        sendScancode(key, flags) {
            this.native.sendScancode(key, flags);
            this.sendBufferedData();
        }

        sendMouseEvent(flag, x, y) {
            this.native.sendMouseEvent(flag, x, y);
            this.sendBufferedData();
        }

        sendBufferedData() {
            const out = this.native.getSendingData();
            if (out.length) {
                this.socket.send(out);

                // let text = "";
                // console.log('Received Binary Message of ' + out.length + ' bytes');
                // for (let byte of out) {
                //     text += ":" + (byte+0x10000).toString(16).substr(-2);
                // }
                // this.counter = (this.counter || 0) + 1
                // console.log(this.counter, "Send: " + text);

                this.native.clearSendingData();
            }
        }

        addReceivingData(data) {
            this.native.addReceivingData(data);
        }
    };

    const wrappersPlayer = {
        setPointerPosition: identity,
        setTime: identity,
    };

    const resultFuncs = {
        RDPClient: RDPClient,
        newRDPSocket: function(url) {
            const socket = new WebSocket(url, "RDP");
            socket.binaryType = 'arraybuffer';
            return socket;
        },
        newWrmPlayer: function(events) {
            const playerEvents = {};
            wrapEvents(playerEvents, wrappersGd, events, noop);
            wrapEvents(playerEvents, wrappersPlayer, events, noop);
            return new Module.WrmPlayer(playerEvents);
        }
    };

    const addChannelClass = (ChannelClassName, wrappers) => {
        resultFuncs['new' + ChannelClassName] = function(cb, events, ...channelArgs) {
            wrapEvents(events, wrappers, events, noop);
            const chann = new Module[ChannelClassName](cb, events, ...channelArgs);
            const setChann = events['setEmcChannel'];
            if (setChann) {
                setChann.call(events, chann);
            }
            return chann;
        };
    };

    addChannelClass('ClipboardChannel', {
        setGeneralCapability: identity,
        formatListStart: identity,
        formatListFormat: wCb_em2js_U8p,
        formatListStop: identity,
        formatDataResponse: wCb_em2js_U8p,
        formatDataResponseFileStart: identity,
        formatDataResponseFile: wCb_em2js_U8p,
        formatDataResponseFileStop: identity,
        formatDataRequest: identity,
        fileContentsRequest: identity,
        fileContentsResponse: wCb_em2js_U8p,
        receiveResponseFail: identity,
        lock: identity,
        unlock: identity,
        free: identity,
    });

    addChannelClass('CustomChannel', {
        receiveData: wCb_em2js_U8p,
        free: identity,
    });

    return resultFuncs;
};
