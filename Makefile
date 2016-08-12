LXC_AWA_AGENT=lxc-awa-agent
LXC_LIB=lxc
LXC_LDFLAGS=-L/usr/lib/x86_64-linux-gnu -l$(LXC_LIB)
OBJ = lxc-awa-agent.o

AWA_LDFLAGS=-L/lib -lawa
CC=gcc

CFLAGS=-I/include -I. -g
LDFLAGS=$(LXC_LDFLAGS) $(AWA_LDFLAGS)

SERVER_DEFS=lxc-server-defs
OBJ_SD=lxc-server-defs.o


%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(LXC_AWA_AGENT) $(SERVER_DEFS)

$(LXC_AWA_AGENT): $(OBJ)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

$(SERVER_DEFS): $(OBJ_SD)
	gcc -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	rm -rf *.o
	rm -f $(LXC_AWA_AGENT) $(SERVER_DEFS)
