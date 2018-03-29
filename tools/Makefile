packages := libnl-genl-3.0

CFLAGS := $(shell pkg-config --cflags $(packages)) -I../include/uapi
LDFLAGS := $(shell pkg-config --libs $(packages))

tools := clbadm
clbadm-objs := clbadm.o

all: $(tools)

%: %-objs.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.SECONDEXPANSION:
.PHONY: clean
clean:
	$(RM) $(tools) $(foreach x, $(tools), $($(x)-objs))
