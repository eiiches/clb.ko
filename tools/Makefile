packages := libnl-genl-3.0 libprotobuf-c

CFLAGS := $(shell pkg-config --cflags $(packages)) -I../include/uapi  -g -ggdb
LDFLAGS := $(shell pkg-config --libs $(packages)) -g -ggdb

tools := clbadm
clbadm-objs := clbadm.o netlink.pb-c.o clb-client.o

all: netlink.pb-c.h $(tools)

clbadm: $(clbadm-objs)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

netlink.pb-c.c netlink.pb-c.h: ../netlink.proto
	protoc -I.. --c_out=. netlink.proto

.SECONDEXPANSION:
.PHONY: clean
clean:
	$(RM) $(tools) $(foreach x, $(tools), $($(x)-objs))
