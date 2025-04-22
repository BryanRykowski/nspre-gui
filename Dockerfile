FROM debian:bookworm
WORKDIR /src
RUN mkdir /build
RUN apt-get update && apt-get upgrade -y && \
apt-get install libsdl2-dev git cmake build-essential -y
RUN git clone https://github.com/BryanRykowski/nspre-gui . && \
git submodule update --init --recursive
CMD apt-get update && apt-get upgrade -y && \
git pull && \
cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=-s && \
cmake --build /build && \
chown -R 1000:1000 /build
