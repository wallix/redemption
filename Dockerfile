FROM ubuntu:20.04

# Install build dependencies
RUN apt-get -qq update && apt-get install -y g++ libboost-tools-dev libboost-test-dev libssl-dev libkrb5-dev \
    libgssglue-dev libsnappy-dev libpng-dev libavcodec-dev libavformat-dev libavutil-dev libswscale-dev \
    libx264-dev libbz2-dev && apt-get install -y git
# Create build directory
RUN mkdir -p /gcc/
# Set container working directory
WORKDIR /gcc/
# Copy source code into build container
COPY . /gcc/
# Clone ppocr repository; could be replaced with `git submodule update --init` but 
# if this runs from another git repository (a fork for example) the owner of the 
# fork must ensure they also fork ppocr and make sure that ../ppocr leads to the proper git repository
RUN cd modules && rm -rf ppocr && git clone https://github.com/wallix/ppocr.git ppocr
# Build rdpproxy
RUN bjam linkflags=-static-libstdc++ variant=release -q --toolset=gcc cxxflags='-DREDEMPTION_DISABLE_NO_BOOST_PREPROCESSOR_WARNING' rdpproxy
# Install rdpproxy into /usr/local
RUN bjam --toolset=gcc variant=release install --prefix=/usr/local || echo "done"

FROM ubuntu:20.04
# Install runtime dependencies
RUN apt-get -qq update && apt-get install -y python3 libgssapi-krb5-2 libpng16-16 libsnappy1v5
# Copy built rdpporxy
COPY --from=0 /usr/local /usr/local
# Copy built rdpproxy configuration
COPY --from=0 /etc /etc
# Expose RDP Server port
EXPOSE 3389
# Copy container startup script
COPY ./start.sh /
# Copy passthrough script
COPY ./tools/passthrough /usr/local/share/passthrough/
# Container entry point
CMD [ "/start.sh" ]
