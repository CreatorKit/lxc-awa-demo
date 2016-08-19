#ifndef LXC_OBJ_DEFS_H
#define LXC_OBJ_DEFS_H

#include <awa/client.h>
#include <awa/server.h>

#define OPERATION_PERFORM_TIMEOUT 1000

#define LXC_AGENT_OBJID     "13375"
#define LXCA_RESID_NAME     "101"
#define LXCA_RESID_CREATE   "102"
#define LXCA_RESID_APPID    "103"

#define LXC_OBJID           "13376"
#define LXC_RESID_NAME      "101"
#define LXC_RESID_START     "102"
#define LXC_RESID_STOP      "103"
#define LXC_RESID_DESTROY   "104"
#define LXC_RESID_STATUS    "105"

#define LXC_DEFAULT_NAME    "Container"
#define LXC_MAX_INSTANCES   10

#define OBJECT_INSTANCE(obj, inst) "/" obj "/" #inst
#define RESOURCE_INSTANCE(obj, inst, res) "/" obj "/" #inst "/" res

void DefineLxcAgentClientObject(AwaClientSession * session);
void DefineLxcClientObject(AwaClientSession * session);
void DefineLxcAgentServerObject(AwaServerSession * session);
void DefineLxcServerObject(AwaServerSession * session);

#endif