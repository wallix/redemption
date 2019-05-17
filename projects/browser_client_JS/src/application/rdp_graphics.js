const rgbToCss = function(color) {
    return '#'+color.toString(16).padStart(6, '0');
};

class RDPGraphics
{
    constructor(canvasElement) {
        this.ecanvas = canvasElement
        this.canvas = canvasElement.getContext('2d');
        this.ecusorCanvas = document.createElement('canvas')
        this.cusorCanvas = this.ecusorCanvas.getContext('2d');
        this.cachePointers = [];

        this.canvas.imageSmoothingEnabled = false;
    }

    resizeCanvas(w, h) {
        this.ecanvas.width = w;
        this.ecanvas.height = h;
    }

    drawImage(imageData, dx, dy, dirtyX, dirtyY, dirtyWidth, dirtyHeight, rop) {
        // rop supposed to 0xCC
        this.canvas.putImageData(imageData, dx, dy, dirtyX, dirtyY, dirtyWidth, dirtyHeight);
    }

    drawRect(x, y, w, h, color) {
        // console.log('drawRect');
        this.canvas.fillStyle = rgbToCss(color);
        this.canvas.fillRect(x,y,w,h);
    }

    drawSrcBlt(sx, sy, w, h, dx, dy, rop) {
        // console.log('drawSrcBlt');
        const sourceImageData = this.canvas.getImageData(sx, sy, w, h);
        if (rop === 0xCC) {
            this.canvas.putImageData(sourceImageData, dx, dy);
        }
        else {
            let op;
            switch (rop) {
                case 0x00: op = 'darken'; break;
                case 0xF0: op = 'source-over'; break;
                case 0x55: op = 'xor'; break;
                case 0xFF: op = 'lighten'; break;
            }
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

    drawLineTo(backMode, startX, startY, endX, endY, backColor, penStyle, penWidth, penColor) {
        // console.log('drawLineTo');
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
    }

    drawPolyline(startX, startY, deltas, penColor) {
        // console.log('drawPolyline');
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
