
var data = [42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,
	    42, 42, 42, 42, 42, 42, 42, 42,]

var len = 64;
var current = 0;

function getDataOctet(i) {
	if (current < len) {
		current++;
		//console.log('getOctet = ' + data[current-1]);
		//return data[current-1];
		_recv_value(data[current-1], i);
	} else {
		//return 0;
		_recv_value(0, i);
	}
}
