const rgbToCss = function(color) {
    return '#'+color.toString(16).padStart(6, '0');
};

class RDPGraphics
{
    constructor(canvasElement, cursorCanvasElement) {
        this.ecanvas = canvasElement
        this.canvas = canvasElement.getContext('2d');
        this.ecusorCanvas = cursorCanvasElement
        this.cusorCanvas = cursorCanvasElement.getContext('2d');
        this.cachePointers = [];
    }

    resizeCanvas(w, h) {
        this.ecanvas.width = w;
        this.ecanvas.height = h;
    }

    drawImage(...args) {
        this.canvas.putImageData(...args);
    }

    drawRect(x, y, w, h, color) {
        // console.log('drawRect');
        this.canvas.fillStyle = rgbToCss(color);
        this.canvas.fillRect(x,y,w,h);
    }

    drawSrcBlt(sx, sy, w, h, dx, dy, rop) {
        // console.log('drawSrcBlt');
        const sourceImageData = this.canvas.getImageData(sx, sy, w, h);
        if (rop == 0xCC) {
            this.canvas.putImageData(sourceImageData, dx, dy);
        }
        else {
            const op = scrROp[rop]
            if (op) {
                if (has_intersection(sx,sy,w,h,dx,dy,w,h)) {
                    // console.log(sx,sy,dx,dy,w,h);
                    this.canvas.globalCompositeOperation = op
                    this.canvas.putImageData(sourceImageData, 0, 0, w, h, dx, dy, w, h);
                    this.canvas.globalCompositeOperation = 'source-over'
                }
                else {
                    this.canvas.putImageData(sourceImageData, dx, dy);
                }
            }
        }
    }

    drawLineTo(backMode, nXStart, nYStart, nXEnd, nYEnd, backColor, penStyle, penWidth, penColor) {
        // console.log('drawLineTo');
		this.canvas.save();
		this.canvas.beginPath();
		this.canvas.moveTo(nXStart, nYStart);
		this.canvas.lineTo(nXEnd, nYEnd);
		this.canvas.lineWidth = penWidth;
		this.canvas.fillStyle = rgbToCss(backColor);
		this.canvas.strokeStyle = rgbToCss(penColor);
		switch (penStyle) {
            case 1: this.canvas.setLineDash([ 10, 6, 10, 6 ]); break;
            case 2: this.canvas.setLineDash([ 3, 3, 3, 3 ]); break;
            case 3: this.canvas.setLineDash([ 9, 6, 3, 6 ]); break;
            case 4: this.canvas.setLineDash([ 9, 3, 3, 3 ]); break;
            default: this.canvas.setLineDash([ 16, 0, 16, 0 ]);
		}
		this.canvas.setLineDash([ 16, 0, 16, 0 ]);
		// BackMode does not imply the transparency level of what is about too be drawn
        // canvas.globalAlpha = (backMode == 1 /* TRANSPARENT */? 0.0 : 1.0);
		this.canvas.stroke();
		this.canvas.restore();
    }

    drawPolyline(xStart, yStart, deltas, penColor) {
        // console.log('drawPolyline');
		this.canvas.save();
		this.canvas.strokeStyle = rgbToCss(penColor);
		this.canvas.beginPath();
		this.canvas.moveTo(xStart, yStart);
        let endx = xStart;
        let endy = yStart;
        const iend = deltas.length
		for (let i = 0; i < iend; i += 2) {
			endx += deltas[i];
			endy += deltas[i+1];
			this.canvas.lineTo(endx, endy);
		}
		this.canvas.stroke();
		this.canvas.restore();
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
        this.ecusorCanvas.style.cursor = this.image2CSS(image, x, y);
    }

    newPointer(image, offset, x, y) {
        const data = this.image2CSS(image, x, y);
        this.cachePointers[offset] = data;
        this.ecusorCanvas.style.cursor = data;
    }

    cachedPointer(offset) {
        this.ecusorCanvas.style.cursor = this.cachePointers[offset];
    }
};
