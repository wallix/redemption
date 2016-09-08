




/////////////////////////////////////////////////////////////////////////////////////

//------------------------
//  KEYBOARD CONTROLLERS
//------------------------

var textField_ = document.getElementById("hidden_input_text");

function setFocuxOnText() {
    textField_.focus();
}

var textLength = 0;

textField_.addEventListener("keyup", function(event) {
    var str  = textField_.value;
    var size = str.length;

    switch (event.keyCode) {

        case 8:  extLength = size;
                 _backspacePressed();
        break;

        case 13: textField_.value = "";
                 extLength = 0;
                 _enterPressed();
        break;

        default: if (textLength < size) {
                    var char = str.charCodeAt(size-1);
                    _charPressed(char);
                    extLength = size;
                 } else if (textLength > size || size == 0) {
                    _backspacePressed();
                    extLength = size;
                 }
        break;

    }
});

function CTRL_ALT_DELETE() {
    _CtrlAltDelPressed();
}

function refreshPressed() {
    _refreshPressed();
}



/////////////////////////////////////////////////////////////////////////////////////

//------------------------
//   CURSOR CONTROLLERS
//------------------------

var canvas_ = document.getElementById("canvas");

/* test */
canvas_.addEventListener("mouseup", function(event) {
    _mouseReleaseEvent(event.pageX - canvas_.offsetLeft,
                        event.pageY - canvas_.offsetTop,
                        event.buttons);
});

canvas_.addEventListener("mousedown", function(event) {
    _mousePressEvent(event.pageX - canvas_.offsetLeft,
                        event.pageY - canvas_.offsetTop,
                        event.buttons);
});

canvas_.addEventListener("mousemove", function(event) {
    _mouseMoveEvent(event.pageX - canvas_.offsetLeft,
                    event.pageY - canvas_.offsetTop);
});
/* test */

/*
canvas_.addEventListener("click", function(event) {
    _mousePressEvent(event.pageX - canvas_.offsetLeft,
                        event.pageY - canvas_.offsetTop,
                        event.buttons);
    _mouseReleaseEvent(event.pageX - canvas_.offsetLeft,
                        event.pageY - canvas_.offsetTop,
                        event.buttons);
});

canvas_.addEventListener("touchmove", function(event) {
    _mouseMoveEvent(event.pageX - canvas_.offsetLeft,
                    event.pageY - canvas_.offsetTop);
});*/




/////////////////////////////////////////////////////////////////////////////////////

//------------------------
//  Draw Factorizations
//------------------------

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
        imgData.data[i+0] ^= 0xff;
        imgData.data[i+1] ^= 0xff;
        imgData.data[i+2] ^= 0xff;
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
            imgData.data[i+0] ^= b_back;
            imgData.data[i+1] ^= g_back;
            imgData.data[i+2] ^= r_back;
            imgData.data[i+3]= 255;
        }
    }
    for (var y = 0; y < h; y++) {
        shift = (y%2)*4;
        for (var x = 1; x < dw; x += 8) {
            var i = y*dw + x + shift;
            imgData.data[i+0] ^= b_fore;
            imgData.data[i+1] ^= g_fore;
            imgData.data[i+2] ^= r_fore;
            imgData.data[i+3]= 255;
        }
    }

    this.cctx.putImageData(imgData, dx, dy);
    this._ctxR();
}

Drawable.prototype.bitmap = function(x, y, w, h, data, shift) {
    this._ctxS();
    var imgData = this.cctx.createImageData(w, h+1);
    var dw4 = w*4;
    var dw3 = w*3;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<h+1; dy++) {
        j = (h-dy)*dw3;
        for (var dx=0; dx<dw4; dx+=4) {
            i = dy*dw4 + dx;
            imgData.data[i+2]= data[j++];
            imgData.data[i+1]= data[j++];
            imgData.data[i+0]= data[j++];
            imgData.data[i+3]= 255;
        }
    }

    //imgData.data = data;

    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);

    var dw = (sx+w)*4;
    var dw3 = (sx+w)*3;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        j = (sy+h-dy)*dw3 + sx*3;
        // TODO sx * 4 ?
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            imgData.data[i+0]= data[j++];
            imgData.data[i+1]= data[j++];
            imgData.data[i+2]= data[j++];
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
    var dw3 = (sx+w)*3;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        j = (sy+h-dy)*dw3 + sx*3;
        // TODO sx * 4 ?
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            imgData.data[i+0] &= ~(data[j++]);
            imgData.data[i+1] &= ~(data[j++]);
            imgData.data[i+2] &= ~(data[j++]);
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
    var dw3 = (sx+w)*3;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        j = (sy+h-dy)*dw3 + sx*3;
        // TODO sx * 4 ?
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            imgData.data[i+0]= 0xff ^ data[j++];
            imgData.data[i+1]= 0xff ^ data[j++];
            imgData.data[i+2]= 0xff ^ data[j++];
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
    var dw3 = (sx+w)*3;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        j = (sy+h-dy)*dw3 + sx*3;
        // TODO sx * 4 ?
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            imgData.data[i+0] ^= data[j++];
            imgData.data[i+1] ^= data[j++];
            imgData.data[i+2] ^= data[j++];
            imgData.data[i+3]= 255;
        }
    }
    this.cctx.putImageData(imgData, x, y-1, -shift, 1, w, h);
    this._ctxR();
}

Drawable.prototype.rDPMemBlt_0x88 = function(x, y, w, h, data, shift, sx, sy) {
    this._ctxS();
    var imgData=this.cctx.createImageData(w, h+1);
    var dw = (sx+w)*4;
    var dw3 = (sx+w)*3;
    var dh = h+1;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<dh; dy++) {
        j = (sy+h-dy)*dw3 + sx*3;
        // TODO sx * 4 ?
        for (var dx=sx; dx<dw; dx+=4) {
            i = dy*dw + dx;
            imgData.data[i+0] &= data[j++];
            imgData.data[i+1] &= data[j++];
            imgData.data[i+2] &= data[j++];
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
    var dw3 = (sx+w)*3;
    var i = 0;
    var j = 0;
    for (var dy=0; dy<h+1; dy++) {
        j = (h-dy)*dw3;
        for (var dx=0; dx<dw; dx+=4) {
            i = dy*dw + dx;

            var r = data[j++];
            var g = data[j++];
            var b = data[j++];

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




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//--------------------------------
//    SOCKET EVENTS FUNCTIONS
//--------------------------------

/* test */
var len = 250568;

var current = 0;
/* test */

/*
function connecting() {
    var ip = document.getElementById("ip").value;
    var user = document.getElementById("user").value;
    var password = document.getElementById("password").value;
    var port = document.getElementById("port").value;

    if (init_socket(ip, user, password, port)) {

        document.getElementById("errorMsgDiv").style = "display:none";
        drawable.opaqueRect(0, 0, drawable.canvas.width, drawable.canvas.height, 0x00);

        var pip = allocate(intArrayFromString(ip), 'i8', ALLOC_NORMAL);
        var puser = allocate(intArrayFromString(user), 'i8', ALLOC_NORMAL);
        var ppassword = allocate(intArrayFromString(password), 'i8', ALLOC_NORMAL);

        document.getElementById("form").style = "display:none";
        document.getElementById("emscripten_canvas").style = "display:block";

        _connexion(pip, puser, ppassword, port);


        startTimer();
        //current = 0;
        getDataOctet();
        endTimer();

    } else {

        var errorDiv = document.getElementById("errorMsgDiv");
        errorDiv.textContent = "Connection failed";
        errorDiv.style = "display:block";
    }
}*/


function disconnecting() {
    document.getElementById("form").style = "display:block";
    document.getElementById("emscripten_canvas").style = "display:none";

    drawable.opaqueRect(0, 0, drawable.canvas.width, drawable.canvas.height, 0x00);

    // socket.onclose();

    _disconnection();
}
/*
function init_socket(ip, user, password, port) { // ip = string; port = int
    console.log('init_socket');

    return true;
}*/
/*
function send_to_serveur(data, size) { // data = [uint8_t];  size = size_t
    console.log('data_sent_to_serveur');
}*/


/*
function getDataOctet() {


    var i = 0;
    for (i = current; i < (current + 2000); i++) {
        if (i < len) {
            _recv_value(inData[i]);
        }
    }

    current = i;

    for (var i = 0; i < len; i++) {
        _recv_value(inData[i]);
    }


}*/

