
window.onload = function() {
   try {
      var socket = io.connect('http://localhost:8080');


      socket.on('connect', function(data) {
        socket.send('Hello.');
      });
   }
   catch(err) {
        alert('ERROR: socket.io encountered a problem:\n\n' + err);
   }
};

