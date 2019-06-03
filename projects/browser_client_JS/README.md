# Install Emscripten

http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html

## Download

```bash
wget 'https://github.com/juj/emsdk/archive/master.zip' -Oemsdk-master.zip
unzip emsdk-master.zip
cd emsdk-master/
./emsdk install latest
```

## Setting

```bash
./emsdk activate latest
source ./emsdk_env.sh
```

## Environment

```bash
source ./emsdk_set_env.sh
```

## Update

```bash
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## Test

```cpp
// test.cpp
#include <cstdio>
int main()
{
    puts("ok");
}
```

```bash
em++ test.cpp -o test.js
node test.js
```


# Compile RdpClient

## Configure boost path for test targets

    mkdir system_include/
    ln -s /usr/include/boost system_include/

## Configure Emscripten

    source $EMSDK_PATH/emsdk_set_env.sh


## Run bjam

    bjam -j7 toolset=clang debug js_client

Or with tests:

    bjam -j7 toolset=clang debug

## Install

    bjam -j7 toolset=clang debug -s copy_application_mode=symbolic rdpclient

Set module name with `-s JS_MODULE_NAME=xxx`

Set source map with `-s JS_SOURCE_MAP='prefix//'`

By default `copy_application_mode` is equal to `copy`


# Open client

## Enable websocket

Inner `rdpproxy.ini`.

```ini
[globals]
enable_websocket=yes
```

Or use `websocat` (websocket -> tcp).

```bash
sudo apt install cargo
wget https://github.com/vi/websocat/archive/master.zip
unzip master.zip
cd websocat-master
cargo build --release --bin websocat
./target/release/websocat --binary ws-l:127.0.0.1:8080 tcp:127.0.0.1:3389
```

## Open rdpclient.html

`bjam open_client` or `bjam open_client -s browser='my browser' -s username=x -p password=x`
