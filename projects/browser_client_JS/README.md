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

# Include path

```bash
ln -s /usr/include/boost system_include/
```
