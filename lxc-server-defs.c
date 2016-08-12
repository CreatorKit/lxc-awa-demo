#include <stdlib.h>
#include <stdio.h>

#include <awa/common.h>
#include <awa/server.h>

#define OPERATION_PERFORM_TIMEOUT 1000

#define LXC_AGENT_OBJID     "13375"
#define LXCA_RESID_NAME     "101"
#define LXCA_RESID_CREATE   "102"

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

static void DefineLxcAgentObject(AwaServerSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_AGENT_OBJID), "LXCD", 1, 1);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXCA_RESID_NAME),   "Name",   true, AwaResourceOperations_ReadOnly, "LXC Agent");
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXCA_RESID_CREATE), "Create", true, AwaResourceOperations_Execute);

    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);
    AwaServerDefineOperation_Add(operation, objectDefinition);
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaServerDefineOperation_Free(&operation);
}

static void DefineLxcObject(AwaServerSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_OBJID), "Container", 0, LXC_MAX_INSTANCES);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_NAME),    "Name",   true, AwaResourceOperations_ReadOnly, LXC_DEFAULT_NAME);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_START),   "Start",  true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_STOP),    "Stop",   true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_DESTROY), "Destroy",true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_STATUS),  "Status", true, AwaResourceOperations_ReadOnly, "UNKNOWN");

    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);
    AwaServerDefineOperation_Add(operation, objectDefinition);
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaServerDefineOperation_Free(&operation);
}


int main(void)
{
    AwaServerSession * session = AwaServerSession_New();

    AwaServerSession_Connect(session);

    DefineLxcAgentObject(session);
    DefineLxcObject(session);

    AwaServerSession_Disconnect(session);
    AwaServerSession_Free(&session);
    return 0;
}
