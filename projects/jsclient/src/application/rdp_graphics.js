const rgbToCss = function(color) {
    return '#'+color.toString(16).padStart(6, '0');
};

// const has_intersection = (x1,y1,w1,h1,x2,y2,w2,h2) =>
//     ((x2 >= x1 && x2 < x1 + w1) || (x1 >= x2 && x1 < x2 + w2))
//  && ((y2 >= y1 && y2 < y1 + h1) || (y1 >= y2 && y1 < y2 + h2));

class RDPGraphics
{
    constructor(canvasElement) {
        this.ecanvas = canvasElement
        this.canvas = canvasElement.getContext('2d');
        this.ecusorCanvas = document.createElement('canvas')
        this.cusorCanvas = this.ecusorCanvas.getContext('2d');
        this.cachePointers = [];
        this.cacheImages = [];
        this.promise = Promise.resolve()
        this.orderStack = [];

        this.canvas.imageSmoothingEnabled = false;
    }

    resizeCanvas(w, h) {
        if (this.ecanvas.width !== w || this.ecanvas.height !== h) {
            // restore canvas after resize
            const imgData = this.canvas.getImageData(0, 0, this.ecanvas.width, this.ecanvas.height);
            this.ecanvas.width = w;
            this.ecanvas.height = h;
            this.canvas.putImageData(imgData, 0, 0);
        }
    }

    setCachedImageSize(n) {
        this.cacheImages.length = n;
    }

    cachedImage(imageData, imageIdx) {
        const p = createImageBitmap(imageData);
        const setImg = (img) => { this.cacheImages[imageIdx] = img };
        this.promise = this.promise.then(() => p).then(setImg);
    }

    drawCachedImage(imageIdx, rop, sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight) {
        // rop supposed to 0xCC
        this.promise = this.promise.then(() => {
            this.canvas.drawImage(this.cacheImages[imageIdx], sx, sy, sWidth, sHeight, dx, dy, dWidth, dHeight);
        });
    }

    drawImage(imageData, rop, ...args) {
        // assume rop == 0xCC
        this.promise = this.promise.then(() => {
            this.canvas.putImageData(imageData, ...args);
        });
        // TODO clone imageData or move ownership (return true + free) ?
    }

    drawRect(x, y, w, h, color) {
        // console.log('drawRect');
        this.promise = this.promise.then(() => {
            this.canvas.fillStyle = rgbToCss(color);
            this.canvas.fillRect(x,y,w,h);
        });
    }

    drawSrcBlt(sx, sy, w, h, dx, dy, rop) {
        // console.log('drawSrcBlt');
        this.promise = this.promise.then(() => {
            const sourceImageData = this.canvas.getImageData(sx, sy, w, h);
            if (rop === 0xCC) {
                this.canvas.putImageData(sourceImageData, dx, dy);
            }
            else {
                let op;
                switch (rop) {
                    case 0x00: op = 'darken'; break;
                    // case 0xF0: op = 'source-over'; break;
                    case 0x55: op = 'xor'; break;
                    case 0xFF: op = 'lighten'; break;
                    default: op = 'source-over'; break;
                }
                this.canvas.globalCompositeOperation = op;
                this.canvas.putImageData(sourceImageData, dx, dy);
                this.canvas.globalCompositeOperation = 'source-over';
            }
        });
    }

    drawLineTo(backMode, startX, startY, endX, endY, backColor, penStyle, penWidth, penColor) {
        // console.log('drawLineTo');
        this.promise = this.promise.then(() => {
            this.canvas.save();
            this.canvas.fillStyle = rgbToCss(backColor);
            this.canvas.strokeStyle = rgbToCss(penColor);
            // behavior of stroke is strange (transparency color with a odd penWidth)
            if (!penStyle && startX === endX) {
                if (endX < startX) {
                    startX = endX;
                }
                if (endY < startY) {
                    [startY, endY] = [endY, startY];
                }
                this.canvas.fillRect(startX, startY, penWidth||1, endY-startY+1);
            }
            else if (!penStyle && startY === endY) {
                if (endY < startY) {
                    startY = endY;
                }
                if (endX < startX) {
                    [startX, endX] = [endX, startX];
                }
                this.canvas.fillRect(startX, startY, endX-startX+1, penWidth||1);
            }
            else {
                this.canvas.beginPath();
                this.canvas.moveTo(startX, startY);
                this.canvas.lineTo(endX, endY);
                this.canvas.lineWidth = penWidth;
                switch (penStyle) {
                    case 1: this.canvas.setLineDash([ 10, 6, 10, 6 ]); break;
                    case 2: this.canvas.setLineDash([ 3, 3, 3, 3 ]); break;
                    case 3: this.canvas.setLineDash([ 9, 6, 3, 6 ]); break;
                    case 4: this.canvas.setLineDash([ 9, 3, 3, 3 ]); break;
                    case 5: this.canvas.setLineDash([ 16, 0, 16, 0 ]); break;
                }
                // BackMode does not imply the transparency level of what is about too be drawn
                // canvas.globalAlpha = (backMode === 1 /* TRANSPARENT */? 0.0 : 1.0);
                this.canvas.stroke();
            }
            this.canvas.restore();
        });
    }

    drawPolyline(startX, startY, deltas, penColor) {
        // console.log('drawPolyline');
        this.promise = this.promise.then(() => {
            this.canvas.save();
            this.canvas.strokeStyle = rgbToCss(penColor);
            this.canvas.beginPath();
            this.canvas.moveTo(startX, startY);
            let endX = startX;
            let endY = startY;
            const iend = deltas.length
            for (let i = 0; i < iend; i += 2) {
                endX += deltas[i];
                endY += deltas[i+1];
                this.canvas.lineTo(endX, endY);
            }
            this.canvas.stroke();
            this.canvas.restore();
        });
    }

    _transformPixels(x, y, w, h, f) {
        const imgData = this.canvas.getImageData(x, y, w, h);
        const u32a = new Uint32Array(imgData.data.buffer);
        const len = imgData.width * imgData.height;
        for (let i = 0; i < len; ++i) {
            u32a[i] = f(u32a[i] & 0xff000000) | 0xff000000;
        }
        this.canvas.putImageData(imgData, x, y);
    }

    _invert(x, y, w, h) {
        this._transformPixels(x,y,w,h, (src) => src ^ 0xffffff);
    }

    drawPatBlt(x, y, w, h, rop, color) {
        this.promise = this.promise.then(() => {
            switch (rop) {
                case 0x00:
                    this.canvas.fillStyle = "#000";
                    this.canvas.fillRect(x,y,w,h);
                    break;
                case 0x05: this._transformPixels(x,y,w,h, (src) => ~(color | src)); break;
                case 0x0f: this._transformPixels(x,y,w,h, (src) => ~src); break;
                case 0x50: this._transformPixels(x,y,w,h, (src) => src & ~color); break;
                case 0x55: this._invert(x,y,w,h); break;
                case 0x5a: this._transformPixels(x,y,w,h, (src) => color ^ src); break;
                case 0x5f: this._transformPixels(x,y,w,h, (src) => ~(color & src)); break;
                case 0xa0: this._transformPixels(x,y,w,h, (src) => color & src); break;
                case 0xa5: this._transformPixels(x,y,w,h, (src) => ~(color ^ src)); break;
                case 0xaa: break;
                case 0xaf: this._transformPixels(x,y,w,h, (src) => color | ~src); break;
                case 0xf0:
                    this.canvas.fillStyle = rgbToCss(color);
                    this.canvas.fillRect(x,y,w,h);
                    break;
                case 0xfa: this._transformPixels(x,y,w,h, (src) => src | color); break;
                case 0xf5: this._transformPixels(x,y,w,h, (src) => src | ~color); break;
                case 0xff:
                    this.canvas.fillStyle = "#fff";
                    this.canvas.fillRect(x,y,w,h);
                    break;
                default: console.log('unsupported PatBlt rop', rop);
            }
        });
    }

    _transformPixelsBrush(orgX, orgY, w, h, backColor, foreColor, brushData, f) {
        const imgData = this.canvas.getImageData(orgX, orgY, w, h);
        const u32a = new Uint32Array(imgData.data.buffer);
        w = imgData.width;
        h = imgData.height;
        for (let y = 0; y < h; ++y) {
            const brushU8 = brushData[(y + orgY) % 8];
            const i = y * w;
            for (let x = 0; x < w; ++x) {
                const selectColor = (brushU8 & ((1 << 7) >> ((x + orgX) % 8)));
                u32a[i+x] = f(selectColor ? backColor : foreColor, u32a[i+x] & 0xff000000) | 0xff000000;
            }
        }
        this.canvas.putImageData(imgData, orgX, orgY);
    }

    drawPatBltEx(x, y, w, h, rop, backColor, foreColor, brushData) {
        this.promise = this.promise.then(() => {
            switch (rop) {
                case 0xf0: this._transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData, (src,c) => src); break;
                case 0x5a: this._transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData, (src,c) => src ^ c); break;
                default: console.log('unsupported PatBltEx rop', rop);
            }
        });
    }

    drawDestBlt(x, y, w, h, rop) {
        this.promise = this.promise.then(() => {
            switch (rop) {
            case 0x00:
                this.canvas.fillStyle = "#000";
                this.canvas.fillRect(x,y,w,h);
                break;
            case 0x55: this._invert(x,y,w,h); break;
            // case 0xAA: break;
            case 0xff:
                this.canvas.fillStyle = "#fff";
                this.canvas.fillRect(x,y,w,h);
                break;
            }
        });
    }

    image2CSS(image, x, y) {
        this.ecusorCanvas.width = image.width;
        this.ecusorCanvas.height = image.height;
        this.cusorCanvas.putImageData(image, 0, 0);
        const dataURL = this.ecusorCanvas.toDataURL();
        // console.log('url(' + dataURL + ') ' + x + ' ' + y + ', auto');
        return 'url(' + dataURL + ') ' + x + ' ' + y + ', auto';
    }

    setPointer(image, x, y) {
        this.ecanvas.style.cursor = this.image2CSS(image, x, y);
    }

    newPointer(image, offset, x, y) {
        const data = this.image2CSS(image, x, y);
        this.cachePointers[offset] = data;
        this.ecanvas.style.cursor = data;
    }

    cachedPointer(offset) {
        this.ecanvas.style.cursor = this.cachePointers[offset];
    }
};
