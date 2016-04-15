var wprdp = wprdp || {}

wprdp.optypes = { 0 : 'beginpaint', 1 : 'endpaint', 2 : 'bitmap', 3 : 'opaquerect', 4 : 'setbounds', 5 : 'patblt', 6 : 'multi_opaquerect'
	, 7 : 'scrblt', 8 : 'ptr_new', 9 : 'ptr_free', 10 : 'ptr_set', 11 : 'ptr_setnull', 12 : 'ptr_setdefault'
	, 13 : 'line_to', 14 : 'polyline', 15 : 'cache_bitmap', 16 : 'memblt', 18 : 'synchronize'
	, 19 : 'cache_glyph', 20 : 'glyph_index', 21 : 'clipboard_copy', 22 : 'dstblt', 24 : 'clipboard_file_copy' };

// Data formats by type of operation, in number of bytes
wprdp.datamasks = {};
wprdp.datamasks['beginpaint'] = [1, 2, 2, 2];
wprdp.datamasks['endpaint'] = [1];
wprdp.datamasks['bitmap'] = [1, 1];
wprdp.datamasks['bitmap_item'] = [2, 2, 2, 2, 2, 2, 2, 1, 2];
wprdp.datamasks['opaquerect'] = [1, 2, 2, 2, 2, 4];
wprdp.datamasks['setbounds'] = [1, 2, 2, 2, 2];
wprdp.datamasks['patblt'] = [1, 2, 2, 2, 2, 1, 4, 4, 1, 1, 1];
wprdp.datamasks['multi_opaquerect'] = [1, 1];
wprdp.datamasks['opaquerect_item'] = [2, 2, 2, 2];
wprdp.datamasks['scrblt'] = [1, 2, 2, 2, 2, 1, 2, 2];
wprdp.datamasks['ptr_new'] = [1, 1, 2, 2];
wprdp.datamasks['ptr_free'] = [1, 1];
wprdp.datamasks['ptr_set'] = [1, 1];
wprdp.datamasks['ptr_setnull'] = [1];
wprdp.datamasks['ptr_setdefault'] = [1];
wprdp.datamasks['line_to'] = [1, 2, 2, 2, 2, 2, 4, 1, 1, 4];
wprdp.datamasks['polyline'] = [1, 2, 2, 4, 2];
wprdp.datamasks['point'] = [2, 2];
wprdp.datamasks['cache_bitmap'] = [1, 1, 2, 2, 2, 2, 1, 2];
wprdp.datamasks['memblt'] = [1, 1, 2, 2, 2, 2, 2, 2, 2];
wprdp.datamasks['synchronize'] = [1, 2, 2, 2];
wprdp.datamasks['cache_glyph'] = [1, 1, 2];
wprdp.datamasks['glyph'] = [1, 2, 2, 2, 2, 2]; // used to parse each glyph to cache
wprdp.datamasks['glyph_index'] = [1, 1, 4, 4, 2, 2, 2, 2, 2, 2, 4];
wprdp.datamasks['clipboard_copy'] = [1, 4];
wprdp.datamasks['clipboard_file_copy'] = [1, 4];
wprdp.datamasks['dstblt'] = [1, 2, 2, 2, 2];

// Array of lengths for fixed length order types
wprdp.fixedLength = {};
wprdp.fixedLength['beginpaint'] = 7;
wprdp.fixedLength['endpaint'] = 1;
wprdp.fixedLength['bitmap_item'] = 17;
wprdp.fixedLength['opaquerect'] = 13;
wprdp.fixedLength['setbounds'] = 9;
wprdp.fixedLength['opaquerect_item'] = 8;
wprdp.fixedLength['scrblt'] = 14;
wprdp.fixedLength['ptr_new'] = 6;
wprdp.fixedLength['ptr_free'] = 2;
wprdp.fixedLength['ptr_set'] = 2;
wprdp.fixedLength['ptr_setnull'] = 1;
wprdp.fixedLength['ptr_setdefault'] = 1;
wprdp.fixedLength['line_to'] = 22;
wprdp.fixedLength['point'] = 4;
wprdp.fixedLength['memblt'] = 16;
wprdp.fixedLength['synchronize'] = 1;
wprdp.fixedLength['glyph'] = 11; // used to parse each glyph to cache
wprdp.fixedLength['dstblt'] = 9;
wprdp.fixedLength['glyph_index'] = 26;

wprdp.hasconsole = (typeof console !== 'undefined' && 'debug' in console
		&& 'info' in console && 'warn' in console && 'error' in console);

wprdp.firstBeginPaintReceived = false;

wprdp.DataIterator = function (data, optype, index, offsetData) {
    this.dView = new DataView(data);
    this.initialIndex = index;
    this.indexData = index;
    this.offsetData = (!offsetData) ? 0 : offsetData;
    this.signedValue = false;

    this.currentDataArray = wprdp.datamasks[optype];
	for ( i = 0 ; i < this.indexData ; ++i ) {
        this.offsetData += this.currentDataArray[i];
    }

    this.getNext = function() {
        var result = 0;
        switch ( this.currentDataArray[this.indexData++] ) {
        case 1 :
            result = (this.signedValue === false) ? this.dView.getUint8(this.offsetData) : this.dView.getInt8(this.offsetData);
            this.offsetData += 1;
            break;
        case 2 :
            result = (this.signedValue === false) ? this.dView.getUint16(this.offsetData, true) : this.dView.getInt16(this.offsetData, true);
            this.offsetData += 2;
            break;
        case 4 :
            result = (this.signedValue === false) ? this.dView.getUint32(this.offsetData, true) : this.dView.getInt32(this.offsetData, true);
            this.offsetData += 4;
            break;
        default :
            break;
        }
        return result;
    }

    this.resetIndex = function(){
    	this.indexData = this.initialIndex;
    }

    this.getCurrentOffset = function() {
    	return this.offsetData;
    }

    this.setSignedValue = function (val) {
    	this.signedValue = val;
    }
};

var removeOn = function(string){
	return string.replace(/^on([A-Z])/, function(full, first){
		return first.toLowerCase();
	});
};

var ua = navigator.userAgent.toLowerCase(),
platform = navigator.platform.toLowerCase(),
UA = ua.match(/(opera|ie|trident|firefox|chrome|version)[\s\/:]([\w\d\.]+)?.*?(safari|version[\s\/:]([\w\d\.]+)|$)/) || [null, 'unknown', 0],
UA2 = ua.match(/(edge)[\s\/:]([\w\d\.]+)?/),
mode = (UA[1] == 'ie' || UA[1] == 'trident') && document.documentMode;
os = platform.indexOf('win') != - 1 ? 'windows' : ( platform.indexOf('linux') != - 1 ? 'linux' : 'unsupported' );

_browser = {
	name: (UA[1] == 'version') ? UA[3] : ( ( UA2 !== null && UA2[1] == 'edge' ) || UA[1] == 'trident' ? 'ie' : UA[1] ),
	version: mode || parseFloat((UA[1] == 'opera' && UA[4]) ? UA[4] : UA[2]),
	os : os
}
_browser[_browser.name] = true;

wprdp.RDP = function( url, canvas, cssCursor, useTouch) {
	var i;

	this.log = new wprdp.Log();

	this.canvas = canvas;
	this.cctx = canvas.getContext('2d');
	this.cctx.strokeStyle = 'rgba(255,255,255,0)';
	this.cctx.FillStyle = 'rgba(255,255,255,0)';

	this.bstore = $('<canvas>').get(0);
	this.bstore.width = this.canvas.width;
	this.bstore.height = this.canvas.height;

	this.bctx = this.bstore.getContext('2d');
	this.aMF = 0;
	this.Tcool = true;
	this.RDPconnected = false;
	this.pTe = null;
	this.ccnt = 0;
	this.clx = 0;
	this.cly = 0;
	this.clw = 0;
	this.clh = 0;
	this.mX = 0;
	this.mY = 0;
	this.chx = 10;
	this.chy = 10;
	this.cursors = new Array();
	this.sid = null;
	this.open = false;
	this.cssC = cssCursor;
	this.uT = useTouch;
	this.depth = 16;
	this.optype = 'undefined';
	this.caches = [];
	for (i = 0; i < 5; i++) {
		this.caches[i] = []
	}
	this.glyphCaches = [];
	for (i = 0; i < 10; i++) {
		this.glyphCaches[i] = []
	}
	if (!cssCursor) {
		this.cI = new Element('img', {
			'src' : 'images/c_default.png',
			'styles' : {
				'position' : 'absolute',
				'z-index' : 998,
				'left' : this.mX - this.chx,
				'top' : this.mY - this.chy
			}
		}).inject(document.body);
	}

	//browser identifying variables
	this.msie = window.navigator.userAgent.indexOf('MSIE ');
	this.trident = window.navigator.userAgent.indexOf('Trident/');
	this.url = url;

	//////////////////////////// EVENT HANDLING SECTION BEGIN ////////////////////////////////
	this.$events = {};

	this.addEvent = function(type, fn, internal){
		type = removeOn(type);
		if ( jQuery.inArray( fn, (this.$events[type] || []) ) == -1 ) {
			var newValue = (this.$events[type] || []);
			newValue.push(fn);
			this.$events[type] = newValue;
		}
		if (internal) fn.internal = true;
		return this;
	}

	this.addEvents = function(events){
		for (var type in events) this.addEvent(type, events[type]);
		return this;
	}

	this.toArray = function(item) {
		if (item == null) return [];
		return (Type.isEnumerable(item) && typeof item != 'string') ? (typeOf(item) == 'array') ? item : slice.call(item) : [item];
	}

	this.fireEvent = function(type, args, delay){
		type = removeOn(type);
		var events = this.$events[type];
		if (!events) return this;
		args = this.toArray(args);
		var myCallback = function(index, fn){
			if (delay) fn.delay(delay, this, args);
			else fn.apply(this, args);
		};
		jQuery.each(events, myCallback.bind(this));
		return this;
	}

	this.removeEvent = function(type, fn){
		type = removeOn(type);
		var events = this.$events[type];
		if (events && !fn.internal){
			var index =  events.indexOf(fn);
			if (index != -1) delete events[index];
		}
		return this;
	}

	this.removeEvents = function(events){
		var type;
		if (typeOf(events) == 'object'){
			for (type in events) this.removeEvent(type, events[type]);
			return this;
		}
		if (events) events = removeOn(events);
		for (type in this.$events){
			if (events && events != type) continue;
			var fns = this.$events[type];
			for (var i = fns.length; i--;) if (i in fns){
				this.removeEvent(type, fns[i]);
			}
		}

	return this;
}
	//////////////////////////// EVENT HANDLING SECTION END //////////////////////////////////

	//////////////////////////// WEBSOCKET CALLBACKS BEGIN ///////////////////////////////////
	this.lastKeydownTarget = $('#screen').get(0); // Focus on RDP canvas by default

	this.docMouseDown = function(event) {
		this.lastKeydownTarget = $(event.target).get(0); // kept to compensate the absence of focus on canvas
	}

	this.stopEvent = function(evt) {
		if (evt.preventDefault) evt.preventDefault();
		else evt.returnValue = false;

		if (evt.stopPropagation) evt.stopPropagation();
		else evt.cancelBubble = true;
	}

	this.onWSopen = function(evt) {
		this.open = true;
		this.log.setWS(this.sock);

		// Add listeners for the various input events
		$(this.canvas).bind('mousemove', this.onMm.bind(this));

		$(document).bind('mousedown', this.docMouseDown.bind(this));
		$(this.canvas).bind('mousedown', this.onMd.bind(this));

		$(this.canvas).bind('mouseup', this.onMu.bind(this));
		$(this.canvas).bind('mousewheel DOMMouseScroll', this.onMw.bind(this));
		// Disable the browser's context menu
		$(this.canvas).bind('contextmenu', function(e) {
			this.stopEvent(e);
		}.bind(this));
		// For touch devices
		if (this.uT) {
			$(this.canvas).bind('touchstart', this.onTs.bind(this));
			$(this.canvas).bind('touchend', this.onTe.bind(this));
			$(this.canvas).bind('touchmove', this.onTm.bind(this));
		}
		if (!this.cssC) {
			// Same events on pointer image
			this.cI.bind('mousemove', this.onMm.bind(this));
			this.cI.bind('mousedown', this.onMd.bind(this));
			this.cI.bind('mouseup', this.onMu.bind(this));
			this.cI.bind('mousewheel', this.onMw.bind(this));
			this.cI.bind('contextmenu', function(e) {
				this.stopEvent(e);
			}.bind(this));
			if (this.uT) {
				this.cI.bind('touchstart', this.onTs.bind(this));
				this.cI.bind('touchend', this.onTe.bind(this));
				this.cI.bind('touchmove', this.onTm.bind(this));
			}
		}

		// The keyboard events need to be attached to the
		// document, because otherwise we seem to lose them.
		document.addEventListener('keydown', this.onKd.bind(this));
		document.addEventListener('keyup', this.onKu.bind(this));

		document.addEventListener('keypress', this.onKp.bind(this));
		this.RDPconnected = true;
		this.fireEvent('connected');
	}

	this.getRDPMessage = function ( message ) {
		var prefix = message.substr(0, 2)
		var code = 0;
		if ( ( prefix == "T:" ) || ( prefix == "E:" ) ) {
			message = message.substr(3);
			var n = message.indexOf(']');
			if ( n != -1 ) {
				code = message.substr(0, n);
			}
			message = message.substr( n + 1 );
		}
		return [code, message];
	}

	this.getWSCloseMessage = function (evt) {
		var mess = 'RDP Web Socket session closed\n';
		var codes = [1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010, 1011, 1012, 1013, 1014, 1015,
		             3001, 3002, 3007, 3009, 3012, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009,, 4010,
		             4011, 4012, 4013, 4014];
		if ( codes.indexOf(evt.code) != -1 ) {
			mess += (evt.reason + '\n\nPlease reconnect');
		}

		return mess;
	}

	this.onWSclose = function(evt) {
		/*if (Browser.name == 'chrome') {
	    // Current chrome is buggy in that it does not
	    // fire WebSockets error events, so we use the
	    // wasClean flag in the close event.
	    if ((!evt.wasClean) && (!this.open)) {
	        this.fireEvent('alert', 'Could not connect to WebSockets gateway');
	    }
		}*/
		//this.open = false;
		if ( evt.code !== undefined && evt.code != 3012 ) {
			if ( evt.code !== 1000 ) {
				alert( this.getWSCloseMessage(evt) );
				this._reset();
			}
			this.RDPconnected = false;
			this.fireEvent('disconnected');

			// Timeout case
			if ( evt.code == 4000 ) {
				displayBeforeUnloadMessage = false;
				location.reload();
			}
			// Finger print issue
			else if ( evt.code == 4012 ) {
				displayBeforeUnloadMessage = false;
				window.close();
			}
		}
		else {
			this.RDPconnected = false;
			displayBeforeUnloadMessage = true;
			reasonMsg = evt.reason;
			window.close();
		}
	}

	this.onWSerr = function(evt) {
		this.open = false;
		switch (this.sock.readyState) {
		case this.sock.CONNECTING:
			this.fireEvent('alert',
					'Could not connect to WebSockets gateway');
			break;
		}
		this._reset();
	}

	this.onWSmsg = function(evt) {
		try {
			switch (typeof (evt.data)) {
			// We use text messages for alerts and debugging ...
			case 'string':
				// this.log.debug(evt.data);
				var prefix = evt.data.substr(0, 2);
				switch (prefix) {
				case "T:":
					var mess = this.getRDPMessage( evt.data );
					this._reset(parseInt(mess[0]), mess[1]);
					break;
				case "E:":
					var mess = this.getRDPMessage( evt.data );
					this._reset(parseInt(mess[0]), mess[1]);
					break;
				case 'I:':
					this.log.info(evt.data.substring(2));
					break;
				case 'W:':
					this.log.warn(evt.data.substring(2));
					break;
				case 'D:':
					this.log.debug(evt.data.substring(2));
					break;
				case 'S:':
					this.sid = evt.data.substring(2);
					break;
				case 'L:':
					document.title = evt.data.substring(2);
					break;
				}
				break;
			// ... and binary messages for the actual RDP stuff.
			case 'object':
				this._processMessages(evt.data);
				break;
			}
		} catch (err) {
			this.log.err('onWSmsg: ' + err.message);
		}

	}

	//////////////////////////// WEBSOCKET CALLBACKS END ///////////////////////////////////

	////////////////////////// MAIN RUN BEGIN ///////////////////////
	this.Run = function () {
		try {
			this.sock = new WebSocket(this.url + '&w=' + this.canvas.width
					+ '&h=' + this.canvas.height + '&k=' + wprdp.current_layout);
		} catch (err) {
			window.alert(err.message);
			windows.close();
		}
		this.sock.binaryType = 'arraybuffer';
		this.sock.onopen = this.onWSopen.bind(this);
		this.sock.onclose = this.onWSclose.bind(this);
		this.sock.onmessage = this.onWSmsg.bind(this);
		this.sock.onerror = this.onWSerr.bind(this);
	}

	////////////////////////// MAIN RUN END ///////////////////////

	this.Disconnect = function() {
		this._reset( 1000, 'Javascript disconnect' );
	}

	this.SendComb = function(comb) {
		const
		KBD_FLAGS_DOWN = 0x4000;

		//Add here all the keys from the canvas
		switch (comb) {
		case 1:
			code = 0x2a; // ctrl+alt+delete
			break;
		}

		if (this.sock.readyState == this.sock.OPEN) {
			this.log.debug('send  special combination', code);
			buf = new ArrayBuffer(12);
			a = new Uint32Array(buf);

			a[0] = 3; // WSOP_CS_SPECIALCOMB
			a[1] = code;

			this.sock.send(buf);
		}
	}

	this.SendKeyWithFlag = function(key, flag) {
		switch (key) {
		case 'Control':
			code = 0x1d; // control
			break;
		case 'Alt':
			code = 0x38; // alt
			break;
		}

		if (this.sock.readyState == this.sock.OPEN) {
			buf = new ArrayBuffer(12);
			a = new Uint32Array(buf);
			i = 0;
			a[i++] = 1; // WSOP_CS_KUPDOWN
			a[i++] = flag;
			a[i] = code;
			this.sock.send(buf);
		}
	}

	this.SendClipboard = function(buffer) {
		if (this.sock.readyState == this.sock.OPEN) {
			buf = new ArrayBuffer(4 + buffer.byteLength);
			a = new Uint8Array(buf);

			a[0] = 4; // WSOP_CS_SENDCLIPBOARD
			a[1] = 0; // Padding
			a[2] = 0; // Padding
			a[3] = 0; // Padding
			a.set( new Uint8Array(buffer), 4 );
			this.sock.send(buf);
		}
	}

	this.SendKey = function(key) {
		const
		KBD_FLAGS_DOWN = 0x4000;
		const
		KBD_FLAGS_RELEASE = 0x8000;
		//Add here all the keys from the canvas
		switch (key) {
		case 'numlock':
			code = 0x45; // numlock
			break;
		case 'capslock':
			code = 0x3a; // capslock
			break;
		default:
			var sc;
			if(wprdp['sc_firefox_' + wprdp.current_layout_str]) {
				sc = wprdp['sc_firefox_' + wprdp.current_layout_str][key];
			}
			code = sc || key;
			break;
		}

		if (this.sock.readyState == this.sock.OPEN) {
			buf = new ArrayBuffer(12);
			a = new Uint32Array(buf);
			i = 0;
			a[i++] = 1; // WSOP_CS_KUPDOWN
			a[i++] = KBD_FLAGS_DOWN;
			a[i] = code;
			this.sock.send(buf);
			i = 0;
			a[i++] = 1; // WSOP_CS_KUPDOWN
			a[i++] = KBD_FLAGS_RELEASE;
			a[i] = code;
			this.sock.send(buf);
		}
	}

	this.getMobileCmbForKey = function(key) {
		if(!key) return [];
		var res = [];
		for(var i = 0; i < key.length; i++) {
			if(wprdp['cmb_mobile_' + wprdp.current_layout_str]) {
				res = res.concat(wprdp['cmb_mobile_' + wprdp.current_layout_str][key[i]]);
			}
		}
		return res;
	}

	this.isKeyForKeyUp = function(key) {
		if(key)
			return key.indexOf('Up', key.length - 'Up'.length) !== -1;
		return false;
	}

	this.isKeyForKeyDown = function(key) {
		if(key)
			return key.indexOf('Down', key.length - 'Down'.length) !== -1;
		return false;
	}

	this.cP = function() {
		this.cI.setStyles({
			'left' : this.mX - this.chx,
			'top' : this.mY - this.chy
		});
	}
	/**
	 * Check, if a given point is inside the clipping region.
	 */
	this._ckclp = function(x, y) {
		if (this.clw || this.clh) {
			return ((x >= this.clx) && (x <= (this.clx + this.clw))
					&& (y >= this.cly) && (y <= (this.cly + this.clh)));
		}
		// No clipping region
		return true;
	}

	this._processOperation = function(data, op, offset) { // process a binary RDP message for type [op]
		this.optype = wprdp.optypes[op];
		switch (this.optype) {
			case 'beginpaint':
				offset = this._beginPaint(data, offset);
				break;
			case 'endpaint':
				this._ctxR();
				offset += wprdp.fixedLength['endpaint'];
				break;
			case 'bitmap':
				offset = this._bitmapUpdate(data, offset);
				break;
			case 'opaquerect':
				offset = this._opaqueRect(data, offset);
				break;
			case 'setbounds':
				offset = this._setBounds(data, offset);
				break;
			case 'patblt':
				offset = this._patBlt(data, offset);
				break;
			case 'multi_opaquerect':
				offset = this._multiOpaqueRect(data, offset);
				break;
			case 'scrblt':
				offset = this._scrBlt(data, offset);
				break;
			case 'ptr_new':
				offset = this._ptrNew(data, offset);
				break;
			case 'ptr_free':
				offset = this._ptrFree(data, offset);
				break;
			case 'ptr_set':
				offset = this._ptrSet(data, offset);
				break;
			case 'ptr_setnull':
				if (this.cssC) {
					$(this.canvas).css('cursor', 'none');
				} else {
					this.cI.src = 'images/c_none.png';
				}
				offset += wprdp.fixedLength['ptr_setnull'];
				break;
			case 'ptr_setdefault':
				if (this.cssC) {
					$(this.canvas).css('cursor', 'default');
				} else {
					this.chx = 10;
					this.chy = 10;
					this.cI.src = 'images/c_default.png';
				}
				offset += wprdp.fixedLength['ptr_setdefault'];
				break;
			case 'line_to':
				offset = this._lineTo(data, offset);
				break;
			case 'polyline':
				offset = this._polyline(data, offset);
				break;
			case 'cache_bitmap':
				offset = this._cacheBitmap(data, offset);
				break;
			case 'memblt':
				offset = this._memBlt(data, offset);
				break;
			case 'synchronize':
				offset = this._synchronize(data, offset);
				break;
			case 'cache_glyph':
				offset = this._cacheGlyph(data, offset);
				break;
			case 'glyph_index':
				offset = this._glyphIndex(data, offset);
				break;
			case 'clipboard_copy':
				offset = this._copyToClipboard(data, offset);
				break;
			case 'clipboard_file_copy':
				offset = this._copyFileToClipboard(data, offset);
				break;
			case 'dstblt':
				offset = this._dstBlt(data, offset);
				break;
			default:
				this.log.warn('Unknown Operation : ' + op + ', data size = ' + data.byteLength + ', offset = ' + offset);
		}

		return offset;
	}

	this._processMessage = function( data, offset ) {
		if ( offset == data.byteLength ) {
			return 0;
		} else {
			var op = new Uint8Array(data, offset, 1)[0];
			return this._processOperation( data, op, offset );
		}
	}

	this._processMessages = function( data ) { // process multiple order RDP message
		var dataOffset = 0;
		do {
			var keepOffset = dataOffset;
			dataOffset = this._processMessage( data, dataOffset );
		} while ( dataOffset < data.byteLength && dataOffset > keepOffset );
	}

	this._glyphIndex = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		const cacheId = iterator.getNext();
		const backColor = iterator.getNext();
		const foreColor = iterator.getNext();
		const bkLeft = iterator.getNext();
		const bkRight = iterator.getNext();
		const bkTop = iterator.getNext();
		const bkBottom = iterator.getNext();
		const x = iterator.getNext();
		const y = iterator.getNext();
		const len = iterator.getNext();

		const indices = new Uint8Array(data, iterator.getCurrentOffset(), len);

		this._drawGlyph(cacheId, backColor, foreColor, bkLeft, bkRight, bkTop, bkBottom, x, y, indices);

		return ( iterator.getCurrentOffset() + len );
	}

	this._cacheGlyph = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);

		var p;
		const cacheId = iterator.getNext();
		const count = iterator.getNext();
		var len = 0;
		var offsetResult = offset;

		for (p = 0; p < count; p++) {

			iterator = new wprdp.DataIterator(data, 'glyph', 0, iterator.getCurrentOffset() + len);
			iterator.setSignedValue(true);

			cacheIndex = iterator.getNext();
			var x = iterator.getNext();
			var y = iterator.getNext();
			var sx = iterator.getNext();
			var sy = iterator.getNext();
			var len = iterator.getNext();

			var cache = this.glyphCaches[cacheId];

			cache[cacheIndex] = {
				x : x,
				y : y,
				cx : sx,
				cy : sy,
				aj : new Uint8Array(data, iterator.getCurrentOffset(), len)
			};
		}

		return ( iterator.getCurrentOffset() + len );
	}

	this._memBlt = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);

		const cacheId = iterator.getNext();
		const nLeftRect = iterator.getNext();
		const nTopRect = iterator.getNext();
		const w = iterator.getNext();
		const h = iterator.getNext();
		const x = iterator.getNext();
		const y = iterator.getNext();
		const cacheIndex = iterator.getNext();

		const cache = this.caches[cacheId];
		const cacheEntry = cache[cacheIndex];
		if (cacheEntry) {
			this.bctx.putImageData(cacheEntry.bitmap, 0, 0, 0, 0, cacheEntry.w, cacheEntry.h);
			this.cctx.drawImage(this.bstore, x, y, w, h, nLeftRect, nTopRect, w, h);
		}

		return iterator.getCurrentOffset();
	}

	this._synchronize = function(data, offset) {
		if (!offset)
			offset = 0;
		// NOT IMPLEMENTED
		return ( offset + wprdp.fixedLength['synchronize'] );
	}

	this._cacheBitmap = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		const cacheId = iterator.getNext();
		const bpp = iterator.getNext();
		const w = iterator.getNext();
		const h = iterator.getNext();
		const cacheIndex = iterator.getNext();
		const compressed = iterator.getNext();
		const len = iterator.getNext();

		const bmdata = new Uint8Array(data, iterator.getCurrentOffset(), len);

		const outB = this.cctx.createImageData(w, h);
		if (compressed) {
			if (bpp == 8) {
				wprdp.decompress(1, bmdata, w, h, len, outB.data);
			} else if (bpp == 16 || bpp == 15) {
				wprdp.decompress(2, bmdata, w, h, len, outB.data);
			} else if (bpp == 24) {
				wprdp.decompress(3, bmdata, w, h, len, outB.data);
			} else {
				this.log.err('CacheBitmap: ' + bpp
						+ '-bit compression not supported');
				return;
			}
		} else {
			if (bpp == 16 || bpp == 15) {
				wprdp.dRGB162RGBA(bmdata, len, outB.data);
			} else if (bpp == 24) {
				wprdp.dRGB242RGBA(bmdata, len, outB.data);
			} else {
				this.log.err('CacheBitmap: ' + bpp
						+ '-bit bitmap not supported');
				return;
			}
		}
		wprdp.flipV(outB.data, w, h);
		var cache = this.caches[cacheId];
		cache[cacheIndex] = {
			w : w,
			h : h,
			bitmap : outB
		};

		return ( iterator.getCurrentOffset() + len );
	}

	this._ptrSet = function (data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		var cid = iterator.getNext();

		if (this.cssC) {
			$(this.canvas).css('cursor',
					this.cursors[cid]);
		} else {
			var cobj = this.cursors[cid];
			this.chx = cobj.x;
			this.chy = cobj.y;
			this.cI.src = cobj.u;
		}

		return iterator.getCurrentOffset();
	}

	this._ptrNew = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);

		var cid = iterator.getNext();
		var xpos = iterator.getNext();
		var ypos = iterator.getNext();

		if (this.cssC) {
			var url = window.location.pathname + '?s=' + this.sid + '&c=' + cid;

			if ( this.msie > 0 || this.trident > 0 )
				this.cursors[cid] = 'url(' + url + '), default';
			else {
				var outobj = this;
				var xhr = new XMLHttpRequest();
			    xhr.open('GET', url, true);
			    xhr.responseType = 'blob';
			    xhr.onload = function() {
			      var reader  = new FileReader();
			      reader.onloadend = function () {
			    	  outobj.cursors[cid] = 'url(' + reader.result + ') ' + xpos + ' ' + ypos + ', default';
			      }
			      reader.readAsDataURL(xhr.response);
			    };
			    xhr.send();
			}
		} else {
			this.cursors[cid] = (this.msie > 0 || this.trident > 0) ? {
				u : window.location.pathname + '?s=' + this.sid + '&c=' + cid
			}
					: {
						u : window.location.pathname + '?s=' + this.sid + '&c='
								+ cid,
						x : xpos,
						y : ypos
					};
		}

		return iterator.getCurrentOffset();
	}

	this._ptrFree = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		this.cursors[iterator.getNext()] = undefined;

		return iterator.getCurrentOffset();
	}

	this._scrBlt = function (data, offset) {
		if ( !offset )
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		const x = iterator.getNext();
		const y = iterator.getNext();
		const w = iterator.getNext();
		const h = iterator.getNext();
		var rop = iterator.getNext();

		if ( rop < 0 ) {
			rop = rop + 256;
		}

		const sx = iterator.getNext();
		const sy = iterator.getNext();

		if ((w > 0) && (h > 0)) {
			this._ctxS();
			if (this._sROP(rop)) {
				if (this._ckclp(x, y) && this._ckclp(x + w, y + h)) {
					// No clipping necessary
					this.cctx.putImageData(this.cctx.getImageData(sx,
							sy, w, h), x, y);
				} else {
					// Clipping necessary
					this.bctx.putImageData(this.cctx.getImageData(sx,
							sy, w, h), 0, 0);
					this.cctx.drawImage(this.bstore, 0, 0, w, h, x, y,
							w, h);
				}
			}
			this._ctxR();
		} else {
			this.log.warn('ScrBlt: width and/or height is zero');
		}

		return iterator.getCurrentOffset();
	}

	this._setBounds = function (data, offset) {
		if ( !offset )
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		const x = iterator.getNext();
		const y = iterator.getNext();
		const w = iterator.getNext() - x;
		const h = iterator.getNext() - y;

		if (x == 0 && y == 0 && w == 0 && h == 0) {
			this._ctxR();
			this.clx = 0;
			this.cly = 0;
			this.clw = this.canvas.width;
			this.clh = this.canvas.height;
		} else {
			this._ctxS();
			this._cR(x, y, w + 1, h + 1);
			this.clx = x;
			this.cly = y;
			this.clw = w + 1;
			this.clh = h + 1;
		}

		return iterator.getCurrentOffset();
	}

	this._beginPaint = function(data, offset) {
		if ( !offset )
			offset = 0;

		this._ctxS();

		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		const w = iterator.getNext();
		const h = iterator.getNext();
		this.depth = iterator.getNext();

		if (this.canvas.width != w) {
			this.canvas.width = w;
		}
		if (this.canvas.height != h) {
			this.canvas.height = h;
		}

		// To be replaced by solution based upon monitor_ready event
		if ( wprdp.firstBeginPaintReceived == false ) {
			//do not send numlock if we are in touch device
			if(!this._isTouchDevice()) {
				this.SendKey('numlock');
				$('#numlock a').addClass('enabled');
				wprdp.firstBeginPaintReceived = true;
			}
		}

		return iterator.getCurrentOffset();
	}

	this._isTouchDevice = function() {
		try {
	  		document.createEvent("TouchEvent");
	  		return true;
  		} catch (e) {
	  		return false;
  		}
	}

	this._copyToClipboard = function (data, offset) {
		if (!offset)
			offset = 0;
		var localView = new DataView(data);
	    // var clip = document.createEvent('CustomEvent');

	    var clipboardLength = localView.getInt32(offset + 2, true);
	    var arrayLength = (clipboardLength - 2)/2 ; // trimming extra 0 two bytes at the end
	    var array = new Uint16Array(data, offset + 6, arrayLength);

	    var clip = document.createEvent('CustomEvent');
	    clip.initCustomEvent('clipboardevent', true, false, {clipcontent:array});
	    // var clip = new CustomEvent('clipboardevent', { detail:{clipcontent:array} , bubbles:true, cancelable:false});
	    clip.preventDefault();
	    window.dispatchEvent (clip);

	    return ( offset + 6 + clipboardLength );
	}

	this._copyFileToClipboard = function (data, offset) {
		if (!offset)
			offset = 0;
		var localView = new DataView(data);
		var numFiles = localView.getInt32(offset + 1, true);
		var clip = document.createEvent('CustomEvent');
		clip.initCustomEvent('clipboardfileevent', true, false, {numfiles:numFiles});
		// var clip = new CustomEvent('clipboardfileevent', { detail:{numfiles:numFiles} , bubbles:true, cancelable:false});
		clip.preventDefault();
	    window.dispatchEvent (clip);
	}

	this._bitmapUpdate = function (data, offset) {
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		var count = iterator.getNext();

		var b;
		var len = 0;

		for (b = 0; b < count; b++) {
			iterator = new wprdp.DataIterator(data, 'bitmap_item', 0, iterator.getCurrentOffset() + len );

			var dl = iterator.getNext();
			var dt = iterator.getNext();
			var dr = iterator.getNext();
			var db = iterator.getNext();
			var w = iterator.getNext();
			var h = iterator.getNext();
			var bpp = iterator.getNext();
			var compressed = (iterator.getNext() != 0);
			var len = iterator.getNext();

			var bmdata = new Uint8Array(data, iterator.getCurrentOffset(), len);

			if (this._ckclp(dl, dt) && this._ckclp(dr, db)) {
				var outB = this.cctx.createImageData(w, h);
				if (compressed) {
					if (bpp == 8) {
						wprdp.decompress(1, bmdata, w, h, len,
								outB.data);
					} else if (bpp == 16 || bpp == 15) {
						wprdp.decompress(2, bmdata, w, h, len,
								outB.data);
					} else if (bpp == 24) {
						wprdp.decompress(3, bmdata, w, h, len,
								outB.data);
					} else {
						this.log.err('BitmapUpdate: ' + bpp
								+ '-bit compression not supported');
						return;
					}
				} else {
					if (bpp == 16 || bpp == 15) {
						wprdp.dRGB162RGBA(bmdata, len, outB.data);
					} else if (bpp == 24) {
						wprdp.dRGB242RGBA(bmdata, len, outB.data);
					} else {
						this.log.err('BitmapUpdate: ' + bpp
								+ '-bit bitmap not supported');
						return;
					}
				}
				wprdp.flipV(outB.data, w, h);
				this.cctx.putImageData(outB, dl, dt, 0, 0, dr - dl + 1, db - dt + 1);
			} else {
				// putImageData ignores the clipping region, so we must clip ourselves:
				// We first paint into a second canvas, then use drawImage (which honors clipping).
				var outB = this.bctx.createImageData(w, h);
				if (compressed) {
					if (bpp == 8) {
						wprdp.decompress(1, bmdata, w, h, len,
								outB.data);
					} else if (bpp == 16 || bpp == 15) {
						wprdp.decompress(2, bmdata, w, h, len,
								outB.data);
					} else if (bpp == 24) {
						wprdp.decompress(3, bmdata, w, h, len,
								outB.data);
					} else {
						this.log.err('BitmapUpdate: ' + bpp
								+ '-bit compression not supported');
						return;
					}
				} else {
					if (bpp == 16 || bpp == 15) {
						wprdp.dRGB162RGBA(bmdata, len, outB.data);
					} else if (bpp == 24) {
						wprdp.dRGB242RGBA(bmdata, len, outB.data);
					} else {
						this.log.err('BitmapUpdate: ' + bpp
								+ '-bit bitmap not supported');
						return;
					}
				}
				wprdp.flipV(outB.data, w, h);
				this.bctx.putImageData(outB, 0, 0, 0, 0, dr - dl + 1, db - dt + 1);
				this.cctx.drawImage(this.bstore, 0, 0, dr - dl + 1, db - dt + 1, dl, dt, dr - dl + 1, db - dt + 1);
			}
		}

		return ( iterator.getCurrentOffset() + len );
	}

	this._lineTo = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);

		const backMode = iterator.getNext();
		const nXStart = iterator.getNext();
		const nYStart = iterator.getNext();
		const nXEnd = iterator.getNext();
		const nYEnd = iterator.getNext();
		const backColor = iterator.getNext();
		const penStyle = iterator.getNext();
		const penWidth = iterator.getNext();
		const penColor = iterator.getNext();

		this._ctxS();
		this.cctx.beginPath();
		this.cctx.moveTo(nXStart, nYStart);
		this.cctx.lineTo(nXEnd, nYEnd);
		this.cctx.lineWidth = penWidth;
		this.cctx.fillStyle = this._color2s(backColor);
		this.cctx.strokeStyle = this._color2s(penColor);
		switch (penStyle) {
		case 1:
			this.cctx.setLineDash([ 10, 6, 10, 6 ]);
			break;
		case 2:
			this.cctx.setLineDash([ 3, 3, 3, 3 ]);
			break;
		case 3:
			this.cctx.setLineDash([ 9, 6, 3, 6 ]);
			break;
		case 4:
			this.cctx.setLineDash([ 9, 3, 3, 3 ]);
			break;
		default:
			this.cctx.setLineDash([ 16, 0, 16, 0 ]);
		}
		this.cctx.setLineDash([ 16, 0, 16, 0 ]);
		this.cctx.globalAlpha = (backMode == 1 /* TRANSPARENT */? 0.
				: 1.0);
		this.cctx.stroke();
		this._ctxR();

		return iterator.getCurrentOffset();
	}

	this._polyline = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		const xStart = iterator.getNext();
		const yStart = iterator.getNext();
		const penColor = iterator.getNext();
		const count = iterator.getNext();

		// Convertir les points en shorts

		this._ctxS();
		this.cctx.strokeStyle = this._color2s(penColor);
		this.cctx.beginPath();
		this.cctx.moveTo(xStart, yStart);

		var p;
		var point1, point2;

		iterator = new wprdp.DataIterator(data, 'point', 0, iterator.getCurrentOffset());
        iterator.setSignedValue(true);

        point1 = xStart;
        point2 = yStart;

		for (p = 0; p < count; p++) {
			point1 += iterator.getNext();
			point2 += iterator.getNext();

			this.cctx.lineTo(point1, point2);

			iterator.resetIndex();
		}
		this.cctx.stroke();
		this._ctxR();

		return iterator.getCurrentOffset();
	}

	this._opaqueRect = function (data, offset) {
		if ( !offset )
			offset = 0;
		var x, y, w, h, c;

		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		x = iterator.getNext();
		y = iterator.getNext();
		w = iterator.getNext();
		h = iterator.getNext();
		c = iterator.getNext();

		this._ctxS();
		this.cctx.fillStyle = this._color2s(c);
		this.cctx.fillRect(x, y, w, h);
		this._ctxR();

		return iterator.getCurrentOffset();
	}

	this._multiOpaqueRect = function(data, offset) {
		if (!offset)
			offset = 0;
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);

		const color = iterator.getNext();
		const count = iterator.getNext();

		this._ctxS();
		this.cctx.fillStyle = this._color2s(color);
		var offs = 0;
		var x, y, w, h;
		var iteratorItem = new wprdp.DataIterator(data, 'opaquerect_item', 0, iterator.getCurrentOffset());
		for (i = 0; i < count; ++i) {
			x = iteratorItem.getNext();
			y = iteratorItem.getNext();
			w = iteratorItem.getNext();
			h = iteratorItem.getNext();

			this.cctx.fillRect(x, y, w, h);

			iteratorItem.resetIndex();
		}
		this._ctxR();

		return ( iterator.getCurrentOffset() + iteratorItem.getCurrentOffset() );
	}

	this._patBlt = function(data, offset) {
		if (!offset)
			offset = 0;
		var hdr, i, nLeftRect, nTopRect, nWidth, nHeight, rop, backColor, foreColor, brushX, brushY, brushStyle;
		var x, y, bx, by, idx;

		// PatBlt
		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		nLeftRect = iterator.getNext();
		nTopRect = iterator.getNext();
		nWidth = iterator.getNext();
		nHeight = iterator.getNext();
		rop = iterator.getNext();

		if ( rop < 0 ) {
			rop = rop + 256;
		}

		backColor = iterator.getNext();
		foreColor = iterator.getNext();
		brushX = iterator.getNext();
		brushY = iterator.getNext();
		brushStyle = iterator.getNext();

		if (brushStyle == 0) {
			// Solid brush style
			this._ctxS();
			if ( rop == 0x5A /* xor */ ) {
				var cb = this._color2RGBA(foreColor);
				var imgData = this.cctx.getImageData(nLeftRect, nTopRect, nWidth, nHeight);
				var len = 4 * nWidth * nHeight;
				for (i = 0; i < len; i++) {
					if ( (i & 0x3) != 3 ) {
						imgData.data[i] ^= cb[i%4];
					}
				}
				this._sROP(0xF0 /* GDI_PATCOPY: D = P */);
				this.cctx.putImageData(imgData, nLeftRect, nTopRect);
			}
			else if (this._sROP(rop)) {
				this.cctx.fillStyle = this._color2s(foreColor);;
				this.cctx.fillRect(nLeftRect, nTopRect, nWidth, nHeight);
			}
			this._ctxR();
		} else if (brushStyle == 3) {
			const bdata = new Uint8Array(data, iterator.getCurrentOffset(), 8);
			const imgPat = this.cctx.createImageData(nWidth, nHeight);
			const cbFore = this._color2RGBA(foreColor);
			const cbBack = this._color2RGBA(backColor);
			bx = brushX;
			by = brushY;
			idx = 0;
			this._ctxS();
			for (y = 0; y < nHeight; y++) {
				bx = 0;
				for (x = 0; x < nWidth; x++) {
					if (bdata[7 - bx] & (0x80 >> (by & 0x7))) {
						imgPat.data[idx] = cbBack[0];
						imgPat.data[idx + 1] = cbBack[1];
						imgPat.data[idx + 2] = cbBack[2];
						imgPat.data[idx + 3] = cbBack[3];
					} else {
						imgPat.data[idx] = cbFore[0];
						imgPat.data[idx + 1] = cbFore[1];
						imgPat.data[idx + 2] = cbFore[2];
						imgPat.data[idx + 3] = cbFore[3];
					}

					idx += 4;
					bx = (bx + 1) & 0x7;
				}
				by = (by + 1) & 0x7;
			}
			if (rop == 0x5A /* xor */) {
				var imgData = this.cctx.getImageData(nLeftRect, nTopRect, nWidth, nHeight);
				var len = 4 * nWidth * nHeight;
				for (i = 0; i < len; i++) {
					if ((i & 0x3) != 3) {
						imgData.data[i] ^= imgPat.data[i];
					}
				}
				this._sROP(0xF0 /* GDI_PATCOPY: D = P */);
				this.cctx.putImageData(imgData, nLeftRect, nTopRect);
			} else if (this._sROP(rop)) {
				this.cctx.putImageData(imgPat, nLeftRect, nTopRect);
			}
			this._ctxR();
		} else {
			this.log
					.warn('PatBlt: Patterned brush not yet implemented');
		}

		return iterator.getCurrentOffset() + 8;
	}

	// NOT USED ?
	this._dstBlt = function(data, offset) {
		if (!offset)
			offset = 0;

		var nLeftRect, nTopRect, nWidth, nHeight;

		var iterator = new wprdp.DataIterator(data, this.optype, 1, offset);
		iterator.setSignedValue(true);

		nLeftRect = iterator.getNext();
		nTopRect = iterator.getNext();
		nWidth = iterator.getNext();
		nHeight = iterator.getNext();

		this._ctxS();
		this.cctx.fillStyle = this._color2s(0XFFFF);
		this.cctx.fillRect(nLeftRect, nTopRect, nWidth, nHeight);
		this._ctxR();

		return iterator.getCurrentOffset();
	}

	this._color2RGBA = function(c) {
		switch (this.depth) {
		case 16:
			return [((c >> 8) & 0xf8) | ((c >> 13) & 0x7),
			        ((c >> 3) & 0xfc) | ((c >> 9) & 0x3),
			        ((c << 3) & 0xf8) | ((c >> 2) & 0x7),
			        255];
		case 24:
			return [(c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF, 255];

		case 32:
			return [c & 0xFF, (c >> 8) & 0xFF, (c >> 16) & 0xFF, (c >> 24) & 0xFF];
		}
	}

	this._color2s = function(c) {
		var cb = this._color2RGBA(c);
		return 'rgba' + '(' + cb[0] + ',' + cb[1] + ',' + cb[2] + ','
				+ ((0.0 + cb[3]) / 255) + ')';
	}

	this._drawGlyph = function(cacheId, backColor, foreColor, bkLeft, bkRight, bkTop, bkBottom, x, y, indices) {
		var cacheIndex, delta, cacheEntry, idx, cx, cy, pixel;

		const w = bkRight - bkLeft;
		const h = bkBottom - bkTop;

		const len = indices.length;

		const cbFore = this._color2RGBA(foreColor);
		const cbBack = this._color2RGBA(backColor);

		const cache = this.glyphCaches[cacheId];

		const glyph = this.cctx.createImageData(w, h);

		var glyphIndex = 0;

		var glyphLeft = (x - bkLeft);
		var glyphTop = 0;
		var glyphRight = 0;
		var glyphBottom = 0;

		for (cx = 0; cx < w; cx++) {
			idx = 4 * cx;
			for (cy = 0; cy < h; cy++) {
				glyph.data[idx] = cbFore[0];
				glyph.data[idx + 1] = cbFore[1];
				glyph.data[idx + 2] = cbFore[2];
				glyph.data[idx + 3] = cbFore[3];
				delta = 0;
				for (glyphIndex = 0; glyphIndex < len;) {
					cacheIndex = indices[glyphIndex++];
					delta += indices[glyphIndex++];

					cacheEntry = cache[cacheIndex];
					glyphLeft = (x - bkLeft) + cacheEntry.x + delta ;
					glyphRight = cacheEntry.cx + glyphLeft;
					glyphTop = (y - bkTop) + cacheEntry.y;
					glyphBottom = cacheEntry.cy + glyphTop;
					// Within a glyph
					if (cx >= glyphLeft && cx < glyphRight
							&& cy >= glyphTop && cy < glyphBottom) {
						pixel = cx - glyphLeft + (cy - glyphTop)
								* cacheEntry.cx;
						if (cacheEntry.aj[pixel >> 3]
								& (0x80 >> (pixel & 0x7))) {
							glyph.data[idx] = cbBack[0];
							glyph.data[idx + 1] = cbBack[1];
							glyph.data[idx + 2] = cbBack[2];
							glyph.data[idx + 3] = cbBack[3];
						}
					}

				}

				idx += 4 * w;
			}
		}
		this.cctx.putImageData(glyph, bkLeft, bkTop);
	}

	this._cR = function(x, y, w, h) {
		// New clipping region
		this.cctx.beginPath();
		this.cctx.rect(x, y, w - 1, h - 1);
		this.cctx.clip();
	}

	this._fR = function(x, y, w, h, color) {
		return;
		if ((w < 2) || (h < 2)) {
			this.cctx.strokeStyle = color;
			this.cctx.beginPath();
			this.cctx.moveTo(x, y);
			if (w > h) {
				this.cctx.lineWidth = h;
				this.cctx.lineTo(x + w, y);
			} else {
				this.cctx.lineWidth = w;
				this.cctx.lineTo(x, y + h);
			}
			this.cctx.stroke();
		} else {
			this.cctx.fillStyle = color;
			this.cctx.fillRect(x, y, w, h);
		}
	}

	this._sROP = function(rop) {
		switch (rop) {
		case 0xF0:
			// GDI_PATCOPY: D = P
			this.cctx.globalCompositeOperation = 'source-over';
			return true;
		case 0xCC:
			// GDI_SRCCOPY: D = S
			this.cctx.globalCompositeOperation = 'destination-over';
			return true;
		case 0x5A:
			this.log.warn('Raster op 5A : not yet implemented');
			return false;
		default:
			this.log.warn('Unsupported raster op: ', rop.toString(16));
			return false;
		}
		/*
		   case 0x00EE0086:
		// GDI_SRCPAINT: D = S | D
		break;
		case 0x008800C6:
		// GDI_SRCAND: D = S & D
		break;
		case 0x00660046:
		// GDI_SRCINVERT: D = S ^ D
		break;
		case 0x00440328:
		// GDI_SRCERASE: D = S & ~D
		break;
		case 0x00330008:
		// GDI_NOTSRCCOPY: D = ~S
		break;
		case 0x001100A6:
		// GDI_NOTSRCERASE: D = ~S & ~D
		break;
		case 0x00C000CA:
		// GDI_MERGECOPY: D = S & P
		break;
		case 0x00BB0226:
		// GDI_MERGEPAINT: D = ~S | D
		break;
		case 0x00FB0A09:
		// GDI_PATPAINT: D = D | (P | ~S)
		break;
		case 0x00550009:
		// GDI_DSTINVERT: D = ~D
		break;
		case 0x00000042:
		// GDI_BLACKNESS: D = 0
		break;
		case 0x00FF0062:
		// GDI_WHITENESS: D = 1
		break;
		case 0x00E20746:
		// GDI_DSPDxax: D = (S & P) | (~S & D)
		break;
		case 0x00B8074A:
		// GDI_PSDPxax: D = (S & D) | (~S & P)
		break;
		case 0x000C0324:
		// GDI_SPna: D = S & ~P
		break;
		case 0x00220326:
		// GDI_DSna D = D & ~S
		break;
		case 0x00220326:
		// GDI_DSna: D = D & ~S
		break;
		case 0x00A000C9:
		// GDI_DPa: D = D & P
		break;
		case 0x00A50065:
		// GDI_PDxn: D = D ^ ~P
		break;
		 */
	}
	/**
	 * Reset our state to disconnected
	 */
	this._reset = function(code, reason) {
		this.log.setWS(null);
		if (this.sock.readyState == this.sock.OPEN) {
			this.fireEvent('disconnected');
			if ( code === undefined || reason === undefined)
				this.sock.close();
			else
				this.sock.close(code, reason);
		}
		this.clx = 0;
		this.cly = 0;
		this.clw = 0;
		this.clh = 0;
		$(this.canvas).unbind();
		$(document).unbind();
		while (this.ccnt > 0) {
			this.cctx.restore();
			this.ccnt -= 1;
		}
		this.cctx
				.clearRect(0, 0, this.canvas.width, this.canvas.height);
		if (this.cssC) {
			$(this.canvas).css('cursor', 'default');
		} else {
			this.cI.src = '/c_default.png';
		}
		if (!this.cssC) {
			$(this.cI).unbind();
			this.cI.destroy();
		}
	}

	this.fT = function() {
		delete this.fTid;
		if (this.pT) {
			this.fireEvent('touch' + this.pT);
			this.pT = 0;
			return;
		}
		if (this.pTe) {
			this.onMd(this.pTe);
			this.pTe = null;
		}
	}

	this.cT = function() {
		this.log.debug('cT');
		this.Tcool = true;
	}
	/**
	 * Event handler for touch start
	 */
	this.onTs = function(evt) {
		var tn = evt.targetTouches.length;
		this.log.debug('Ts:', tn);
		switch (tn) {
		default:
			break;
		case 1:
			this.pTe = evt;
			evt.preventDefault();
			if ('number' == typeof (this.fTid)) {
				clearTimeout(this.fTid);
			}
			this.fTid = this.fT.delay(50, this);
			break;
		case 2:
			this.pT = 2;
			this.Tcool = false;
			evt.preventDefault();
			if ('number' == typeof (this.fTid)) {
				clearTimeout(this.fTid);
			}
			this.cT.delay(500, this)
			this.fTid = this.fT.delay(50, this);
			break;
		case 3:
			this.pT = 3;
			this.Tcool = false;
			evt.preventDefault();
			if ('number' == typeof (this.fTid)) {
				clearTimeout(this.fTid);
			}
			this.cT.delay(500, this)
			this.fTid = this.fT.delay(50, this);
			break;
		case 4:
			this.pT = 4;
			this.Tcool = false;
			evt.preventDefault();
			if ('number' == typeof (this.fTid)) {
				clearTimeout(this.fTid);
			}
			this.cT.delay(500, this)
			this.fTid = this.fT.delay(50, this);
			break;
		}
		return true;
	}
	/**
	 * Event handler for touch start
	 */
	this.onTe = function(evt) {
		if ((0 == evt.targetTouches.length) && this.Tcool) {
			evt.preventDefault();
			this.onMu(evt, evt.changedTouches[0].pageX,
					evt.changedTouches[0].pageY);
		}
	}
	/**
	 * Event handler for touch move
	 */
	this.onTm = function(evt) {
		// this.log.debug('Tm:', evt);
		if (1 == evt.targetTouches.length) {
			this.onMm(evt);
		}
	}
	/**
	 * Event handler for mouse move events
	 */
	this.onMm = function(evt) {
		var buf, a, x, y;
		evt.preventDefault();
		var theTarget = $(evt.target);
		var theTargetRect = theTarget.get(0).getBoundingClientRect();

		//calculate the real X/Y position to send to RDP
		var layerX = (evt.pageX - theTargetRect.left) / theTargetRect.width * $(theTarget).attr('width');
		var layerY = (evt.pageY - theTargetRect.top) / theTargetRect.height * $(theTarget).attr('height');

		/*
		x = (this.msie > 0 || this.trident > 0) ? layerX
				- offset.left : layerX;
		y = (this.msie > 0 || this.trident > 0) ? layerY
				- offset.top : layerY;*/
		x = layerX;
		y = layerY;

		if (!this.cssC) {
			this.mX = x;
			this.mY = y;
			this.cP();
		}
		// this.log.debug('mM x: ', x, ' y: ', y);
		if (this.sock.readyState == this.sock.OPEN) {
			buf = new ArrayBuffer(16);
			a = new Uint32Array(buf);
			a[0] = 0; // WSOP_CS_MOUSE
			a[1] = 0x0800; // PTR_FLAGS_MOVE
			a[2] = x;
			a[3] = y;
			this.sock.send(buf);
		}
	}
	/**
	 * Event handler for mouse down events
	 */
	this.onMd = function(evt) {
		var buf, a, x, y, which;
		if (this.Tcool) {

			evt.preventDefault();
			if (evt.rightClick && evt.ctrlKey && evt.altKey) {
				this.fireEvent('touch3');
				return;
			}

			var theTarget = $(evt.target);
			var theTargetRect = theTarget.get(0).getBoundingClientRect();

			//calculate the real X/Y position to send to RDP
			var layerX = (evt.pageX - theTargetRect.left) / theTargetRect.width * $(theTarget).attr('width');
			var layerY = (evt.pageY - theTargetRect.top) / theTargetRect.height * $(theTarget).attr('height');

			/*x = (this.msie > 0 || this.trident > 0) ? layerX
					- offset.left
					: layerX;
			y = (this.msie > 0 || this.trident > 0) ? layerY
					- offset.top
					: layerY; */

			x = layerX;
			y = layerY;

			which = this._mB(evt);
			//this.log.debug('mD b: ', which, ' x: ', x, ' y: ', y);
			if (this.sock.readyState == this.sock.OPEN) {
				buf = new ArrayBuffer(16);
				a = new Uint32Array(buf);
				a[0] = 0; // WSOP_CS_MOUSE
				a[1] = 0x8000 | which;
				a[2] = x;
				a[3] = y;
				this.sock.send(buf);
			}
		}
	}
	/**
	 * Event handler for mouse up events
	 */
	this.onMu = function(evt, x, y) {
		var buf, a, x, y, which;
		if (this.Tcool) {
			evt.preventDefault();
			var theTarget = $(evt.target);
			var theTargetRect = theTarget.get(0).getBoundingClientRect();

			//calculate the real X/Y position to send to RDP
			var layerX = (evt.pageX - theTargetRect.left) / theTargetRect.width * $(theTarget).attr('width');
			var layerY = (evt.pageY - theTargetRect.top) / theTargetRect.height * $(theTarget).attr('height');
			/*x = (this.msie > 0 || this.trident > 0) ? layerX
					- offset.left
					: layerX;
			y = (this.msie > 0 || this.trident > 0) ? layerY
					- offset.top
					: layerY; */
			x = layerX;
			y = layerY;
			which = this._mB(evt);
			//this.log.debug('mU b: ', which, ' x: ', x, ' y: ', y);
			if (this.aMF) {
				this.fireEvent('mouserelease');
			}
			if (this.sock.readyState == this.sock.OPEN) {
				buf = new ArrayBuffer(16);
				a = new Uint32Array(buf);
				a[0] = 0; // WSOP_CS_MOUSE
				a[1] = which;
				a[2] = x;
				a[3] = y;
				this.sock.send(buf);
			}
		}
	}
	/**
	 * Event handler for mouse wheel events
	 */
	this.onMw = function(evt) {
		var buf, a, x, y;
		evt.preventDefault();
		var theTarget = $(evt.target);
		var offset = theTarget.offset();
		var layerX = evt.pageX - offset.left;
		var layerY = evt.pageY - offset.top;
		/*x = (this.msie > 0 || this.trident > 0) ? layerX
				- offset.left : layerX;
		y = (this.msie > 0 || this.trident > 0) ? layerY
				- offset.top : layerY; */
		x = layerX;
		y = layerY;

		var _evt = evt.originalEvent ? evt.originalEvent : evt;
		var delta = _evt.detail < 0 || _evt.wheelDelta > 0 ? 1 : -1;

		if (this.sock.readyState == this.sock.OPEN) {
			buf = new ArrayBuffer(16);
			a = new Uint32Array(buf);
			a[0] = 0; // WSOP_CS_MOUSE
			a[1] = 0x200 | ((delta > 0) ? 0x087 : 0x188);
			a[2] = 0;
			a[3] = 0;
			this.sock.send(buf);
		}
	}

	this.onKeyEvent = function(down, evt, keyId) {
		const
		KBD_FLAGS_DOWN = 0x4000;
		const
		KBD_FLAGS_RELEASE = 0x8000;

		var a, buf, sc, i;

		if (( evt.key && evt.key === 'AltGraph') || (evt.keyCode === 225)) {
			this.SendKeyWithFlag( 'Control', (down === 1 || down === 2) ? KBD_FLAGS_DOWN : KBD_FLAGS_RELEASE );
			this.SendKeyWithFlag( 'Alt', (down === 1 || down === 2) ? KBD_FLAGS_DOWN : KBD_FLAGS_RELEASE );
		}

		if ( _browser.ie ) {
			if ( evt.key === 'Unidentified' ) {
				// Discriminate by layouts
				if ( wprdp.current_layout == 0x40C && evt.keyCode === 221 ) { // FR case
					keyId = 'DeadCircumflex';
				}
				else if ( wprdp.current_layout == 0x407 && evt.keyCode === 220 ) { // DE case
					keyId = 'DeadCircumflex';
				}
				else if ( wprdp.current_layout == 0x407 && evt.keyCode === 221 ) { // DE case
					keyId = 'DeadAcute';
				}
				else if ( wprdp.current_layout == 0x407 && evt.keyCode === 219 ) { // DE case
					keyId = '\\';
				}
				else if ( wprdp.current_layout == 0x40A && evt.keyCode === 186 ) { // ES case
					keyId = 'DeadCircumflex';
				}
				else if ( wprdp.current_layout == 0x40A && evt.keyCode === 222 ) { // ES case
					keyId = 'DeadUmlaut';
				}
				else if ( wprdp.current_layout == 0x40A && evt.keyCode === 220 ) { // ES case
					keyId = '\\';
				}
				else
					return;
			}
		}
		else if ( _browser.firefox ) {
			if ( evt.key === 'Dead' ) {
				if ( evt.code ) {
					keyId = evt.code;
				}
			}
			else if ( wprdp.current_layout == 0x40C ) {
				if ( evt.key === 'è' || evt.key === 'à' || evt.key === 'ù' ) {
					if ( evt.code ) {
						keyId = evt.code;
					}
				}
			}
		}

		// Prevent keypress triggering
		evt.preventDefault();

		sc = wprdp.getScanCode( evt, keyId );
		if ( sc[1] !== undefined && this.sock.readyState == this.sock.OPEN ) {
			buf = new ArrayBuffer(12);
			i = 0;
			a = new Uint32Array(buf);

			a[i++] = 1; // WSOP_CS_KUPDOWN
			a[i++] = ( (down === 1 || down === 2) ? KBD_FLAGS_DOWN : KBD_FLAGS_RELEASE );

			a[i-1] |= sc[0];
			a[i] = sc[1];

			if ( a[1] > 0 ) {
				this.sock.send(buf);
			}
		}
	}

	this.updateGUI = function ( evt ) {
		if ( ( evt.key && evt.key === 'CapsLock') || ( evt.keyCode && evt.keyCode === 20 ) ) {
			if ( $('#capslock').hasClass('enabled') ) {
				$('#capslock').removeClass('enabled');
        	}
        	else {
        		$('#capslock').addClass('enabled');
        	}
		}
		if ( ( evt.key && evt.key === 'NumLock') || ( evt.keyCode && evt.keyCode === 144 ) ) {
			if ( $('#numlock').hasClass('enabled') ) {
				$('#numlock').removeClass('enabled');
        	}
        	else {
        		$('#numlock').addClass('enabled');
        	}
		}
	}

	this.useKeyPress = function( evt, eventType ) {
		if ( _browser.chrome ) {
			if ( _browser.os == 'linux' ) {
				var keyCodes;
				if ( wprdp.current_layout_str == 'fr' )
					keyCodes = {'Kd':{'220':''},'Kp':{'60':'','62':''}};
				else if ( wprdp.current_layout_str == 'uk' )
					keyCodes = {'Kd':{'220':''},'Kp':{'92':'', '124':''}};
				else if ( wprdp.current_layout_str == 'es' )
					keyCodes = {'Kd':{'220':''},'Kp':{'60':'', '62':''}};
				else if ( wprdp.current_layout_str == 'de' )
					keyCodes = {'Kd':{'220':''},'Kp':{'60':'', '62':''}};
				else if ( wprdp.current_layout_str == 'us' )
					keyCodes = {'Kd':{'220':''},'Kp':{'60':'', '62':''}};
				else if ( wprdp.current_layout_str == 'ru' )
					keyCodes = {'Kd':{'220':''},'Kp':{'60':'', '62':''}};

				return ( keyCodes[eventType][evt.keyCode] !== undefined );
			}
		}

		return false;
	}

	/**
	 * Event handler for key down events
	 */
	this.onKd = function(evt) {
		if ( this.lastKeydownTarget == $('#screen').get(0) ) {
			this.updateGUI( evt );
			if ( !this.useKeyPress( evt, 'Kd' ) ) {
				this.onKeyEvent(1, evt);
			}
		}
		else {
			// Esc key pressed
			if ( ( evt.key && evt.key === 'Esc') || ( evt.keyCode && evt.keyCode === 27 ) ) {
				if ( $('#clipboardDiv').css('display') === 'block' )
				{
					$('#clipboardDiv').css('display', 'none');
        			$('#showclipboard').html($('#showclipboardlabel').html());
				}
			}
		}
	}
	/**
	 * Event handler for key up events
	 */
	this.onKu = function(evt) {
		if ( this.lastKeydownTarget == $('#screen').get(0) )
			this.onKeyEvent(0, evt);
	}

	this.onKp = function(evt) {
		// Specific treatment for Internet Explorer
		// in the case of Unidentified key field in keydown callback which are set in the keypress one^
		if ( _browser.ie ) {
			var charlist = "]}¤";
			var curKey = evt.key;
			if ( charlist.indexOf(curKey) !== -1 ) {
				this.onKeyEvent(2, evt, curKey);
			}
		} else if ( this.useKeyPress( evt, 'Kp' ) ) {
			this.onKeyEvent(2, evt, evt.keyCode + 'Kp');
		}

		return;
	}

	this._c2s = function(c) {
		return 'rgba' + '(' + c[0] + ',' + c[1] + ',' + c[2] + ','
				+ ((0.0 + c[3]) / 255) + ')';
	}
	/**
	 * Save the canvas state and remember this in our object.
	 */
	this._ctxS = function() {
		this.cctx.save();
		this.ccnt += 1;
	}
	/**
	 * Restore the canvas state and remember this in our object.
	 */
	this._ctxR= function() {
		this.cctx.restore();
		this.ccnt -= 1;
	}
	/**
	 * Convert the button information of a mouse event into
	 * RDP-like flags.
	 */
	this._mB = function(evt) {
		if (this.aMF) {
			return this.aMF;
		}
		var bidx;
		if ('event' in evt && 'button' in evt.event) {
			bidx = evt.event.button;
		} else {
			bidx = (evt.which == 3 || evt.button == 2) ? 2 : 0;
		}
		switch (bidx) {
		case 0:
			return 0x1000; // left button
		case 1:
			return 0x4000; // middle button
		case 2:
			return 0x2000; // right button
		}
		return 0x1000;
	}

	this.SetArtificialMouseFlags = function(mf) {
		if (null == mf) {
			this.aMF = 0;
			return;
		}
		this.aMF = 0x1000; // left button
		if (mf.r) {
			this.aMF = 0x2000; // right
		}
		if (mf.m) {
			this.aMF = 0x4000; // middle
		}
	}
};

//////////////////////////////////////////////////////////////////////

wprdp.o2s = function(obj, depth) {
	depth = depth || [];
	if (depth.contains(obj)) {
		return '{SELF}';
	}
	switch (typeof (obj)) {
	case 'undefined':
		return 'undefined';
	case 'string':
		return '"' + obj.replace(/[\x00-\x1f\\"]/g, escape) + '"';
	case 'array':
		var string = [];
		depth.push(obj);
		for (var i = 0; i < obj.length; ++i) {
			string.push(wprdp.o2s(obj[i], depth));
		}
		depth.pop();
		return '[' + string + ']';
	case 'object':
	case 'hash':
		var string = [];
		depth.push(obj);
		var isE = (obj instanceof UIEvent);
		Object
				.each(obj,
						function(v, k) {
							if (v instanceof HTMLElement) {
								string.push(k + '={HTMLElement}');
							} else if (isE
									&& (('layerX' == k) || ('layerY' == k)
											('view' == k))) {
								string.push(k + '=!0');
							} else {
								try {
									var vstr = wprdp.o2s(v, depth);
									if (vstr) {
										string.push(k + '=' + vstr);
									}
								} catch (error) {
									string.push(k + '=??E??');
								}
							}
						});
		depth.pop();
		return '{' + string + '}';
	case 'number':
	case 'boolean':
		return '' + obj;
	case 'null':
		return 'null';
	}
	return null;
};

wprdp.Log = function(){
	this.ws = null;

	this._p = function(pfx, a) {
		var line = '';
		var i;
		for (i = 0; i < a.length; ++i) {
			switch (typeof (a[i])) {
			case 'string':
			case 'number':
			case 'boolean':
			case 'null':
				line += a[i] + ' ';
				break;
			default:
				line += wprdp.o2s(a[i]) + ' ';
				break;
			}
		}
		if (0 < line.length) {
			this.ws.send(pfx + line);
		}
	}

	this.drop = function() {
	}

	this.debug = function() {/* DEBUG */
		if (this.ws) {
			this._p('D:', arguments);
		}
		if (wprdp.hasconsole) {
			try {
				console.debug.apply(this, arguments);
			} catch (error) {
			}
		}
	/* /DEBUG */
	}

	this.info = function() {
		if (this.ws) {
			this._p('I:', arguments);
		}
		/* DEBUG */if (wprdp.hasconsole) {
			try {
				console.info.apply(this, arguments);
			} catch (error) {
			}
		}/* /DEBUG */
	}

	this.warn = function() {
		if (this.ws) {
			this._p('W:', arguments);
		}
		/* DEBUG */if (wprdp.hasconsole) {
			try {
				console.warn.apply(this, arguments);
			} catch (error) {
			}
		}/* /DEBUG */
	}

	this.err = function() {
		if (this.ws) {
			this._p('E:', arguments);
		}
		/* DEBUG */if (wprdp.hasconsole) {
			try {
				console.error.apply(this, arguments);
			} catch (error) {
			}
		}/* /DEBUG */
	}

	this.setWS = function(_ws) {
		this.ws = _ws;
	}
};

wprdp.buf2RGBA = function(inA, inI, outA, outI) {
	wprdp.pel2RGBA(inA[inI] | (inA[inI + 1] << 8), outA, outI);
}
wprdp.pel2RGBA = function(c, outA, outI) {
	outA[outI++] = ((c >> 8) & 0xf8) | ((c >> 13) & 0x7); // r1 r2 r3 r4 r5 r6 r7 r8
	outA[outI++] = ((c >> 3) & 0xfc) | ((c >> 9) & 0x3); // g1 g2 g3 g4 g5 g6 g1 g2
	outA[outI++] = ((c << 3) & 0xf8) | ((c >> 2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
	outA[outI] = 255; // alpha
}

wprdp.flipV = function(inA, width, height) {
	var sll = width * 4;
	var half = height / 2;
	var lbot = sll * (height - 1);
	var ltop = 0;
	var tmp = new Uint8Array(sll);
	var i, j;
	if ('subarray' in inA) {
		for (i = 0; i < half; ++i) {
			tmp.set(inA.subarray(ltop, ltop + sll));
			inA.set(inA.subarray(lbot, lbot + sll), ltop);
			inA.set(tmp, lbot);
			ltop += sll;
			lbot -= sll;
		}
	} else {
		for (i = 0; i < half; ++i) {
			for (j = 0; j < sll; ++j) {
				tmp[j] = inA[ltop + j];
				inA[ltop + j] = inA[lbot + j];
				inA[lbot + j] = tmp[j];
			}
			ltop += sll;
			lbot -= sll;
		}
	}
}

wprdp.dRGB162RGBA = function(inA, inLength, outA) {
	var inI = 0;
	var outI = 0;
	while (inI < inLength) {
		wprdp.buf2RGBA(inA, inI, outA, outI);
		inI += 2;
		outI += 4;
	}
}

wprdp.dRGB242RGBA = function(inA, inLength, outA) {
	var inI = 0;
	var outI = 0;
	var r, b, g;
	while (inI < inLength) {
		b = inA[inI++];
		g = inA[inI++];
		r = inA[inI++];

		outA[outI++] = r;
		outA[outI++] = g;
		outA[outI++] = b;
		outA[outI++] = 255;
	}
}

wprdp.get_pixel = function(nb, buffer, offset) {
	var value;
	var r, g, b, a;

	if (nb == 1) {
		value = (buffer[offset] < 128 ? 255 : 0);
		r = value; // r1 r2 r3 r4 r5 r6 r7 r8
		g = value; // g1 g2 g3 g4 g5 g6 g1 g2
		b = value; // b1 b2 b3 b4 b5 b1 b2 b3
		a = 255;
	} else if (nb == 2) {
		value = buffer[offset++];
		value |= buffer[offset] << 8;
		r = ((value >> 8) & 0xf8) | ((value >> 13) & 0x7); // r1 r2 r3 r4 r5 r6 r7 r8
		g = ((value >> 3) & 0xfc) | ((value >> 9) & 0x3); // g1 g2 g3 g4 g5 g6 g1 g2
		b = ((value << 3) & 0xf8) | ((value >> 2) & 0x7); // b1 b2 b3 b4 b5 b1 b2 b3
		a = 255;
	} else {
		b = buffer[offset++];
		g = buffer[offset++];
		r = buffer[offset];
		a = 255;
	}

	return r | (g << 8) | (b << 16) | (a << 24);
}

wprdp.get_pixel_from_out = function(buffer, offset) {
	return buffer[offset++] | (buffer[offset++] << 8)
			| (buffer[offset++] << 16) | (buffer[offset] << 24);
}

wprdp.put_pixel = function(buffer, offset, value) {
	buffer[offset++] = value & 0xff; // r
	buffer[offset++] = (value >> 8) & 0xff; // g
	buffer[offset++] = (value >> 16) & 0xff; // b
	buffer[offset] = (value >> 24) & 0xff; // a

	if ((value & 0xff) == 0x39 && ((value >> 8) & 0xff) == 0x6D
			&& ((value >> 16) & 0xff) == 0xA5) {
		value = value;
	}
	return 4;
}

wprdp.decompress = function(Bpp, inA, width, height, size, outA) {
	var line_size = 4 * width;
	var yprev = 0xFF000000;
	var out = 0;
	var color1 = 0xFF000000;
	var color2 = 0xFF000000;
	var mix = 0xFFFFFFFF;
	var code;
	var mask = 0;
	var fom_mask = 0;
	var count = 0;
	var opcode = 0xFF;
	var lastopcode;
	var input = 0;
	var i;

	while (input < size) {
		// Read RLE operators, handle short and long forms
		code = inA[input++];

		switch (code >> 4) {
		case 0xf:
			switch (code) {
			case 0xFD:
				opcode = 13; // WHITE;
				count = 1;
				break;
			case 0xFE:
				opcode = 14; // BLACK;
				count = 1;
				break;
			case 0xFA:
				opcode = 10;// SPECIAL_FGBG_2;
				count = 8;
				break;
			case 0xF9:
				opcode = 9;// SPECIAL_FGBG_1;
				count = 8;
				break;
			case 0xF8:
				opcode = code & 0xf;
				count = inA[input++];
				count |= (inA[input++] << 8);
				count += count;
				break;
			default:
				opcode = code & 0xf;
				count = inA[input++];
				count |= (inA[input++] << 8);
				// Opcodes 0xFB, 0xFC, 0xFF are some unknown orders of length 1 ?
				break;
			}
			break;
		case 0x0e: // Bicolor, short form (1 or 2 bytes)
			opcode = 8;// BICOLOR;
			count = code & 0xf;
			if (!count) {
				count = inA[input++] + 16;
			}
			count += count;
			break;
		case 0x0d: // FOM SET, short form (1 or 2 bytes)
			opcode = 7;// FOM_SET;
			count = code & 0x0F;
			if (count) {
				count <<= 3;
			} else {
				count = inA[input++] + 1;
			}
			break;
		case 0x05:
		case 0x04: // FOM, short form (1 or 2 bytes)
			opcode = 2;// FOM;
			count = code & 0x1F;
			if (count) {
				count <<= 3;
			} else {
				count = inA[input++] + 1;
			}
			break;
		case 0x0c: // MIX SET, short form (1 or 2 bytes)
			opcode = 6;// MIX_SET;
			count = code & 0x0f;
			if (!count) {
				count = inA[input++] + 16;
			}
			break;
		default:
			opcode = (code >> 5) & 0xFF; // FILL, MIX, FOM, COLOR, COPY
			count = code & 0x1f;
			if (!count) {
				count = inA[input++] + 32;
			}
			break;
		}

		/* Read preliminary data */
		switch (opcode) {
		case 2 /* FOM */:
			mask = 1;
			fom_mask = inA[input++];
			break;
		case 9 /* SPECIAL_FGBG_1 */:
			mask = 1;
			fom_mask = 3;
			break;
		case 10 /* SPECIAL_FGBG_2 */:
			mask = 1;
			fom_mask = 5;
			break;
		case 8 /* BICOLOR */:
			color1 = wprdp.get_pixel(Bpp, inA, input);
			input += Bpp;
			color2 = wprdp.get_pixel(Bpp, inA, input);
			input += Bpp;
			break;
		case 3 /* COLOR */:
			color2 = wprdp.get_pixel(Bpp, inA, input);
			input += Bpp;
			break;
		case 6 /* MIX_SET */:
			mix = wprdp.get_pixel(Bpp, inA, input);
			input += Bpp;
			break;
		case 7 /* FOM_SET */:
			mix = wprdp.get_pixel(Bpp, inA, input);
			input += Bpp;
			mask = 1;
			fom_mask = inA[input++];
			break;
		// for FILL, MIX or COPY nothing to do here
		}

		// MAGIC MIX of one pixel to comply with crap in Bitmap RLE compression
		if (opcode == 0 /* FILL */&& opcode == lastopcode && out != line_size) {
			yprev = (out < line_size) ? 0xFF000000 : wprdp.get_pixel_from_out(
					outA, out - line_size);
			out += wprdp.put_pixel(outA, out, (yprev ^ mix) | 0xFF000000);
			count--;
		}

		lastopcode = opcode;

		/* Output body */
		switch (opcode) {
		case 0 /* FILL */:
			i = 0;
			for (; i < count && out < line_size; i++) {
				out += wprdp.put_pixel(outA, out, 0xFF000000);
			}
			for (; i < count; i++) {
				yprev = wprdp.get_pixel_from_out(outA, out - line_size);
				out += wprdp.put_pixel(outA, out, yprev);
			}
			break;
		case 6 /* MIX_SET */:
		case 1 /* MIX */:
			i = 0;
			for (; i < count && out < line_size; i++) {
				out += wprdp.put_pixel(outA, out, mix);
			}
			for (; i < count; i++) {
				yprev = wprdp.get_pixel_from_out(outA, out - line_size);
				out += wprdp.put_pixel(outA, out, (yprev ^ mix) | 0xFF000000);
			}
			break;
		case 2 /* FOM */:
		case 7 /* FOM_SET */:
		case 9 /* SPECIAL_FGBG_1 */:
		case 10 /* SPECIAL_FGBG_2 */:
			i = 0;
			for (; i < count && out < line_size; i++) {
				if (mask == 0x100) {
					mask = 1;
					fom_mask = inA[input++];
				}
				if (mask & fom_mask) {
					out += wprdp.put_pixel(outA, out, mix);
				} else {
					out += wprdp.put_pixel(outA, out, 0xFF000000);
				}
				mask <<= 1;
			}
			for (; i < count; i++) {
				yprev = wprdp.get_pixel_from_out(outA, out - line_size);
				if (mask == 0x100) {
					mask = 1;
					fom_mask = inA[input++];
				}
				if (mask & fom_mask) {
					out += wprdp.put_pixel(outA, out,
							(yprev ^ mix) | 0xFF000000);
				} else {
					out += wprdp.put_pixel(outA, out, yprev);
				}
				mask <<= 1;
			}
			break;
		case 3 /* COLOR */:
			for (i = 0; i < count; i++) {
				out += wprdp.put_pixel(outA, out, color2);
			}
			break;
		case 4 /* COPY */:
			for (i = 0; i < count; i++) {
				out += wprdp.put_pixel(outA, out, wprdp.get_pixel(Bpp, inA,
						input));
				input += Bpp;
			}
			break;
		case 8 /* BICOLOR */:
			for (i = 0; i < count; i+=2) {
				out += wprdp.put_pixel(outA, out, color1);
				out += wprdp.put_pixel(outA, out, color2);
			}
			break;
		case 13 /* WHITE */:
			for (i = 0; i < count; i++) {
				out += wprdp.put_pixel(outA, out, 0xFFFFFFFF);
			}
			break;
		case 14 /* BLACK */:
			for (i = 0; i < count; i++) {
				out += wprdp.put_pixel(outA, out, 0xFF000000);
			}
			break;
		}
	}
}