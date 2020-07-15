"strict";

const rgbToCss = function(color) {
    return '#'+color.toString(16).padStart(6, '0');
};

// const has_intersection = (x1,y1,w1,h1,x2,y2,w2,h2) =>
//     ((x2 >= x1 && x2 < x1 + w1) || (x1 >= x2 && x1 < x2 + w2))
//  && ((y2 >= y1 && y2 < y1 + h1) || (y1 >= y2 && y1 < y2 + h2));


const newRdpPointer = function(canvasElement, module) {
    const _ecusorCanvas = (function(){
        try {
            return new OffsreenCanvas(0,0);
        }
        catch (e) {
            return document.createElement('canvas');
        }
    })();
    const _cusorCanvas = _ecusorCanvas.getContext('2d');
    const _cachePointers = [];
    const _buffer = module.HEAPU8.buffer;

    _ecusorCanvas.imageSmoothingEnabled = false;

    const _image2CSS = function(idata, w, h, x, y) {
        const array = new Uint8ClampedArray(_buffer, idata, w * h * 4);
        const image = new ImageData(array, w, h);
        _ecusorCanvas.width = w;
        _ecusorCanvas.height = h;
        _cusorCanvas.putImageData(image, 0, 0);
        const dataURL = _ecusorCanvas.toDataURL();
        // console.log('url(' + dataURL + ') ' + x + ' ' + y + ', auto');
        return 'url(' + dataURL + ') ' + x + ' ' + y + ', auto';
    };

    return {
        // @{
        // implementation for hack
        _cusorCanvas: _cusorCanvas,
        _cusorElement: _ecusorCanvas,
        _cachePointers: _cachePointers,
        // @}

        setPointer: function(idata, w, h, x, y) {
            canvasElement.style.cursor = _image2CSS(idata, w, h, x, y);
        },

        newPointer: function(idata, w, h, offset, x, y) {
            const data = _image2CSS(idata, w, h, x, y);
            _cachePointers[offset] = data;
            canvasElement.style.cursor = data;
        },

        cachedPointer: function(offset) {
            canvasElement.style.cursor = _cachePointers[offset];
        },
    };
};

const newRdpCanvas = function(canvasElement, module, ropError) {
    let _width = canvasElement.width;
    let _height = canvasElement.height;

    const _buffer = module.HEAPU8.buffer;
    let _imgBufferSize = 64*64*4;
    let _imgBufferIndex = module._malloc(_imgBufferSize);

    const _canvas = canvasElement.getContext('2d');
    const _cacheImages = [];

    _canvas.imageSmoothingEnabled = false;

    const unsupportedRop = ropError;

    const _transformImage2 = function(dstImg, srcImg, sx, sy, x, y, f) {
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

        _canvas.putImageData(dstImg, x, y);
    };

    const _transformDstImage = function(img, sx, sy, dx, dy, dw, dh, f) {
        const dstImg = _canvas.getImageData(dx, dy, dw, dh);
        _transformImage2(dstImg, img, sx, sy, dx, dy, f);
    };

    const _copyImage = function(img, sx, sy, dw, dh) {
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
    };

    const _transformPixels2 = function(sx, sy, dx, dy, dw, dh, f) {
        const dstImg = _canvas.getImageData(dx, dy, w, h);
        const srcImg = _canvas.getImageData(sx, sy, w, h);
        _transformImage2(dstImg, srcImg, 0, 0, dx, dy, f);
    };

    const _transformImage = function(img, x, y, f) {
        const u32a = new Uint32Array(img.data.buffer);
        const len = img.width * img.height;
        for (let i = 0; i < len; ++i) {
            u32a[i] = f(u32a[i] & 0xffffff) | 0xff000000;
        }
        _canvas.putImageData(img, x, y);
    };

    const _transformPixels = function(x, y, w, h, f) {
        const img = _canvas.getImageData(x, y, w, h);
        _transformImage(img, x, y, f);
    };

    const _transformPixelsBrush = function(orgX, orgY, w, h, backColor, foreColor, brushData, f) {
        const imgData = _canvas.getImageData(orgX, orgY, w, h);
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
        _canvas.putImageData(imgData, orgX, orgY);
    };

    const drawRect = function(x, y, w, h, color) {
        // console.log('drawRect');
        _canvas.fillStyle = rgbToCss(color);
        _canvas.fillRect(x,y,w,h);
    };

    return {
        // @{
        // implementation for hack
        _canvas: _canvas,
        _cacheImages: _cacheImages,
        _transformImage2: _transformImage2,
        _transformDstImage: _transformDstImage,
        _copyImage: _copyImage,
        _transformPixels2: _transformPixels2,
        _transformPixels: _transformPixels,
        _transformImage: _transformImage,
        _transformPixelsBrush: _transformPixelsBrush,
        // @}

        get width() { return _width; },
        get height() { return _height; },

        delete: function() {
            // console.log('RdpGraphics: free memory')
            module._free(_imgBufferIndex);
        },

        drawRect: drawRect,

        frameMarker: function(isFrameStart) {
        },

        resizeCanvas: function(w, h, bpp) {
            // console.log('RdpGraphics: resize(' + w + ', ' + h + ', ' + bpp + ')');
            canvasElement.width = _width = w;
            canvasElement.height = _height = h;
            drawRect(0, 0, w, h, 0);
        },

        setBmpCacheEntries: function(
            cache0_nb_entries, cache0_bmp_size, cache0_is_persistent,
            cache1_nb_entries, cache1_bmp_size, cache1_is_persistent,
            cache2_nb_entries, cache2_bmp_size, cache2_is_persistent,
        ) {
            _cacheImages.length = cache0_nb_entries + cache1_nb_entries + cache2_nb_entries;
        },

        setBmpCacheIndex: function(byteOffset, bitsPerPixel, w, h, lineSize, imageIdx) {
            // assume w*h <= 64*64
            module.loadRgbaImageFromIndex(_imgBufferIndex,
                                          byteOffset, bitsPerPixel, w, h, lineSize);
            const array = _buffer.slice(_imgBufferIndex, _imgBufferIndex + w*h*4);
            // array is copied by Uint8ClampedArray
            _cacheImages[imageIdx] = new ImageData(new Uint8ClampedArray(array), w, h);
        },

        drawMemBlt: function(imageIdx, rop, sx, sy, dx, dy, dw, dh) {
            const img = _cacheImages[imageIdx];
            dw = Math.min(img.width - sx, _width - dx, dw);
            dh = Math.min(img.height - sy, _height - dy, dh);

            if (dw <= 0 || dh <= 0) {
                return;
            }

            switch (rop) {
                case 0x00:
                    _canvas.fillStyle = "#000";
                    _canvas.fillRect(dx, dy, dw, dh);
                    break;

                case 0x55:
                    const newImg = _copyImage(img, sx, sy, dw, dh);
                    _transformImage(newImg, dx, dy, (src) => src ^ 0xffffff);
                    break;

                case 0xCC:
                    _canvas.putImageData(img, dx, dy, sx, sy, dw, dh);
                    break;

                case 0x22:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => ~src & dst);
                    break;

                case 0x66:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src ^ dst);
                    break;

                case 0x88:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src & dst);
                    break;

                case 0xBB:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => ~src | dst);
                    break;

                case 0xEE:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh, (src, dst) => src | dst);
                    break;

                case 0xff:
                    _canvas.fillStyle = "#fff";
                    _canvas.fillRect(dx, dy, dw, dh);
                    break;

                default:
                    unsupportedRop('MemBlt', rop);
                    break;
            }
        },

        drawMem3Blt: function(brushData, orgX, orgY, style,
                    imageIdx, rop, sx, sy, dx, dy, dw, dh, backColor, foreColor
        ) {
            const img = _cacheImages[imageIdx];
            dw = Math.min(img.width - sx, _width - dx, dw);
            dh = Math.min(img.height - sy, _height - dy, dh);

            if (dw <= 0 || dh <= 0) {
                return;
            }

            foreColor &= 0xffffff;

            switch (rop) {
                case 0xB8:
                    _transformDstImage(img, sx, sy, dx, dy, dw, dh,
                                       (src, dst) => ((dst ^ foreColor) & src) ^ foreColor);
                    break;

                default:
                    unsupportedRop('Mem3Blt', rop);
                    break;
            }
        },

        drawImage: function(byteOffset, bitsPerPixel, w, h, lineSize, ...args) {
            let destOffset;
            if (bitsPerPixel != 32) {
                const bufferSize = w*h*4;
                if (bufferSize > _imgBufferSize) {
                    module._free(_imgBufferIndex);
                    _imgBufferSize = bufferSize;
                    _imgBufferIndex = module._malloc(bufferSize);
                }

                destOffset = _imgBufferIndex;
                module.loadRgbaImageFromIndex(destOffset, byteOffset,
                                              bitsPerPixel, w, h, lineSize);
            }
            else {
                destOffset = byteOffset;
            }

            // buffer is referenced by Uint8ClampedArray
            const array = new Uint8ClampedArray(_buffer, destOffset, w*h*4);
            _canvas.putImageData(new ImageData(array, w, h), ...args);
        },

        drawScrBlt: function(sx, sy, w, h, dx, dy, rop) {
            // console.log('drawScrBlt');
            switch (rop) {
                case 0x00:
                    _canvas.fillStyle = "#000";
                    _canvas.fillRect(x,y,w,h);
                    break;
                case 0x11: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src | dst)); break;
                case 0x22: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~src & dst); break;
                case 0x33: _transformPixels(sx,sy,w,h, (src) => ~src); break;
                case 0x44: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src & ~dst); break;
                case 0x55: _transformPixels(dx,dy,w,h, (dst) => ~dst); break;
                case 0x66: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src ^ dst); break;
                case 0x77: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src & dst)); break;
                case 0x88: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src & dst); break;
                case 0x99: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~(src ^ dst)); break;
                case 0xaa: break;
                case 0xbb: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => ~src | dst); break;
                case 0xcc: _canvas.putImageData(_canvas.getImageData(sx,sy,w,h), dx,dy); break;
                case 0xdd: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src | ~dst); break;
                case 0xee: _transformPixels2(sx,sy,dx,dy,w,h, (src, dst) => src | dst); break;
                case 0xff:
                    _canvas.fillStyle = "#fff";
                    _canvas.fillRect(x,y,w,h);
                    break;

                default:
                    unsupportedRop('ScrBlt', rop);
                    break;
            }
        },

        drawLineTo: function(
            backMode, startX, startY, endX, endY, backColor, penStyle, penWidth, penColor
        ) {
            // console.log('drawLineTo');
            _canvas.save();
            _canvas.fillStyle = rgbToCss(backColor);
            _canvas.strokeStyle = rgbToCss(penColor);
            // behavior of stroke is strange (transparency color with a odd penWidth)
            if (!penStyle && startX === endX) {
                if (endX < startX) {
                    startX = endX;
                }
                if (endY < startY) {
                    [startY, endY] = [endY, startY];
                }
                _canvas.fillRect(startX, startY, penWidth||1, endY-startY+1);
            }
            else if (!penStyle && startY === endY) {
                if (endY < startY) {
                    startY = endY;
                }
                if (endX < startX) {
                    [startX, endX] = [endX, startX];
                }
                _canvas.fillRect(startX, startY, endX-startX+1, penWidth||1);
            }
            else {
                _canvas.beginPath();
                _canvas.moveTo(startX, startY);
                _canvas.lineTo(endX, endY);
                _canvas.lineWidth = penWidth;
                switch (penStyle) {
                    case 1: _canvas.setLineDash([ 10, 6, 10, 6 ]); break;
                    case 2: _canvas.setLineDash([ 3, 3, 3, 3 ]); break;
                    case 3: _canvas.setLineDash([ 9, 6, 3, 6 ]); break;
                    case 4: _canvas.setLineDash([ 9, 3, 3, 3 ]); break;
                    case 5: _canvas.setLineDash([ 16, 0, 16, 0 ]); break;
                }
                // BackMode does not imply the transparency level of what is about too be drawn
                // canvas.globalAlpha = (backMode === 1 /* TRANSPARENT */? 0.0 : 1.0);
                _canvas.stroke();
            }
            _canvas.restore();
        },

        drawPolyline: function(startX, startY, deltas, clipX, clipY, clipW, clipH, penColor) {
            // console.log('drawPolyline');
            _canvas.save();
            _canvas.strokeStyle = rgbToCss(penColor);
            _canvas.beginPath();
            _canvas.moveTo(startX, startY);
            let endX = startX;
            let endY = startY;
            const iend = deltas.length
            for (let i = 0; i < iend; i += 2) {
                endX += deltas[i];
                endY += deltas[i+1];
                _canvas.lineTo(endX, endY);
            }
            _canvas.stroke();
            _canvas.restore();
        },

        drawPatBlt: function(brushData, orgX, orgY, style, x, y, w, h, rop, backColor, foreColor) {
            switch (rop) {
                case 0x00:
                    _canvas.fillStyle = "#000";
                    _canvas.fillRect(x,y,w,h);
                    break;
                case 0x05: _transformPixels(x,y,w,h, (src) => ~(backColor | src)); break;
                case 0x0f: _transformPixels(x,y,w,h, (src) => ~src); break;
                case 0x50: _transformPixels(x,y,w,h, (src) => src & ~backColor); break;
                case 0x55: _transformPixels(x,y,w,h, (src) => src ^ 0xffffff); break;
                case 0x5a:
                    if (style === 0x03) {
                        _transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData,
                                              (src,c) => src ^ c);
                    }
                    else {
                        _transformPixels(x,y,w,h, (src) => backColor ^ src);
                    }
                    break;
                case 0x5f: _transformPixels(x,y,w,h, (src) => ~(backColor & src)); break;
                case 0xa0: _transformPixels(x,y,w,h, (src) => backColor & src); break;
                case 0xa5: _transformPixels(x,y,w,h, (src) => ~(backColor ^ src)); break;
                case 0xaa: break;
                case 0xaf: _transformPixels(x,y,w,h, (src) => backColor | ~src); break;
                case 0xf0:
                    if (style === 0x03) {
                        _transformPixelsBrush(x,y,w,h,backColor,foreColor,brushData,
                                              (src,c) => src);
                    }
                    else {
                        _canvas.fillStyle = rgbToCss(backColor);
                        _canvas.fillRect(x,y,w,h);
                    }
                    break;
                case 0xfa: _transformPixels(x,y,w,h, (src) => src | backColor); break;
                case 0xf5: _transformPixels(x,y,w,h, (src) => src | ~backColor); break;
                case 0xff:
                    _canvas.fillStyle = "#fff";
                    _canvas.fillRect(x,y,w,h);
                    break;

                default:
                    unsupportedRop('PatBlt', rop);
                    break;
            }
        },

        drawDestBlt: function(x, y, w, h, rop) {
            switch (rop) {
            case 0x00:
                _canvas.fillStyle = "#000";
                _canvas.fillRect(x,y,w,h);
                break;
            case 0x55: _transformPixels(x,y,w,h, (src) => src ^ 0xffffff); break;
            // case 0xAA: break;
            case 0xff:
                _canvas.fillStyle = "#fff";
                _canvas.fillRect(x,y,w,h);
                break;

            default:
                unsupportedRop('DestBlt', rop);
                break;
            }
        },

        updatePointerPosition: function(x, y) {
        },
    };
};

const isPowerOf2 = function(value) {
    return (value & (value - 1)) === 0;
}

const newRdpGL = function(canvasElement, module, ropError) {
    let _width = canvasElement.width;
    let _height = canvasElement.height;

    const _buffer = module.HEAPU8.buffer;

    const unsupportedRop = ropError;

    const glOptions = {
        preserveDrawingBuffer: true,
        antialias: false,
        alpha: false,
    };
    const gl = canvasElement.getContext('webgl', glOptions)
            || canvasElement.getContext('experimental-webgl', glOptions);

    const createShaderCtx = function(type, code) {
        return {
            type: type,
            compiled: false,
            shader: gl.createShader(type),
            code: code
        };
    };

    const rectFragmentShader = createShaderCtx(gl.FRAGMENT_SHADER, `
        // precision highp float;
        precision mediump float;

        uniform vec3 uColor;

        void main() {
            gl_FragColor = vec4(uColor.x, uColor.y, uColor.z, 1.0);
        }`);

    const imgFragmentShader = createShaderCtx(gl.FRAGMENT_SHADER, `
        // precision highp float;
        precision mediump float;
        varying vec2 vTex;
        uniform sampler2D sampler0;
        void main(void){
            gl_FragColor = texture2D(sampler0, vTex);
        }`);

    const rectVertexShader = createShaderCtx(gl.VERTEX_SHADER, '');
    const imgVertexShader = createShaderCtx(gl.VERTEX_SHADER, '');

    const rectProgram = gl.createProgram();
    const imgProgram = gl.createProgram();

    const programs = [
        [rectProgram, rectVertexShader, rectFragmentShader],
        [imgProgram, imgVertexShader, imgFragmentShader],
    ];

    for (const [prog, vs, fs] of programs) {
        gl.attachShader(prog, vs.shader);
        gl.attachShader(prog, fs.shader);
    }

    const updateVertexShaderCode = function() {
        rectVertexShader.compiled = false;
        rectVertexShader.code = `
            attribute vec2 aVertexPosition;

            void main() {
                // clip from 0 to 2
                vec2 zeroToTwo = aVertexPosition / vec2(${_width/2}, ${_height/2});

                // convert 0->2 to -1->+1
                vec2 clipSpace = (zeroToTwo - 1.0);

                // flip y coordinate
                vec2 transform = clipSpace * vec2(1, -1);

                gl_Position = vec4(transform, 0, 1);
            }`
        ;

        imgVertexShader.compiled = false;
        imgVertexShader.code = `
            attribute vec2 aVertexPosition;
            attribute vec2 aUV;
            // uniform vec2 pos;

            varying vec2 vTex;

            void main(void) {
                gl_Position = vec4(
                    ( aVertexPosition + vec2(-${_width/2}, -${_height/2}) )
                    * vec2(1.0, -${_width/_height}),
                    0.0, ${_width/2}
                );
                vTex = aUV;
            }`
        ;
    };

    const compileShader = function(shaderCtx) {
        if (shaderCtx.compiled) {
            return;
        }

        gl.shaderSource(shaderCtx.shader, shaderCtx.code);
        gl.compileShader(shaderCtx.shader);
    };

    const buildPrograms = function() {
        for (const [prog, vs, fs] of programs) {
            compileShader(vs);
            compileShader(fs);
        }

        // link in parallel threads
        for (const [prog, vs, fs] of programs) {
            gl.linkProgram(prog);
        }

        let hasError = false;
        for (const [prog, vs, fs] of programs) {
            if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
                console.error('Link failed: ' + gl.getProgramInfoLog(prog));
                console.error('vs info-log: ' + gl.getShaderInfoLog(vs.shader));
                console.error('fs info-log: ' + gl.getShaderInfoLog(fs.shader));
                hasError = true;
            }
        }

        if (hasError) {
            throw new Error(gl.getError());
        }
    };

    const initShaders = function(){
        updateVertexShaderCode();
        buildPrograms();

        gl.viewport(0, 0, _width, _height);
        gl.clearColor(.0, .0, .0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT);
    };

    initShaders();

    const rectVertexBuffer = gl.createBuffer();
    const texVertexBuffer = gl.createBuffer();
    const texture = gl.createTexture();

    const drawRect = function(x, y, w, h, color) {
        // console.log('drawRect');
        gl.useProgram(rectProgram);

        const vertex = [
            x+w,y, x,y+h, x,y,
            x+w,y, x,y+h, x+w,y+h
        ];

        let vertexArray = new Float32Array(vertex);
        gl.bindBuffer(gl.ARRAY_BUFFER, rectVertexBuffer);
        gl.bufferData(gl.ARRAY_BUFFER, vertexArray, gl.STATIC_DRAW);

        const vertexNumComponents = 2;
        const vertexCount = vertexArray.length / vertexNumComponents;

        const aVertexPosition = gl.getAttribLocation(rectProgram, "aVertexPosition");

        gl.enableVertexAttribArray(aVertexPosition);
        gl.vertexAttribPointer(aVertexPosition, vertexNumComponents, gl.FLOAT, false, 0, 0);

        const red   = ((color >> 16) & 0xff) / 255.;
        const green = ((color >> 8 ) & 0xff) / 255.;
        const blue  = ((color      ) & 0xff) / 255.;

        const uColor = gl.getUniformLocation(rectProgram, "uColor");

        gl.uniform3fv(uColor, [red, green, blue]);

        gl.drawArrays(gl.TRIANGLES, 0, vertexCount);
    };

    return {
        get width() { return _width; },
        get height() { return _height; },

        delete() {
            for (const [prog, vs, fs] of programs) {
                gl.deleteProgram(prog);
            }
        },

        frameMarker: function(isFrameStart) {
        },

        drawRect: drawRect,

        // mendatory for drawRect
        drawPatBlt: function(...args) {},

        resizeCanvas: function(w, h, bpp) {
            console.log('RdpGraphics: resize(' + w + ', ' + h + ', ' + bpp + ')');
            if (_width != w || _height != h) {
                canvasElement.width = _width = w;
                canvasElement.height = _height = h;
                initShaders();
            }
            drawRect(0, 0, w, h, 0);
        },

        drawImage: function(byteOffset, bitsPerPixel, w, h, lineSize, x, y, ...args) {
            // console.log('img');
            gl.useProgram(imgProgram);

            const bufLen = w*h*4;
            const pbuf = module._malloc(bufLen);
            module.loadRgbaImageFromIndex(pbuf, byteOffset, bitsPerPixel, w, h, lineSize);
            const img = new Uint8Array(_buffer, pbuf, bufLen);

            gl.bindBuffer(gl.ARRAY_BUFFER, rectVertexBuffer);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
                x,    y+h,
                x,    y,
                x+w,  y,
                x+w,  y+h
            ]), gl.STATIC_DRAW);

            gl.bindBuffer(gl.ARRAY_BUFFER, texVertexBuffer);
            gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
                0, 1,
                0, 0,
                1, 0,
                1, 1
            ]), gl.STATIC_DRAW);

            vloc = gl.getAttribLocation(imgProgram, "aVertexPosition");
            tloc = gl.getAttribLocation(imgProgram, "aUV");
            // uLoc = gl.getUniformLocation(imgProgram, "pos");

            gl.bindTexture(gl.TEXTURE_2D, texture);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, w, h, 0, gl.RGBA, gl.UNSIGNED_BYTE, img);

            // WebGL1 has different requirements for power of 2 images
            // vs non power of 2 images so check if the image is a
            // power of 2 in both dimensions.
            if (isPowerOf2(w) && isPowerOf2(h)) {
                gl.generateMipmap(gl.TEXTURE_2D);
            }
            else {
                // wrapping to clamp to edge
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            }

            // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
            // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

            gl.enableVertexAttribArray(vloc);
            gl.bindBuffer(gl.ARRAY_BUFFER, rectVertexBuffer);
            gl.vertexAttribPointer(vloc, 2, gl.FLOAT, false, 0, 0);

            gl.enableVertexAttribArray(tloc);
            gl.bindBuffer(gl.ARRAY_BUFFER, texVertexBuffer);
            gl.vertexAttribPointer(tloc, 2, gl.FLOAT, false, 0, 0);

            // gl.uniform2fv(uLoc, [x,y]);

            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

            module._free(pbuf);
        },
    };
};

const createCtx = function(...contexts) {
    const deleters = [];
    for (const ctx of contexts) {
        if (ctx.delete) {
            deleters.push(ctx.delete);
        }
    }

    let deleter;
    switch (deleters.length) {
        case 0: deleter = function() {}; break;
        case 1: deleter = deleters[0]; break;
        default: deleter = () => { for (deleter of deleters) deleter(); }; break;
    }

    const ret = Object.assign(...contexts);
    ret.delete = deleter;
    return ret;
};

const defaultRopError = function(cmd, rop) {
    err(`${cmd}: Unsupported rop 0x${rop.toString(16).padStart(2, '0')}`);
};

const newRdpGraphics2D = function(canvasElement, module, ropError) {
    ropError = ropError || defaultRopError;
    return createCtx(newRdpCanvas(canvasElement, module, ropError),
                     newRdpPointer(canvasElement, module));
};

const newRdpGraphicsGL = function(canvasElement, module, ropError) {
    ropError = ropError || defaultRopError;
    return createCtx(newRdpGL(canvasElement, module, ropError),
                     newRdpPointer(canvasElement, module));
};

try {
    module.exports.newRdpGraphics2D = newRdpGraphics2D;
    module.exports.newRdpGraphicsGL = newRdpGraphicsGL;
    module.exports.newRdpCanvas = newRdpCanvas;
    module.exports.newRdpGL = newRdpGL;
    module.exports.newRdpPointer = newRdpPointer;
}
catch (e) {
    // module not found
}
