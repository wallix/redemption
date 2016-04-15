

function Drawable() {

    this.ccnt = 0;

    this.canvas = document.getElementById("canvas");
    this.cctx = this.canvas.getContext('2d');

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


Drawable.prototype.opaqueRect = function(x, y, w, h, c) {
    this._ctxS();
    this.cctx.fillStyle = this._color2s(c, 24);
    this.cctx.fillRect(x, y, w, h);
    this._ctxR();
}


Drawable.prototype.rDPScrBlt = function(sx, sy, x, y, w, h) {
    /*this._ctxS();
    if (this._sROP(rop)) {
        if (this._ckclp(x, y) && this._ckclp(x + w, y + h)) {
            // No clipping necessary
            this.cctx.putImageData(this.cctx.getImageData(sx,
                    sy, w, h), x, y);
        } else {
            // Clipping necessary
            this.cctx.putImageData(this.cctx.getImageData(sx,
                    sy, w, h), 0, 0);
            //this.cctx.drawImage(this.bstore, 0, 0, w, h, x, y,
                    //w, h);
        }
    }
    this._ctxR();*/
}


Drawable.prototype.bitmap = function(x, y, w, h, data) {
    this._ctxS();

    /*var u8_2 = new Uint8Array(atob( data).split("").map(function(c) {
    return c.charCodeAt(0);
    }));*/
    //console.log(data);
    //var b64encoded = btoa(String.fromCharCode.apply(null, data));
    /*
    var b64encoded = String.fromCharCode.apply(null, data);

    var img = new Image();

    img.src = "data:image/jpeg;base64," + b64encoded;

    img.onload = function () {
        this.cctx.drawImage(img, 0, 0, w, h, x, y, w, h);
    };
    img.onerror = function (stuff) {
        console.log("Img Onerror:", stuff);
    };*/

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
    this.cctx.putImageData(imgData, x, y-1, -3, 1, w, h+1);

    this._ctxR();
}



var drawable = new Drawable();
