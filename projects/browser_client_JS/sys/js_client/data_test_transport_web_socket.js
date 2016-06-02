
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
        var start = current;
        current += i;
        var newData = data.slice(start, current);
        console.log('new getDataOctet' + newData[1]);
		_recv_value(newData);

	} else {
        //var tab[i] = [0];
		//_recv_value(tab);
	}
}
