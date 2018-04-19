FROM alpine

RUN apk update && \
    apk add build-base automake autoconf readline-dev git

WORKDIR /vcsi

RUN git clone https://github.com/mdbarr/infostd.git && \
    cd infostd && \
    make && \
    make install && \
    cd ..

RUN git clone https://github.com/mdbarr/vcsi.git && \
    cd vcsi && \
    ./configure && \
    make && \
    make install

CMD ["/usr/local/bin/vcsi"]
