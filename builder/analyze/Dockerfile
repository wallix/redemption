FROM debian:10

ADD build_requirement.txt build_requirement.txt
RUN apt-get update && xargs apt-get -y install < build_requirement.txt && apt-get clean

ADD tools_requirement.txt tools_requirement.txt
RUN apt-get update && xargs apt-get -y install < tools_requirement.txt && apt-get clean

ADD lua_requirement.txt lua_requirement.txt
RUN while read line; do [ -n "$line" ] && luarocks install $line; done < lua_requirement.txt
