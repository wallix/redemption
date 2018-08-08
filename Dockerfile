FROM ubuntu:17.10

# clang: RUN echo -e "deb http://apt.llvm.org/artful/ llvm-toolchain-artful-6.0 main\ndeb-src http://apt.llvm.org/artful/ llvm-toolchain-artful-6.0 main" | tee -a /etc/apt/sources.list

# Install
RUN apt-get -qq update
RUN apt-get install -y g++
RUN apt-get install -y libboost-tools-dev libboost-test-dev libssl-dev libkrb5-dev libgssglue-dev libsnappy-dev libpng-dev
RUN apt-get install -y libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libx264-dev libbz2-dev
#RUN apt-get install -y libavcodec-extra libavformat57 libavutil55 libswscale4
# RUN apt-get install -y libavcodec-extra

RUN mkdir -p /gcc/
WORKDIR /gcc/

COPY . /gcc/
RUN bjam -q --toolset=gcc cxxflags='-DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING' rdpproxy
