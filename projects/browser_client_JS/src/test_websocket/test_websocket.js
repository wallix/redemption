
var fs = require('fs');

// Loading index.html displayed to the client
var server = require('http').createServer(function(req, res) {
//      fs.readFile('./index.html', 'utf-8', function(error, content) {
        fs.readFile('./client_rdp_JS_natif.html', 'utf-8', function(error, content) {
        res.writeHead(200, {"Content-Type": "text/html"});
        res.end(content);
    });
    
/*
   fs.readFile('./wallix.png', function(err, buf){
        var image = fs.readFileSync('./wallix.png');
        res.writeHead(200, {'Content-Type': 'image/gif' });
        res.end(image);
        console.log('image file is initialized');
    });
*/
});

var io = require('socket.io').listen(server);

server.listen(8080);
console.log('Server running at  http://localhost:8080 '+ server.address().address + server.address().port);

    function sendBinaryData(item, index) {
        var res = String.fromCharCode(item);
        var data = {
                    "type":"text",
                    "content":res,
                    "index":index
                    };
        
        socket.send({data:JSON.stringify(data), msg:'sending inData', index:index});
    }


io.sockets.on('connection', function (socket) {
    console.log('A client is connected !');
    socket.emit('event', { msg: 'The world is round, there is no up or down.' });
    socket.on('disconnect', function(){console.log('A client has disconnected !');});
    socket.on("close", function () {console.log("Browser gone.");});



    socket.on('message', function (str) {
        
        var ob = JSON.parse(str);
        console.log("Socket id = " + socket.id);
        console.log("\n\n Server received from client : " + str);


        console.dir(ob);
        console.log("ob.type: %s", ob.data.type);


        switch(ob.data.type) {

            case 'type1':
        //        console.log("Socket id = " + socket.id);
                console.log("Server @type1 : received from client : " + ob.data.content);

                //inData.forEach(sendBinaryData);
                //socket.send({data:JSON.stringify(data), msg:'sending inData', index:i});
                //console.log("Server @type1 : done sending inData ");

                socket.send('messagetype1', { msg: 'The world is round, there is no up or down.' });
                console.log("Server socket.emit done");

                break;

            case 'type2':
                console.log("\nServer @type2 : received from client : " + ob.data.content)

                //var path ="wallix.png";
                //console.log("path is " + path)

                //fs.exists(path, function() {
                //    console.log(path + " exists ")
                    var data = { 
                                "type":"type2",
                                "path":"' + path + '",
                                "content":"Here is a message the client awaits!",
                                id:"picture"
                                };
               //     socket.send({data:data, msg:'Client ? Want some data ?' });

                    var number = 0;
                    while (number < 10) {
                        data = {
                            "type":"text",
                            "content":"0xbb",
                            "index":number
                            };

                        socket.emit('event', { data:data, msg: 'Client ? Want some data ?', num:number });         
                        console.log( "socket.emit event number " + number);
                        number++;
                    }
                //});
                break;
        }   
    });

    socket.on('poke', function (message) {
        console.log('poke event :A client is talking to me ! He says : ' + message);
    }); 

});
