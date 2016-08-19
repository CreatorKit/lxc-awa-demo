#include "lxc-obj-defs.h"

static void AddLxcAgentResources(AwaObjectDefinition *objectDefinition)
{
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXCA_RESID_NAME),   "Name",   true, AwaResourceOperations_ReadOnly, "LXC Agent");
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXCA_RESID_CREATE), "Create", true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXCA_RESID_APPID),  "AppID", true, AwaResourceOperations_ReadWrite, NULL);
}

static void AddLxcResources(AwaObjectDefinition *objectDefinition)
{
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_NAME),    "Name",   true, AwaResourceOperations_ReadOnly, LXC_DEFAULT_NAME);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_START),   "Start",  true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_STOP),    "Stop",   true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_DESTROY), "Stop",   true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_STATUS),  "Status", true, AwaResourceOperations_ReadOnly, "UNKNOWN");
}

void DefineLxcAgentClientObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_AGENT_OBJID), "LXCD", 1, 1);

    AddLxcAgentResources(objectDefinition);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

void DefineLxcClientObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_OBJID), "Container", 0, LXC_MAX_INSTANCES);

    AddLxcResources(objectDefinition);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

void DefineLxcAgentServerObject(AwaServerSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_AGENT_OBJID), "LXCD", 1, 1);

    AddLxcAgentResources(objectDefinition);

    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);
    AwaServerDefineOperation_Add(operation, objectDefinition);
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaServerDefineOperation_Free(&operation);
}

void DefineLxcServerObject(AwaServerSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_OBJID), "Container", 0, LXC_MAX_INSTANCES);

    AddLxcResources(objectDefinition);

    AwaServerDefineOperation * operation = AwaServerDefineOperation_New(session);
    AwaServerDefineOperation_Add(operation, objectDefinition);
    AwaServerDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaServerDefineOperation_Free(&operation);
}
