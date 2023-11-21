#!/usr/bin/env bash
bastion_branches=''
other_branches=''
for d in */ ; do
    name=${d/\/}
    if [[ $name =~ ^'maintenance_bastion_'([0-9]+)'.0'$ ]]; then
        name="Bastion <b>${BASH_REMATCH[1]}</b>"
        bastion_branches+="<li><a href='$d'>$name</a></li>"$'\n'
    else
        other_branches+="<li><a href='$d'>$name</a></li>"$'\n'
    fi
done

toc="$other_branches$(sort -Vr <<<"$bastion_branches")"

echo -E '<!DOCTYPE html>
<html>
<head>
<title>Index</title>
<style>
a {
  font-size: 1.2em;
  padding: .3rem;
  display: block;
}
a:first-letter {
  text-transform: uppercase;
}
a:hover {
  background: #eee;
}
</style>
<body>
<ul>
'"$toc"'
</ul>
</body>
</head>
</html>'
