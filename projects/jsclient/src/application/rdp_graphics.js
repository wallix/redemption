"strict";

const rgbToCss = function(color) {
    return '#'+color.toString(16).padStart(6, '0');
};

// const has_intersection = (x1,y1,w1,h1,x2,y2,w2,h2) =>
//     ((x2 >= x1 && x2 < x1 + w1) || (x1 >= x2 && x1 < x2 + w2))
//  && ((y2 >= y1 && y2 < y1 + h1) || (y1 >= y2 && y1 < y2 + h2));

class RDPGraphics
{
    constructor(canvasElement, module) {
        // this.width = canvasElement.width;
        // this.height = canvasElement.height;

        this._module = module;
        this._buffer = module.HEAPU8.buffer;
        this._imgBufferSize = 64*64*4;
        this._imgBufferIndex = module._malloc(this._imgBufferSize);

        this._ecanvas = canvasElement
        this._canvas = canvasElement.getContext('2d');
        try {
            this._ecusorCanvas = new OffsreenCanvas(0,0);
        }
        catch (e) {
            this._ecusorCanvas = document.createElement('canvas');
        }
        this._cusorCanvas = this._ecusorCanvas.getContext('2d');
        this._cachePointers = [];
        this._cacheImages = [];

        this._canvas.imageSmoothingEnabled = false;
    }

    delete() {
        console.log('RDPGraphics: free memory')
        this._module._free(this._imgBufferIndex);
    }

    frameMarker(isFrameStart) {
    }

    unsupportedRop(cmd, rop) {
        console.error(`${cmd}: Unsupported rop 0x${rop.toString(16).padStart(2, '0')}`);
    }

    resizeCanvas(w, h, bpp) {
        console.log('RDPGraphics: resize(' + w + ', ' + h + ', ' + bpp + ')');

        this.width = w;
        this.height = h;
        this._ecanvas.width = w;
        this._ecanvas.height = h;
        this.drawRect(0, 0, w, h, 0);
    }

    setBmpCacheEntries(
        cache0_nb_entries, cache0_bmp_size, cache0_is_persistent,
        cache1_nb_entries, cache1_bmp_size, cache1_is_persistent,
        cache2_nb_entries, cache2_bmp_size, cache2_is_persistent,
    ) {
        this._cacheImages.length = cache0_nb_entries + cache1_nb_entries + cache2_nb_entries;
    }

    setBmpCacheIndex(byteOffset, bitsPerPixel, w, h, lineSize, imageIdx) {
        // assume w*h <= 64*64
        this._module.loadRgbaImageFromIndex(this._imgBufferIndex,
                                            byteOffset, bitsPerPixel, w, h, lineSize);
        const array = this._buffer.slice(this._imgBufferIndex, this._imgBufferIndex + w*h*4);
        // array is copied by Uint8ClampedArray
        this._cacheImages[imageIdx] = new ImageData(new Uint8ClampedArray(array), w, h);
    }

    _transformImage2(dstImg, srcImg, sx, sy, x, y, f) {
        const destU32a = new Uint32Array(dstImg.data.buffer);
        const srcU32a = new Uint32Array(srcImg.data.buffer);

        const w = dstImg.width;
        const len = w * dstImg.height;
        const srcInc = srcImg.width - w;
        let isrc = sy * srcImg.width + sx;

        for (let idst = 0; idst < len;) {
            for (let ie = idst + w; idst < ie; ++idst, ++isrc) {
                destU32a[idst] = f(srcU32a[isrc] & 0xffffff, destU32a[idst] & 0xffffff)
                               | 0xff000000;
            }
            isrc += srcInc;
        }

        this._canvas.putImageData(dstImg, x, y);
    }

    _transformDstImage(img, sx, sy, dx, dy, dw, dh, f) {
        const dstImg = this._canvas.getImageData(dx, dy, dw, dh);
        this._transformImage2(dstImg, img, sx, sy, dx, dy, f);
    }

    _copyImage(img, sx, sy, dw, dh) {
        const w = img.width;
        const h = img.height;
        const u32av = new Uint32Array(img.data.buffer);
        const data = new Uint32Array(dh*dw);

        if (0 === sx && w === dw) {
            data.set(u32av.subarray(sy*dw, dh*dw));
        }
        else {
            let i = sy*w + sx;
            const ie = i + dh*w;
            for (; i < ie; ++i, i+=w) {
                data.set(u8av.slice(i, i+dw));
            }
        }

        return new ImageData(new Uint8ClampedArray(data.buffer), dw, dh);
    }

    drawMemBlt(imageIdx, rop, sx, sy, dx, dy, dw, dh) {
        const img = this._cacheImages[imageIdx];
        dw = Math.min(img.width - sx, this.width - dx, dw);
        dh = Math.min(img.height - sy, this.height - dy, dh);

        if (dw <= 0 || dh <= 0) {
            return;
        }

        switch (rop) {
            case 0x00:
                this._canvas.fillStyle = "#000";
                this._canvas.fillRect(dx, dy, dw, dh);
                break;

            case 0x55:
                const newImg = this._copyImage(img, sx, sy, dw, dh);
                this._transformImage(newImg, dx, dy, (src) => src ^ 0xffffff);
                break;

            case 0xCC:
                this._canvas.putImageData(img, dx, dy, sx, sy, dw, dh);
                break;

            case 0x22:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => ~src & dst);
                break;

            case 0x66:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src ^ dst);
                break;

            case 0x88:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src & dst);
                break;

            case 0xBB:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => ~src | dst);
                break;

            case 0xEE:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src | dst);
                break;

            case 0xff:
                this._canvas.fillStyle = "#fff";
                this._canvas.fillRect(dx, dy, dw, dh);
                break;

            default:
                this.unsupportedRop('MemBlt', rop);
                break;
        }
    }

    drawMem3Blt(brushData, orgX, orgY, style,
                imageIdx, rop, sx, sy, dx, dy, dw, dh, backColor, foreColor
    ) {
        const img = this._cacheImages[imageIdx];
        dw = Math.min(img.width - sx, this.width - dx, dw);
        dh = Math.min(img.height - sy, this.height - dy, dh);

        if (dw <= 0 || dh <= 0) {
            return;
        }

        foreColor &= 0xffffff;

        switch (rop) {
            case 0xB8:
                this._transformDstImage(img, sx, sy, dx, dy, dw, dh,
                                        (src, dst) => ((dst ^ foreColor) & src) ^ foreColor);
                break;

            default:
                this.unsupportedRop('Mem3Blt', rop);
                break;
        }
    }

    drawImage(byteOffset, bitsPerPixel, w, h, lineSize, ...args) {
        let destOffset;
        if (bitsPerPixel != 32) {
            const bufferSize = w*h*4;
            if (bufferSize > this._imgBufferSize) {
                this._module._free(this._imgBufferIndex);
                this._imgBufferSize = bufferSize;
                this._imgBufferIndex = this._module._malloc(bufferSize);
            }

            destOffset = this._imgBufferIndex;
            this._module.loadRgbaImageFromIndex(destOffset, byteOffset,
                                               bitsPerPixel, w, h, lineSize);
        }
        else {
            destOffset = byteOffset;
        }

        // buffer is referenced by Uint8ClampedArray
        const array = new Uint8ClampedArray(this._buffer, destOffset, w*h*4);
        this._canvas.putImageData(new ImageData(array, w, h), ...args);
    }

    drawRect(x, y, w, h, color) {
        // console.log('drawRect');
        this._canvas.fillStyle = rgbToCss(color);
        this._canvas.fillRect(x,y,w,h);
    }

    _transformPixels2(sx, sy, dx, dy, dw, dh, f) {
        const dstImg = this._canvas.getImageData(dx, dy, w, h);
        const srcImg = this._canvas.getImageData(sx, sy, w, h);
        this._transformImage2(dstImg, srcImg, 0, 0, dx, dy, f);
    }

    drawScrBlt(sx, sy, w, h, dx, dy, rop) {
        // console.log('drawScrBlt');
        switch (rop) {
            case 0x00:
                this._canvas.fillStyle = "#000";
                this._canvas.fillRect(x,y,w,h);
                break;
            case 0x11: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src | dst)); break;
            case 0x22: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~src & dst); break;
            case 0x33: this._transformPixels(sx,sy,w,h, (src) => ~src); break;
            case 0x44: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src & ~dst); break;
            case 0x55: this._transformPixels(dx,dy,w,h, (dst) => ~dst); break;
            case 0x66: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src ^ dst); break;
            case 0x77: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src & dst)); break;
            case 0x88: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src & dst); break;
            case 0x99: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src ^ dst)); break;
            case 0xaa: break;
            case 0xbb: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~src | dst); break;
            case 0xcc: this._canvas.putImageData(this._canvas.getImageData(sx,sy,w,h), dx,dy); break;
            case 0xdd: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src | ~dst); break;
            case 0xee: this._transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src | dst); break;
            case 0xff:
                this._canvas.fillStyle = "#fff";
                this._canvas.fillRect(x,y,w,h);
                break;

            default:
                this.unsupportedRop('ScrBlt', rop);
                break;
        }
    }

    drawLineTo(backMode, startX, startY, endX, endY, backColor, penStyle, penWidth, penColor) {
        // console.log('drawLineTo');
        this._canvas.save();
        this._canvas.fillStyle = rgbToCss(backColor);
        this._canvas.strokeStyle = rgbToCss(penColor);
        // behavior of stroke is strange (transparency color with a odd penWidth)
        if (!penStyle && startX === endX) {
            if (endX < startX) {
                startX = endX;
            }
            if (endY < startY) {
                [startY, endY] = [endY, startY];
            }
            this._canvas.fillRect(startX, startY, penWidth||1, endY-startY+1);
        }
        else if (!penStyle && startY === endY) {
            if (endY < startY) {
                startY = endY;
            }
            if (endX < startX) {
                [startX, endX] = [endX, startX];
            }
            this._canvas.fillRect(startX, startY, endX-startX+1, penWidth||1);
        }
        else {
            this._canvas.beginPath();
            this._canvas.moveTo(startX, startY);
            this._canvas.lineTo(endX, endY);
            this._canvas.lineWidth = penWidth;
            switch (penStyle) {
                case 1: this._canvas.setLineDash([ 10, 6, 10, 6 ]); break;
                case 2: this._canvas.setLineDash([ 3, 3, 3, 3 ]); break;
                case 3: this._canvas.setLineDash([ 9, 6, 3, 6 ]); break;
                case 4: this._canvas.setLineDash([ 9, 3, 3, 3 ]); break;
                case 5: this._canvas.setLineDash([ 16, 0, 16, 0 ]); break;
            }
            // BackMode does not imply the transparency level of what is about too be drawn
            // canvas.globalAlpha = (backMode === 1 /* TRANSPARENT */? 0.0 : 1.0);
            this._canvas.stroke();
        }
        this._canvas.restore();
    }

    drawPolyline(startX, startY, deltas, clipX, clipY, clipW, clipH, penColor) {
        // console.log('drawPolyline');
        this._canvas.save();
        this._canvas.strokeStyle = rgbToCss(penColor);
        this._canvas.beginPath();
        this._canvas.moveTo(startX, startY);
        let endX = startX;
        let endY = startY;
        const iend = deltas.length
        for (let i = 0; i < iend; i += 2) {
            endX += deltas[i];
            endY += deltas[i+1];
            this._canvas.lineTo(endX, endY);
        }
        this._canvas.stroke();
        this._canvas.restore();
    }

    _transformPixels(x, y, w, h, f) {
        const img = this._canvas.getImageData(x, y, w, h);
        this._transformImage(img, x, y, f);
    }

    _transformImage(img, x, y, f) {
        const u32a = new Uint32Array(img.data.buffer);
        const len = img.width * img.height;
        for (let i = 0; i < len; ++i) {
            u32a[i] = f(u32a[i] & 0xffffff) | 0xff000000;
        }
        this._canvas.putImageData(img, x, y);
    }

    _transformPixelsBrush(orgX, orgY, w, h, backColor, foreColor, brushData, f) {
        const imgData = this._canvas.getImageData(orgX, orgY, w, h);
        const u32a = new Uint32Array(imgData.data.buffer);
        w = imgData.width;
        h = imgData.height;
        for (let y = 0; y < h; ++y) {
            const brushU8 = brushData[(y + orgY) % 8];
            const i = y * w;
            for (let x = 0; x < w; ++x) {
                const selectColor = (brushU8 & ((1 << 7) >> ((x + orgX) % 8)));
                u32a[i+x] = f(selectColor ? backColor : foreColor, u32a[i+x] & 0xffffff)
                          | 0xff000000;
            }
        }
        this._canvas.putImageData(imgData, orgX, orgY);
    }

    drawPatBlt(brushData, orgX, orgY, style, x, y, w, h, rop, backColor, foreColor) {
        switch (rop) {
            case 0x00:
                this._canvas.fillStyle = "#000";
                this._canvas.fillRect(x,y,w,h);
                break;
            case 0x05: this._transformPixels(x,y,w,h, (src) => ~(backColor | src)); break;
            case 0x0f: this._transformPixels(x,y,w,h, (src) => ~src); break;
            case 0x50: this._transformPixels(x,y,w,h, (src) => src & ~backColor); break;
            case 0x55: this._transformPixels(x,y,w,h, (src) => src ^ 0xffffff); break;
            case 0x5a:
                if (style === 0x03) {
                    this._transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData,
                                               (src,c) => src ^ c);
                }
                else {
                    this._transformPixels(x,y,w,h, (src) => backColor ^ src);
                }
                break;
            case 0x5f: this._transformPixels(x,y,w,h, (src) => ~(backColor & src)); break;
            case 0xa0: this._transformPixels(x,y,w,h, (src) => backColor & src); break;
            case 0xa5: this._transformPixels(x,y,w,h, (src) => ~(backColor ^ src)); break;
            case 0xaa: break;
            case 0xaf: this._transformPixels(x,y,w,h, (src) => backColor | ~src); break;
            case 0xf0:
                if (style === 0x03) {
                    this._transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData,
                                               (src,c) => src);
                }
                else {
                    this._canvas.fillStyle = rgbToCss(backColor);
                    this._canvas.fillRect(x,y,w,h);
                }
                break;
            case 0xfa: this._transformPixels(x,y,w,h, (src) => src | backColor); break;
            case 0xf5: this._transformPixels(x,y,w,h, (src) => src | ~backColor); break;
            case 0xff:
                this._canvas.fillStyle = "#fff";
                this._canvas.fillRect(x,y,w,h);
                break;

            default:
                this.unsupportedRop('PatBlt', rop);
                break;
        }
    }

    drawDestBlt(x, y, w, h, rop) {
        switch (rop) {
        case 0x00:
            this._canvas.fillStyle = "#000";
            this._canvas.fillRect(x,y,w,h);
            break;
        case 0x55: this._transformPixels(x,y,w,h, (src) => src ^ 0xffffff); break;
        // case 0xAA: break;
        case 0xff:
            this._canvas.fillStyle = "#fff";
            this._canvas.fillRect(x,y,w,h);
            break;

        default:
            this.unsupportedRop('DestBlt', rop);
            break;
        }
    }

    _image2CSS(idata, w, h, x, y) {
        const array = new Uint8ClampedArray(this._buffer, idata, w * h * 4);
        const image = new ImageData(array, w, h);
        this._ecusorCanvas.width = w;
        this._ecusorCanvas.height = h;
        this._cusorCanvas.putImageData(image, 0, 0);
        const dataURL = this._ecusorCanvas.toDataURL();
        // console.log('url(' + dataURL + ') ' + x + ' ' + y + ', auto');
        return 'url(' + dataURL + ') ' + x + ' ' + y + ', auto';
    }

    setPointer(idata, w, h, x, y) {
        this._ecanvas.style.cursor = this._image2CSS(idata, w, h, x, y);
    }

    newPointer(idata, w, h, offset, x, y) {
        const data = this._image2CSS(idata, w, h, x, y);
        this._cachePointers[offset] = data;
        this._ecanvas.style.cursor = data;
    }

    cachedPointer(offset) {
        this._ecanvas.style.cursor = this._cachePointers[offset];
    }

    updatePointerPosition(x, y) {
    }
};

try {
    module.exports.RDPGraphics = RDPGraphics;
}
catch (e) {
}
