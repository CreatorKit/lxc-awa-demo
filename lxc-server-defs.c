#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/server.h>

#include "lxc-obj-defs.h"

int main(void)
{
    AwaServerSession * session = AwaServerSession_New();

    AwaServerSession_Connect(session);

    DefineLxcAgentServerObject(session);
    DefineLxcServerObject(session);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}
