FROM debian:10

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && apt install -y build-essential libncurses-dev python3 expect-dev  moreutils flex unzip bison wget libxml2-utils tclsh \
	python python-tempita python-six python-future python-ply xorriso qemu-system-x86 curl gawk

ADD https://www.doc.ic.ac.uk/~vsartako/asplos/genode.tar.gz /
ADD https://www.doc.ic.ac.uk/~vsartako/asplos/tch.tar.xz  /
RUN tar -xf /genode.tar.gz
RUN tar -xf /tch.tar.xz

##############

RUN cd /genode && ./tool/create_builddir x86_64

COPY build.conf /genode/build/x86_64/etc/

##############

COPY CubicleOS /CubicleOS

RUN gunzip /CubicleOS/unikraft/lib/libnginx/rootfs.h.gz

RUN cd /CubicleOS/app-sqlite/ && make
RUN cd /CubicleOS/app-nginx/ && make
RUN cd /CubicleOS/vanilla/ && gcc main.c sqlite3.c -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_THREADSAFE=0 -O2 -o vanilla

RUN cd /CubicleOS/kernel/ && make nginx && make sqlite
