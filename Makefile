LXC_AWA_AGENT=lxc-awa-agent
LXC_LIB=lxc
LXC_LDFLAGS=-L/usr/lib/x86_64-linux-gnu -l$(LXC_LIB)

DEPS = lxc-obj-defs.h
DEPS += LWM2M_Device_obj.h

OBJ = lxc-awa-agent.o
OBJ += LWM2M_Device_obj.o
OBJ += lxc-obj-defs.o

AWA_INCLUDE=-I/include
AWA_LDFLAGS=-L/lib -lawa
CC=gcc
LD=gcc

CFLAGS=$(AWA_INCLUDE)
LDFLAGS=$(LXC_LDFLAGS) $(AWA_LDFLAGS)

SERVER_DEFS=lxc-server-defs
OBJ_SD = lxc-server-defs.o
OBJ_SD += lxc-obj-defs.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I. -g

all: $(LXC_AWA_AGENT) $(SERVER_DEFS)

$(LXC_AWA_AGENT): $(OBJ)
	$(LD) -o $@ $^ $(LDFLAGS) $(CFLAGS)

$(SERVER_DEFS): $(OBJ_SD)
	$(LD) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -f $(LXC_AWA_AGENT) $(SERVER_DEFS)
