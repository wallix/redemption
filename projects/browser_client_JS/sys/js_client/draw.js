

var canvas_ = document.getElementById("canvas");

function Drawable() {
    this.ccnt = 0;
    this.canvas = document.getElementById("canvas");
    this.cctx = this.canvas.getContext('2d');
    this.mouseX = 0;
    this.mouseY = 0;


    /////////////////////////////////////////////////////////////////////////////////////

    //------------------------
    //      CONTROLLERS
    //------------------------

    this.canvas.addEventListener("mousedown", function(event) {
        _mousePressEvent(this.mouseX, this.mouseY,
                         event.buttons);
    });

    this.canvas.addEventListener("onmouseup", function(event) {
        _mouseReleaseEvent(this.mouseX, this.mouseY,
                           event.buttons);
    });

    this.canvas.addEventListener("onmousemove", function(event) {
        this.mouseX = event.pageX - canvas_.offsetLeft;
        this.mouseY = event.pageY - canvas_.offsetTop;
        _mouseReleaseEvent(this.mouseX, this.mouseY);
    });
}



Drawable.prototype._ctxS = function(){
    this.cctx.save();
    this.ccnt += 1;
}

Drawable.prototype._ctxR = function() {
    this.cctx.restore();
    this.ccnt -= 1;
}

Drawable.prototype._reset = function() {
    while (this.ccnt > 0) {
        this.cctx.restore();
        this.ccnt -= 1;
    }
    this.cctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
}

Drawable.prototype._color2RGBA = function(c, bpp) {
    switch (bpp) {
    case 16:
        return [((c >> 8) & 0xf8) | ((c >> 13) & 0x7),
                ((c >> 3) & 0xfc) | ((c >> 9) & 0x3),
                ((c << 3) & 0xf8) | ((c >> 2) & 0x7),
                255];
    case 24:
        return [c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, 255];

    case 32:
        return [c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF];
    }
}

Drawable.prototype._color2s = function(c, bpp) {
    var cb = this._color2RGBA(c, bpp);
    return 'rgba' + '(' + cb[0] + ',' + cb[1] + ',' + cb[2] + ','
            + ((0.0 + cb[3]) / 255) + ')';
}



/////////////////////////////////////////////////////////////////////////////////////

//-----------------------------
//       DRAW FUNCTIONS
//-----------------------------

Drawable.prototype.opaqueRect = function(x, y, w, h, c) {
    this._ctxS();
    this.cctx.fillStyle = this._color2s(c, 24);
    this.cctx.fillRect(x, y, w, h);
    this._ctxR();
}

Drawable.prototype.lineTo = function(x1, y1, x2, y2, r, g, b) {
    this._ctxS();
    this.cctx.beginPath();
    this.cctx.moveTo(x1,y1);
    this.cctx.lineTo(x2,y2);
    this.cctx.fillStyle = "rgba("+r+", "+g+", "+b+", 255)";
    this.cctx.stroke();
    this._ctxR();
}

Drawable.prototype.rDPScrBlt = function(sx, sy, dx, dy, w, h) {
    this._ctxS();
    this.cctx.putImageData(this.cctx.getImageData(sx,
                    sy, w, h), dx+1, dy+1);
    this._ctxR();
}

Drawable.prototype.rDPScrBlt_Invert = function(sx, sy, dx, dy, w, h) {
    this._ctxS();
    var imgData=this.cctx.getImageData(sx, sy, w, h);

    var size = w * h;

    for (var i = 0; i < size; i++) {
        imgData.data[i+0]= imgData.data[i+0] ^ 0xff;
        imgData.data[i+1]= imgData.data[i+1] ^ 0xff;
        imgData.data[i+2]= imgData.data[i+2] ^ 0xff;
        imgData.data[i+3]= 255;
    }

    this.cctx.putImageData(imgData, dx+1, dy+1);
    this._ctxR();
}

Drawable.prototype.RDPPatBlt_0xF0 = function(dx, dy, w, h, r, g, b) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h);

    var dw = w*4;
    var shift = 0;
    for (var y = 0; y < h; y++) {
        shift = (y%2)*4;
        for (var x = 0; x < dw; x += 8) {
            var i = y*dw + x + shift;
            imgData.data[i+0]= b;
            imgData.data[i+1]= g;
            imgData.data[i+2]= r;
            imgData.data[i+3]= 255;
        }
    }

    this.cctx.putImageData(imgData, dx, dy);
    this._ctxR();
}

Drawable.prototype.RDPPatBlt_0x5A = function(dx, dy, w, h, r_back, g_back, b_back, r_fore, g_fore, b_fore) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h);

    var dw = w*4;
    var shift = 0;
    for (var y = 0; y < h; y++) {
        shift = (y%2)*4;
        for (var x = 0; x < dw; x += 8) {
            var i = y*dw + x + shift;
            imgData.data[i+0]= b_back ^ imgData.data[i+0];
            imgData.data[i+1]= g_back ^ imgData.data[i+1];
            imgData.data[i+2]= r_back ^ imgData.data[i+2];
            imgData.data[i+3]= 255;
        }
    }
    for (var y = 0; y < h; y++) {
        shift = (y%2)*4;
        for (var x = 1; x < dw; x += 8) {
            var i = y*dw + x + shift;
            imgData.data[i+0]= b_fore ^ imgData.data[i+0];
            imgData.data[i+1]= g_fore ^ imgData.data[i+1];
            imgData.data[i+2]= r_fore ^ imgData.data[i+2];
            imgData.data[i+3]= 255;
        }
    }

    this.cctx.putImageData(imgData, dx, dy);
    this._ctxR();
}

Drawable.prototype.bitmap = function(x, y, w, h, data, shift) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = w*4;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<h+1; dy++) {
        for (var dx=0; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (h-dy)*dw + dx;
            imgData.data[i+0]= data[j+2];
            imgData.data[i+1]= data[j+1];
            imgData.data[i+2]= data[j+0];
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = (sx+w)*4;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (sy+h-dy)*dw + dx;
            imgData.data[i+0]= data[j+2];
            imgData.data[i+1]= data[j+1];
            imgData.data[i+2]= data[j+0];
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt_0x22 = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = (sx+w)*4;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (sy+h-dy)*dw + dx;
            imgData.data[i+0]= imgData.data[i+0] & ~(data[j+2]);
            imgData.data[i+1]= imgData.data[i+1] & ~(data[j+1]);
            imgData.data[i+2]= imgData.data[i+2] & ~(data[j+0]);
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt_0x55 = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = (sx+w)*4;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (sy+h-dy)*dw + dx;
            imgData.data[i+0]= 0xff ^ data[j+2];
            imgData.data[i+1]= 0xff ^ data[j+1];
            imgData.data[i+2]= 0xff ^ data[j+0];
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt_0x66 = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = (sx+w)*4;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (sy+h-dy)*dw + dx;
            imgData.data[i+0]= imgData.data[i+0] ^ data[j+2];
            imgData.data[i+1]= imgData.data[i+1] ^ data[j+1];
            imgData.data[i+2]= imgData.data[i+2] ^ data[j+0];
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMem3Blt_0xB8 = function(x, y, w, h, data, shift, back_color) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = w*4;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<h+1; dy++) {
        for (var dx=0; dx<dw; dx+=4) {
            i = dy*dw + dx;
            j = (h-dy)*dw + dx;

            var r = data[j+2];
            var g = data[j+1];
            var b = data[j+0];

            if ( (r + (g << 8) + (b << 16)) != back_color) {
                imgData.data[i+0]= r;
                imgData.data[i+1]= g;
                imgData.data[i+2]= b;
                imgData.data[i+3]= 255;
            }
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}




var drawable = new Drawable();

//_run_main();


