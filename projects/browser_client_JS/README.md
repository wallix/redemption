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
./emsdk_env.sh
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
./emsdk_env.sh
```

# Transpilation

```bash
em++ file.cpp -o file.html
```

## Options

`--preload-file path1`

`--embed-file path1`

`--exclude-file path1`

`-O1`, `-O2`, `-O3`


# Compilation

## Include path

```bash
ln -s /usr/include/boost system_include/
```

## Configure Emscripten

```bash
source $EMSDK_PATH/emsdk_set_env.sh
```

## Run bjam

```bash
bjam -j7 toolset=clang "cxxflags=-fcolor-diagnostics -s USE_ZLIB=1 -s USE_LIBPNG=1" debug jsclient
```


# Test

```bash
cd bin/clang.../debug
ln -s ../../../src/js_client.html rdpclient.html
# run proxy, with nla and tls disabled
# open http://localhost:7542/rdpclient.html
```

## Proxy websocket -> tcp

```bash
wget https://github.com/vi/websocat/archive/master.zip
cd websocat-master
cargo build --release --bin websocat
./target/release/websocat --binary ws-l:127.0.0.1:8080 tcp:127.0.0.1:3389
```

## Http server


```bash
cd bin/clang.../debug
python -m SimpleHTTPServer 7542
```
