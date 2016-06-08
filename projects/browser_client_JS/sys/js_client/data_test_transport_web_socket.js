
var data = [42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,
            42, 42, 42, 42, 42, 42, 42, 42,]

var len = 40;

var current = 0;

function* getDataOctet() {
    if (current >= len) {
        console.log('getDataOctet stop');
        yield undefined;
    } else {
        console.log('getDataOctet on going');
    }
    //if (current < len) {
        _recv_value(data[current]);
        current++;
    //} else {

    //}
}

//var funct = getDataOctet();

