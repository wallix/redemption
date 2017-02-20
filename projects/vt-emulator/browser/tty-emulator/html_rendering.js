var TTYHTMLRendering = (function(){

var i2strcolor = function(int_color)
{
    return ('00000' + int_color.toString(16)).slice(-6)
}

var elem2style = function(e)
{
    var style = '';
    if (e.f) style += 'color:#'+i2strcolor(e.f)+';';
    if (e.b) style += 'background-color:#'+i2strcolor(e.b)+';';
    if (e.r) {
        if (e.r & 1) style += 'font-weight:bold;';
        if (e.r & 2) style += 'font-style:italic;';
        if (e.r & 4) style += 'text-decoration:underline;';
    }
    return style
}

return function(screen)
{
    var estyle = screen.style;

    var empty_line = '                                                               '
    while (empty_line.length < screen.columns) {
        empty_line += empty_line;
    }

    var terminal = '';
    for (var lines of screen.data) {
        var htmlline = ''
        for (var line of lines) {
            var sz_s = 0;
            for (var e of line) {
                //console.log(e)
                if (e.r != undefined) estyle.r = e.r;
                if (e.f != undefined) estyle.f = e.f;
                if (e.b != undefined) estyle.b = e.b;
                htmlline += '<span style="' + elem2style(estyle) + '">' +
                    (e.s ? e.s.replace(/&/g, '&amp;').replace(/</g, '&lt;') : '') +
                    '</span>'
                sz_s += e.s ? e.s.length : 0
            }
            if (sz_s && sz_s < screen.columns) {
                htmlline += '<span style="' + elem2style({'f':estyle.f,'b':estyle.b}) + '">' +
                    empty_line.substr(0, screen.columns - sz_s) +
                    '</span>'
            }
        }
        terminal += '<p>' + htmlline + '\n</p>';
    }

    return '<p id="tty-player-title">' + (screen.title||'No Title') + '</p>'+
        '<div id="tty-player-terminal">' +
            terminal +
            // force terminal width
            //'<p>' + empty_line.substr(0, screen.columns) + '</p>' +
        '</div>'
    ;
}

})()
