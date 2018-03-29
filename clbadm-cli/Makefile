packages := libnl-genl-3.0

CFLAGS := $(shell pkg-config --cflags $(packages)) -I../include/uapi
LDFLAGS := $(shell pkg-config --libs $(packages))

clbadm: main.o
	$(CC) $(LDFLAGS) -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	$(RM) clbadm
