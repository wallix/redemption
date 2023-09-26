#!/usr/bin/env bash
toc=''
for d in */ ; do
    toc+="<li><a href='$d'>$d</a></li>"$'\n'
done
echo -E '<!DOCTYPE html>
<html>
<head>
<title>Index</title>
<body>
<ul>
'"$toc"'
</ul>
</body>
</head>
</html>'
