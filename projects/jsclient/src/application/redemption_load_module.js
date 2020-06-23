const redemptionLoadModule = function(Module, window)
{
    const HEAPU8 = Module.HEAPU8;
    const HEAP16 = Module.HEAP16;

    const identity = function(cb, thisp) {
        // TODO return cb; ?
        return function(...args) {
            return cb.call(thisp, ...args);
        };
    };

    const wCb_em2js_ImageData = function(cb, thisp) {
        return function(idata, w, h, ...args) {
            const array = new Uint8ClampedArray(HEAPU8.buffer, idata, w * h * 4);
            const image = new ImageData(array, w, h);
            return cb.call(thisp, image, ...args);
        };
    };

    const noop = function(){};

    // { funcname: [wrapCreator, defaultFunction], ... }
    const wrappersGd = {
        setBmpCacheEntries: identity,
        setBmpCacheIndex: identity,
        drawMemBlt: identity,
        drawMem3Blt: identity,

        drawImage: identity,
        drawRect: identity,
        drawScrBlt: identity,
        drawLineTo: identity,
        drawDestBlt: identity,
        drawPolyline: identity,
        drawPolygoneSC: identity,
        drawPolygoneCB: identity,
        drawEllipseSC: identity,
        drawEllipseCB: identity,
        drawPatBlt: identity,
        drawFrameMarker: identity,

        setPointer: wCb_em2js_ImageData,
        newPointer: wCb_em2js_ImageData,
        cachedPointer: identity,

        resizeCanvas: identity,
        updatePointerPosition: identity,
    };

    const wrappersFront = {
        random: identity,
    };

    const wrapEvents = function(wrappedEvents, wrappers, events, defaultCb) {
        for (const eventName in wrappers) {
            const wrapCb = wrappers[eventName];
            const cb = events[eventName];
            wrappedEvents[eventName] = cb ? wrapCb(cb, events) : defaultCb;
        }
    };


    class RDPClient {
        constructor(socket, width, height, events, username, password, disabledOrders, verbosity) {
            const rdpEvents = {};
            wrapEvents(rdpEvents, wrappersGd, events, undefined);
            wrapEvents(rdpEvents, wrappersFront, events, undefined);
            rdpEvents.drawFrameMarker = rdpEvents.drawFrameMarker || noop;
            rdpEvents.updatePointerPosition = rdpEvents.updatePointerPosition || noop;
            rdpEvents.random = rdpEvents.random || function(idata, len) {
                const data = HEAPU8.subarray(idata, idata + len);
                window.crypto.getRandomValues(data);
            }

            verbosity = verbosity || 0;

            this.native = new Module.RdpClient(
                rdpEvents, width, height,
                username || "", password || "",
                disabledOrders || 0,
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
        formatListFormat: identity,
        formatListStop: identity,
        formatDataResponse: identity,
        formatDataResponseFileStart: identity,
        formatDataResponseFile: identity,
        formatDataResponseFileStop: identity,
        formatDataRequest: identity,
        fileContentsRequest: identity,
        fileContentsResponse: identity,
        receiveResponseFail: identity,
        lock: identity,
        unlock: identity,
        free: identity,
    });

    addChannelClass('CustomChannel', {
        receiveData: identity,
        free: identity,
    });

    return resultFuncs;
};
