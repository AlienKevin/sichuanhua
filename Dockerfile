FROM alpine:3.18

# 使用 HTTPS 协议访问容器云调用证书安装
RUN apk add ca-certificates

# 安装依赖包，如需其他依赖包，请到alpine依赖包管理(https://pkgs.alpinelinux.org/packages?name=php8*imagick*&branch=v3.13)查找。
# 选用国内镜像源以提高下载速度
RUN sed -i 's/dl-cdn.alpinelinux.org/mirrors.tencent.com/g' /etc/apk/repositories \
&& apk add --update --no-cache g++ nlohmann-json icu-dev poco-dev

WORKDIR /app
ADD sichuanhua /app

RUN g++ -std=gnu++20 dict.cpp main.cpp server.cpp -licuuc -licudata -licui18n -lPocoNet -lPocoUtil -lPocoFoundation -o server && chmod +x server

COPY fangyan.json shupin.simp.dict.yaml /app/

CMD ["./server", "serve"]
