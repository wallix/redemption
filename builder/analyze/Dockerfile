FROM debian:12

ADD build_requirement.txt build_requirement.txt
RUN apt-get update && xargs apt-get -y install < build_requirement.txt && apt-get clean

ADD tools_requirement.txt tools_requirement.txt
RUN apt-get update && xargs apt-get -y install < tools_requirement.txt && apt-get clean

ADD app_requirement.txt app_requirement.txt
RUN apt-get update && xargs apt-get -y install < app_requirement.txt && apt-get clean

RUN python3 -m venv /pyenv
ADD py_requirement.txt py_requirement.txt
RUN /pyenv/bin/pip install -r py_requirement.txt
RUN chmod -R o+x /pyenv
ENV PATH="${PATH}:/pyenv/bin"
