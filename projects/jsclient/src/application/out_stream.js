"strict";

class OutStream
{
    constructor(ptr, emccModule) {
        this.i = ptr;
        this.HEAPU8 = emccModule.HEAPU8;
        this.emccModule = emccModule;
    }

    skip(n) {
        this.i += n;
    }

    u64lem(x) {
        if (x > 0xffffffff) {
            this.u32le(x - 0xffffffff);
        }
        else {
            this.bzero(4);
        }
        this.u32le(x);
    }

    u64le(x) {
        this.u32le(x);
        if (x > 0xffffffff) {
            this.u32le(x - 0xffffffff);
        }
        else {
            this.bzero(4);
        }
    }

    u32le(x) {
        this.HEAPU8[this.i++] = x;
        this.HEAPU8[this.i++] = (x >> 8);
        this.HEAPU8[this.i++] = (x >> 16);
        this.HEAPU8[this.i++] = (x >> 24);
    }

    u16le(x) {
        this.HEAPU8[this.i++] = x;
        this.HEAPU8[this.i++] = (x >> 8);
    }

    bzero(n) {
        this.HEAPU8.fill(0, this.i, this.i + n);
        this.i += n;
    }

    copyAsArray(array) {
        this.HEAPU8.set(array, this.i);
        this.i += array.length;
    }

    copyStringAsAlignedUTF16(str) {
        // Module.stringToUTF16(str, this.i, 0x7FFFFFFF/*, this.iend - this.i*/) + 2;
        let iheap = this.i / 2;
        const iend = str.length;
        const HEAPU16 = this.emccModule.HEAPU16;
        for (let i = 0; i < iend; ++i, ++iheap) {
            HEAPU16[iheap] = str.charCodeAt(i);
        }
        this.i = iheap * 2
    }
}
