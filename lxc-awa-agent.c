#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include <awa/common.h>
#include <awa/client.h>

#include <lxc/lxccontainer.h>

#include "queue.h"
#include "LWM2M_Device_obj.h"

#include "lxc-obj-defs.h"

LIST_HEAD(container_list, container_info);

struct lxc_agent
{
    AwaClientSession *session;
    struct container_list *cl;
};

struct container_info
{
    LIST_ENTRY(container_info) list;
    struct lxc_container *c;
    int instance;
    char *name;
    const char *status;
    AwaClientExecuteSubscription *startSub;
    AwaClientExecuteSubscription *stopSub;
    AwaClientExecuteSubscription *destroySub;
    struct lxc_agent *agent;
};

static void createCallback(const AwaExecuteArguments * arguments, void * context);
static void startCallback(const AwaExecuteArguments * arguments, void * context);
static void stopCallback(const AwaExecuteArguments * arguments, void * context);
static void destroyCallback(const AwaExecuteArguments * arguments, void * context);
static void createContainerInstance(struct lxc_agent *a, int instance, char *name);
static void destroyContainerInstance(struct container_info *ci);

static int createContainer(struct container_info *ci, char * name);
static int startContainer(struct lxc_container *c);
static int stopContainer(struct lxc_container *c);
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

static void CreateAgentInstance(AwaClientSession * session)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(session);

    AwaClientSetOperation_CreateObjectInstance(operation, OBJECT_INSTANCE(LXC_AGENT_OBJID, 0));

    AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSetOperation_Free(&operation);
}

static void updateContainerObjectsList(struct lxc_agent *a)
{
    AwaClientSetOperation * operation = AwaClientSetOperation_New(a->session);
    char *str;
    struct container_info *ci;
    struct lxc_container *c;
    bool perform = false;

    for (ci = a->cl->lh_first; ci != NULL; ci = ci->list.le_next)
    {
        c = ci->c;
        if (c && c->is_defined(c))
        {
            ci->status = c->state(c);
            AwaClientSetOperation_AddValueAsCString(operation, str = resourceInstance(LXC_OBJID, ci->instance, LXC_RESID_STATUS), ci->status);
            free(str);
            perform = true;
        }
    }

    if (perform)
    {
        AwaClientSetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
    }
    AwaClientSetOperation_Free(&operation);
}


static char* getAppID(AwaClientSession *session)
{
    char *str;
    char *ret = NULL;
    AwaClientGetOperation * operation = AwaClientGetOperation_New(session);

    AwaClientGetOperation_AddPath(operation, str = resourceInstance(LXC_AGENT_OBJID, 0, LXCA_RESID_APPID));
    AwaClientGetOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);

    const AwaClientGetResponse * response = AwaClientGetOperation_GetResponse(operation);

    if (AwaClientGetResponse_ContainsPath(response, str))
    {
        /* Test whether the response has a valid value for the specified path */
        if (AwaClientGetResponse_HasValue(response, str))
        {
            /* Retrieve the value, as a C-style string */
            const char * value;
            AwaClientGetResponse_GetValueAsCStringPointer(response, str, &value);
            ret = strdup(value);
        }
    }

    AwaClientGetOperation_Free(&operation);
    free(str);
    return ret;
}

static void createContainerInstance(struct lxc_agent *a, int instance, char *name)
{
    char *appId = NULL;

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

        appId = getAppID(a->session);

        printf("Got AppID: %s\n", appId);
        free(appId);

        ci = calloc(1, sizeof(struct container_info));
        LIST_INSERT_HEAD(a->cl, ci, list);
        createContainer(ci, name);
        startContainer(ci->c);

        ci->name = name;
        ci->instance = instance;
        ci->agent = a;

        AwaClientExecuteSubscription *startSub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_START), startCallback, (void*)ci);
        free(str);
        AwaClientExecuteSubscription *stopSub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_STOP), stopCallback, (void*)ci);
        free(str);
        AwaClientExecuteSubscription *destroySub = AwaClientExecuteSubscription_New(str = resourceInstance(LXC_OBJID, instance, LXC_RESID_DESTROY), destroyCallback, (void*)ci);
        free(str);

        ci->startSub = startSub;
        ci->stopSub = startSub;
        ci->destroySub = destroySub;

        /* Start listening to notifications */
        AwaClientSubscribeOperation * subscribeOperation = AwaClientSubscribeOperation_New(a->session);

        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, startSub);
        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, stopSub);
        AwaClientSubscribeOperation_AddExecuteSubscription(subscribeOperation, destroySub);

        AwaClientSubscribeOperation_Perform(subscribeOperation, OPERATION_PERFORM_TIMEOUT);
        AwaClientSubscribeOperation_Free(&subscribeOperation);
        printf("Container instance: %d created\n", ci->instance);
    }
}

static void destroyContainerInstance(struct container_info *ci)
{
    struct lxc_container *c = ci->c;

    if (c && !strncmp(c->state(c), "STOPPED", strlen(c->state(c))))
    {
        AwaClientSubscribeOperation *cancelSubscribeOperation = AwaClientSubscribeOperation_New(ci->agent->session);

        AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, ci->startSub);
        AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, ci->stopSub);
        AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, ci->destroySub);

        AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
        AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);

        // AwaClientExecuteSubscription_Free(&(ci->startSub));
        // AwaClientExecuteSubscription_Free(&(ci->stopSub));
        // AwaClientExecuteSubscription_Free(&(ci->destroySub));

        AwaClientDeleteOperation *operation = AwaClientDeleteOperation_New(ci->agent->session);
        AwaClientDeleteOperation_AddPath(operation, objectInstance(LXC_OBJID, ci->instance));

        AwaClientDeleteOperation_Perform(operation, OPERATION_PERFORM_TIMEOUT);
        AwaClientDeleteOperation_Free(&operation);

        destroyContainer(ci);
        LIST_REMOVE(ci, list);
        printf("Container instance: %d destroyed\n", ci->instance);
    }
    else
    {
        printf("Container not stopped: %s\n", c->state(c));
    }

}

static int createContainer(struct container_info *ci, char *name)
{
    struct lxc_container *c;
    int ret = 0;
    /* Setup container struct */
    c = lxc_container_new(name, NULL);
    if (!c) {
        fprintf(stderr, "Failed to setup lxc_container struct\n");
        ret = 1;
    }
    ci->c = c;

    if (c->is_defined(c)) {
        fprintf(stderr, "Container already exists\n");
        ret = 1;
    }

#ifdef CI40_TEMPLATE
    /* Create the container */
    if (!c->createl(c, "template", NULL, NULL, LXC_CREATE_QUIET,
                    name, NULL)) {
        fprintf(stderr, "Failed to create container rootfs\n");
        ret = 1;
    }
#else
    /* Create the container */
    if (!c->createl(c, "download", NULL, NULL, LXC_CREATE_QUIET,
                    "-d", "ubuntu", "-r", "trusty", "-a", "i386", NULL)) {
        fprintf(stderr, "Failed to create container rootfs\n");
        ret = 1;
    }
#endif

    if (!ret)
    {
        /* Query some information */
        printf("Container state: %s\n", c->state(c));
    }
    return ret;
}

static int startContainer(struct lxc_container *c)
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

static int stopContainer(struct lxc_container *c)
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
    struct lxc_agent *a = (struct lxc_agent*)context;

    static int instance = 0;
    printf("Create Callback received. Context = %p\n", a);
    createContainerInstance(a, instance, NULL);
    instance++;

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void startCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Start Callback received. Context = %p\n", ci);
    startContainer(ci->c);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void stopCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Stop Callback received. Context = %p\n", ci);
    stopContainer(ci->c);

    printf("Resource executed [%zu bytes payload]\n", arguments->Size);
}

static void destroyCallback(const AwaExecuteArguments * arguments, void * context)
{
    struct container_info *ci = (struct container_info*) context;

    printf("Destroy Callback received. Context = %p\n", ci);
    destroyContainerInstance(ci);

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

    DefineLxcAgentClientObject(session);
    CreateAgentInstance(session);
    DefineLxcClientObject(session);
    InitDevice(session);

    /* Application-specific data */
    struct lxc_agent agent;
    struct container_list cl_head;
    LIST_INIT(&cl_head);
    agent.cl = &cl_head;
    agent.session = session;

    AwaClientExecuteSubscription * createSub = AwaClientExecuteSubscription_New(RESOURCE_INSTANCE(LXC_AGENT_OBJID, 0, LXCA_RESID_CREATE), createCallback, (void*)&agent);

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
        updateContainerObjectsList(&agent);
        // DeviceControl(session);
    }

    AwaClientSubscribeOperation * cancelSubscribeOperation = AwaClientSubscribeOperation_New(session);

    AwaClientSubscribeOperation_AddCancelExecuteSubscription(cancelSubscribeOperation, createSub);

    AwaClientSubscribeOperation_Perform(cancelSubscribeOperation, OPERATION_PERFORM_TIMEOUT);
    AwaClientSubscribeOperation_Free(&cancelSubscribeOperation);

    /* Free the execute subscription */
    AwaClientExecuteSubscription_Free(&createSub);

    AwaClientSession_Disconnect(session);
    AwaClientSession_Free(&session);
    return 0;
}
