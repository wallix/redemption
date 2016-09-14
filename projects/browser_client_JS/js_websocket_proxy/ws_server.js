#!/usr/bin/env node
var WebSocketServer = require('websocket').server;
var http = require('http');

var server = http.createServer(function(request, response) {
    console.log((new Date()) + ' Received request for ' + request.url);
    response.writeHead(404);
    response.end();
});
server.listen(8080, function() {
    console.log((new Date()) + ' Server is listening on port 8080');
});

wsServer = new WebSocketServer({
    httpServer: server,
    // You should not use autoAcceptwssockets for production
    // applications, as it defeats all standard cross-origin protection
    // facilities built into the protocol and the browser.  You should
    // *always* verify the connection's origin and decide whether or not
    // to accept it.
    autoAcceptConnections: false
});
wsServer.binaryType = 'arraybuffer';
//wsServer.setEncoding('binary');


function originIsAllowed(origin) {
  // put logic here to detect whether the specified origin is allowed.
  return true;
}

wsServer.on('request', function(request) {
      console.log((new Date()) + ' WebSocket connection request from origin ' + request.origin);
    if (!originIsAllowed(request.origin)) {
      // Make sure we only accept requests from an allowed origin
      request.reject();
      console.log((new Date()) + ' WebSocket connection from origin ' + request.origin + ' rejected.');
      return;
    }
    var wssocket = request.accept('rdp-protocol', request.origin);
//    wssocket.setEncoding('binary');
    wssocket.binaryType = 'arraybuffer';
    console.log((new Date()) + ' Connection accepted.');

    // WebSocket open, now establish connection to remote RDP server
    var net = require('net');
    var targetIp =  '127.0.0.1';
    var targetPort = 3389;

    var tcpclient = new net.Socket({
        readable: true,
        writable: true
    });

    tcpclient.setNoDelay();
    tcpclient.setEncoding('binary');

    var loc = tcpclient.connect(targetPort, targetIp, function() {
        console.log('Client >> Proxy websocket : Connected to ' + targetIp);
        console.log('Client >> Proxy websocket : Client socket adress : '
            + tcpclient.address().address + ':' + tcpclient.address().port);
        console.log('Client >> Proxy websocket : Remote info : ' + tcpclient.remoteAddress + ':' + tcpclient.remotePort);
        console.log('Client >> Proxy websocket : client.fd : ' + tcpclient._handle.fd);
        console.log('Client >> Proxy websocket : process pid : ' + process.pid);

        tcpclient.on('data', function(data) {
                console.log('tcpclient data event (RDP Server) ' + data.length);
                var text = "";
                for (var i = 0 ; i < data.length ; ++i){
                    text += ":" + (data.charCodeAt(i)+0x10000).toString(16).substr(-2);
                }
                console.log("RDP Server Says : "+text);
                var buf = new Buffer(data,'binary')
                var text2 = "";
                for (var i = 0 ; i < buf.length ; ++i){
                    text2 += ":" + (buf[i]+0x10000).toString(16).substr(-2);
                }
                console.log("RDP Server Says : "+text2);
                wssocket.sendBytes(buf);
        });

        wssocket.on('message', function(message) {
            console.log('wssocket message event');
            if (message.type === 'utf8') {
                console.log('Received Message (utf8): ' + message.utf8Data);
                wssocket.sendUTF(message.utf8Data);
            }
            else if (message.type === 'binary') {
                // For RDP proxying we should only receive binary messages
                console.log('Received Binary Message of ' + message.binaryData.length + ' bytes ' + message.binaryData);
                var text = "";
                for (var i = 0 ; i < message.binaryData.length ; ++i){
                    text += ":" + (message.binaryData[i]+0x10000).toString(16).substr(-2);
                }
                console.log("Browser Says : "+text);
                
                tcpclient.write(message.binaryData, message.binaryData.length);
            }
            console.log('Message forwarded to tcpclient');
        });
        wssocket.on('close', function(reasonCode, description) {
            console.log((new Date()) + ' Peer ' + wssocket.remoteAddress + ' disconnected.');
        });

        wssocket.on('error', function(message) {
            console.log('Error on wssocket:' + message);
        });

        tcpclient.on('error', function(message) {
            console.log('Error on tcpclient: ' + message);
        });
        console.log('Connection ready');
    });
});
