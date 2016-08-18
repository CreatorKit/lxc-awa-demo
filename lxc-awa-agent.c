#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <awa/common.h>
#include <awa/client.h>

#include <lxc/lxccontainer.h>

#include "LWM2M_Device_obj.h"

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


struct lxc_agent
{
    AwaClientSession *session;
    struct container_info **ci;
};

struct container_info
{
    struct lxc_container *c;
    int instance;
    char *name;
    const char *status;
};

static void createCallback(const AwaExecuteArguments * arguments, void * context);
static void startCallback(const AwaExecuteArguments * arguments, void * context);
static void stopCallback(const AwaExecuteArguments * arguments, void * context);
static void destroyCallback(const AwaExecuteArguments * arguments, void * context);
static void createContainerInstance(struct lxc_agent *a, int instance, char *name);

static int createContainer(struct container_info *ci, char * name, int instance);
static int startContainer(struct lxc_container *c, int instance);
static int stopContainer(struct lxc_container *c, int instance);
static int destroyContainer(struct container_info *ci);


static char* instanceName(int instance)
{
    char *buf;
    int buflen = strlen(LXC_DEFAULT_NAME) + 10;

    buf = calloc(1, buflen);
    snprintf(buf, buflen, "%s-%d", LXC_DEFAULT_NAME, instance);
    printf("InstanceName: %s\n", buf);
    return buf;
}

static char *objectInstance(char *objId, int instance)
{
    char *buf;
    int buflen = strlen(objId) + 60;

    buf = calloc(1, buflen);
    snprintf(buf, buflen, "/%s/%d", objId, instance);
    printf("objectInstance: %s\n", buf);
    return buf;
}

static char *resourceInstance(char *objId, int instance, char* resId)
{
    char *buf;
    int buflen = strlen(objId) + strlen(resId) + 60;

    buf = calloc(1, buflen);
    snprintf(buf, buflen, "/%s/%d/%s", objId, instance, resId);
    // printf("resourceInstance: %s\n", buf);
    return buf;
}

static void DefineLxcAgentObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_AGENT_OBJID), "LXCD", 1, 1);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXCA_RESID_NAME),   "Name",   true, AwaResourceOperations_ReadOnly, "LXC Agent");
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXCA_RESID_CREATE), "Create", true, AwaResourceOperations_Execute);

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

static void DefineLxcObject(AwaClientSession * session)
{
    AwaObjectDefinition * objectDefinition = AwaObjectDefinition_New(atoi(LXC_OBJID), "Container", 0, LXC_MAX_INSTANCES);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_NAME),    "Name",   true, AwaResourceOperations_ReadOnly, LXC_DEFAULT_NAME);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_START),   "Start",  true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_STOP),    "Stop",   true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsNoType(objectDefinition, atoi(LXC_RESID_DESTROY), "Stop",   true, AwaResourceOperations_Execute);
    AwaObjectDefinition_AddResourceDefinitionAsString(objectDefinition, atoi(LXC_RESID_STATUS),  "Status", true, AwaResourceOperations_ReadOnly, "UNKNOWN");

    AwaClientDefineOperation * operation = AwaClientDefineOperation_New(session);
    AwaClientDefineOperation_Add(operation, objectDefinition);
    AwaClientDefineOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientDefineOperation_Free(&operation);
}

static void CreateAgentInstance(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_CreateObjectInstance(operation, OBJECT_INSTANCE(LXC_AGENT_OBJID, 0));

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void updateContainerObjects(struct lxc_agent *a)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(a->session);
    char *str;
    int count;
    struct container_info *ci;
    struct lxc_container *c;
    bool perform = false;

    for (count = 0; count < LXC_MAX_INSTANCES; count++)
    {
        ci = a->ci[count];
        if (ci)
        {
            c = ci->c;
            if (c && c->is_defined(c))
            {
                ci->status = c->state(c);
                AwaClientSetOperation_AddValueAsCString(operation, str = resourceInstance(LXC_OBJID, count, LXC_RESID_STATUS), ci->status);
                free(str);
                perform = true;
            }
        }
    }

    if (perform)
    {
        AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    }
    AwaClientSetOperation_Free(&operation);
}

static void createContainerInstance(struct lxc_agent *a, int instance, char *name)
{
    if (instance < LXC_MAX_INSTANCES)
    {
        AwaClientSetOperation * operation = AwaClientSetOperation_New(a->session);
        struct container_info *ci;
        char *str;

        AwaClientSetOperation_CreateObjectInstance(operation, str = objectInstance(LXC_OBJID, instance));
        free(str);

        name = name ? name : instanceName(instance);

        AwaClientSetOperation_AddValueAsCString(operation, str = resourceInstance(LXC_OBJID, instance, LXC_RESID_NAME), name);
        free(str);

        AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
        AwaClientSetOperation_Free(&operation);

        ci = calloc(1, sizeof(struct container_info));
        a->ci[instance] = ci;
        createContainer(ci, name, instance);
        startContainer(ci->c, instance);

        ci->name = name;
        ci->instance = instance;
        ci->c = a->ci[instance]->c;

        AwaClientExecuteSubscription * startSub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_START), startCallback, (void*)ci);
        free(str);
        AwaClientExecuteSubscription * stopSub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_STOP), stopCallback, (void*)ci);
        free(str);
        AwaClientExecuteSubscription * destroySub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_DESTROY), destroyCallback, (void*)ci);
        free(str);

        /* Start listening to notifications */
        AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(a->session);

        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, startSub);
        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, stopSub);
        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, destroySub);

        //TODO implemement subscription cancel and freeing

        AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
        AwaClientSubscribeOperation_Free(&subscribeOperation);
    }
}

static int createContainer(struct container_info *ci, char *name, int instance)
{
    struct lxc_container *c;
    int ret = 0;
    /* Setup container struct */
    c = lxc_container_new(name, NULL);
    if (!c) {
        fprintf(stderr, "Failed to setup lxc_container struct\n");
        ret = 1;
    }
    // ci = a->ci[instance];
    ci->c = c;

    if (c->is_defined(c)) {
        fprintf(stderr, "Container already exists\n");
        ret = 1;
    }

    /* Create the container */
    if (!c->createl(c, "download", NULL, NULL, LXC_CREATE_QUIET,
                    "-d", "ubuntu", "-r", "trusty", "-a", "i386", NULL)) {
        fprintf(stderr, "Failed to create container rootfs\n");
        ret = 1;
    }

    if (!ret)
    {
        /* Query some information */
        printf("Container state: %s\n", c->state(c));

        // a->state = CREATED;
    }
    return ret;
}


static int startContainer(struct lxc_container *c, int instance)
{
    int ret = 0;

    /* Start the container */
    if (!c->start(c, 0, NULL)) {
        fprintf(stderr, "Failed to start the container\n");
        ret = 1;
    }

    if (!ret)
    {
        /* Query some information */
        printf("Container state: %s\n", c->state(c));
        printf("Container PID: %d\n", c->init_pid(c));
    }
    return ret;
}

static int stopContainer(struct lxc_container *c, int instance)
{
    int ret = 0;

    if (c)
    {
        /* Stop the container */
        if (!c->shutdown(c, 30)) {
            printf("Failed to cleanly shutdown the container, forcing.\n");
            if (!c->stop(c)) {
                fprintf(stderr, "Failed to kill the container.\n");
                ret = 1;
            }
        }
        /* Query some information */
        printf("Container state: %s\n", c->state(c));
        printf("Container PID: %d\n", c->init_pid(c));

    }
    return ret;
}

static int destroyContainer(struct container_info *ci)
{
    int ret = 0;

    if (ci->c)
    {
        /* Destroy the container */
        if (!ci->c->destroy(ci->c)) {
            fprintf(stderr, "Failed to destroy the container.\n");
            ret = 1;
        }
        else
        {
            lxc_container_put(ci->c);
            ci->c = NULL;
        }
    }
    return ret;
}

static void createCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct lxc_agent *c = (struct lxc_agent*)context;

    static int instance = 0;
    printf("Create Callback received. Context = %p\n", c);
    createContainerInstance(c, instance, NULL);
    instance++;

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void startCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Start Callback received. Context = %p\n", ci);
    startContainer(ci->c , ci->instance);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void stopCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Stop Callback received. Context = %p\n", ci);
    stopContainer(ci->c , ci->instance);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void destroyCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Destroy Callback received. Context = %p\n", ci);
    destroyContainer(ci);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}


static bool stopFlag = false;

/*
 * Trigger the program to exit on CTRL-C
 */
static void stop(int ignore)
{
    printf("Exiting...\n");
    stopFlag = true;
}

int main(void)
{
    signal(SIGINT, stop);

    AwaClientSession * session = AwaClientSession_New();

    AwaClientSession_Connect(session);

    DefineLxcAgentObject(session);
    CreateAgentInstance(session);
    DefineLxcObject(session);
    InitDevice(session);

    /* Application-specific data */
    struct lxc_agent *agent;
    struct container_info **c = calloc(LXC_MAX_INSTANCES, sizeof(struct container_info *));

    agent->session = session;
    agent->ci = c;

    AwaClientExecuteSubscription * createSub = AwaClientExecuteSubscription_New(RESOURCE_INSTANCE(LXC_AGENT_OBJID, 0, LXCA_RESID_CREATE), createCallback, (void*)agent);

    /* Start listening to notifications */
    AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(session);

    AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, createSub);

    AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&subscribeOperation);


    printf("Waiting for events...\n");

    while (!stopFlag)
    {
        sleep(1);

        /* Receive notifications */
        AwaClientSession_Process(session, OPERATION_PERFORM_TIMEOUT);
        AwaClientSession_DispatchCallbacks(session);
        updateContainerObjects(agent);
        DeviceControl(session);
    }

    AwaClientSubscribeOperation * cancelSubscribeOperation = AwaClientSubscribeOperation_New(session);

    AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, createSub);

    AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);

    /* Free the execute subscription */
    AwaClientExecuteSubscription_Free(&createSub);
    // AwaClientExecuteSubscription_Free(&stopSub);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}
