FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN mkdir -p /tmp

COPY Scripts/setup-linux.sh /tmp/setup-linux.sh

RUN chmod +x /tmp/setup-linux.sh \
    && /tmp/setup-linux.sh \
    && rm /tmp/setup-linux.sh

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

WORKDIR /root

CMD ["/bin/bash"]