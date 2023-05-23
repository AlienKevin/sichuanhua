FROM ubuntu:23.04

# Use Aliyun mirror in China
# CMD sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list
# CMD sed -i s@/security.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list

RUN apt-get update

RUN apt-get update && apt-get install -y \
    libicu-dev \
    nlohmann-json3-dev \
    g++-13 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /
COPY . /

WORKDIR /sichuanhua

RUN g++-13 -std=gnu++20 dict.cpp main.cpp server.cpp -licuuc -licudata -licui18n -o server

RUN mv server ../

WORKDIR ../

RUN chmod +x server

# Set the entrypoint script
COPY entrypoint.sh /
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
