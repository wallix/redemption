# Install Emscripten

https://emscripten.org/docs/getting_started/downloads.html

```sh
git clone https://github.com/emscripten-core/emsdk.git &&
cd emsdk &&
./emsdk install latest &&
./emsdk activate latest
```
<!-- version: 12.9.1 -->

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
source ./emsdk_env.sh
em++ test.cpp -o test.js
node test.js
```


# Compile RdpClient

## Configure boost path for test targets

    mkdir system_include/
    ln -s /usr/include/boost system_include/

## Configure Emscripten

    source $EMSDK_PATH/emsdk_env.sh

## Run bjam

    bjam -j7 toolset=clang js_client

Debug mode:

    bjam -j7 toolset=clang debug js_client

And tests with (after `source ./emsdk_env.sh`)

    npm install canvas

    bjam -j7 toolset=clang

If you get some undefined symbol error, run `bjam targets.jam`.

## Debug with source-map

    npm install source-map-support

    bjam debug source_map_support=yes ....


## Install

    bjam -j7 toolset=clang -s copy_application_mode=symbolic rdpclient

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

Create a server at build directory (see `bjam toolset=clang cwd`).

    cd "$(bjam toolset=clang cwd | sed '/^CWD/!d;s/^CWD: //')"
    python -m http.server 7453 --bind 127.0.0.1
    xdg-open http://localhost:7453/client.html

Or run

    ./tools/open_client.sh

    ./tools/open_client.sh debug

Or with parameters

    BROWSER=... WS=... USER=... PASSWORD=... PORT=... ./tools/open_client.sh
