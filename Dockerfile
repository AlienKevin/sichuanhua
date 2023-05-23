FROM ubuntu:23.04

RUN apt-get update && apt-get install -y \
    libicu-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /
COPY . /

RUN g++-13 -std=gnu++20 dict.cpp main.cpp server.cpp -licuuc -licudata -licui18n -o server

CMD mv server ../

CMD ["./server"]
