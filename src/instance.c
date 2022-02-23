#include <string.h>

#include "instance.h"

typedef struct args_t {
    instance_t *inst;
    void       *args;
} args_t;

typedef struct msg_t {
    char       *recv;
    void       *data;
} msg_t;

int pd_init()
{
    return libpd_init();
}

void do_create(instance_t *inst)
{
    printf("Doing an init\n");
    inst->instance = libpd_new_instance();
    libpd_set_instance(inst->instance);
    libpd_init_audio((int)inst->ninputs, (int)inst->noutputs, (int)inst->samplerate);
}

int create(instance_t *inst, size_t blocksize, size_t samplerate, size_t ninputs, size_t noutputs, short *inputs, short *outputs)
{
    inst->blocksize   = blocksize;
    inst->samplerate  = samplerate;
    inst->ninputs     = ninputs;
    inst->noutputs    = noutputs;
    inst->patch       = NULL;
    
    inst->inputs      = inputs;
    inst->outputs     = outputs;

    do_create(inst);

    if (!inst->instance)
        return PDS_INSTANCE_ERROR;
    else
        return PDS_INSTANCE_SUCCESS;
}

void do_destroy(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    if(inst->instance) {
        libpd_free_instance(inst->instance); }
}

int destroy(instance_t *inst)
{
    do_destroy(inst);

    return PDS_INSTANCE_SUCCESS;
}

void do_open(args_t *args)
{
    char **strs = (char **) args->args;
    libpd_set_instance(args->inst->instance);
    args->inst->patch = libpd_openfile(strs[0], strs[1]);
}

int open(instance_t *inst, char *file, char *dir)
{
    if(inst->patch)
        close(inst);

    char *strs[2] = { file, dir };
    args_t args = { inst, strs };

    do_open(&args);

    if (!inst->patch)
        return PDS_INSTANCE_ERROR;
    else
        return PDS_INSTANCE_SUCCESS;
}

void do_close(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    libpd_closefile(inst->patch);
}

int close(instance_t *inst)
{
    do_close(inst);
    inst->patch = NULL;

    return PDS_INSTANCE_SUCCESS;
}

void do_start_dsp(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    libpd_start_message(1);
    libpd_add_float(1.f);
    libpd_finish_message("pd", "dsp");
}

int start_dsp(instance_t *inst)
{
    do_start_dsp(inst);

    return PDS_INSTANCE_SUCCESS;
}

void do_stop_dsp(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    libpd_start_message(1);
    libpd_add_float(0.f);
    libpd_finish_message("pd", "dsp");
}

int stop_dsp(instance_t *inst)
{
    do_stop_dsp(inst);
    return PDS_INSTANCE_SUCCESS;
}

void do_add_float(args_t *args)
{
    libpd_set_instance(args->inst->instance);
    libpd_add_float(*((float *) args->args));
}

int add_float(instance_t *inst, float x)
{
    args_t args = { inst, &x };
    do_add_float(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_add_symbol(args_t *args)
{
    libpd_set_instance(args->inst->instance);
    libpd_add_symbol(((char *) args->args));
}

int add_symbol(instance_t *inst, char *s)
{
    args_t args = { inst, s };
    do_add_symbol(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_finish_list(args_t *args)
{
    libpd_set_instance(args->inst->instance);
    libpd_finish_list(((char *) args->args));
}

int finish_list(instance_t *inst, char *s)
{
    args_t args = { inst, s };
    do_finish_list(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_start_message(args_t *args)
{
    libpd_set_instance(args->inst->instance);
    libpd_start_message(*((int *) args->args));
}

int start_message(instance_t *inst, int length)
{
    args_t args = { inst, &length };
    do_start_message(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_finish_message(args_t *args)
{
    char **strs = (char **) args->args;

    libpd_set_instance(args->inst->instance);
    libpd_finish_message(strs[0], strs[1]);
}

int finish_message(instance_t *inst, char *recv, char *msg)
{
    char *strs[2] = {recv, msg};
    args_t args = { inst, strs };
    do_finish_message(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_bang(args_t *args)
{
    msg_t *msg = (msg_t *) args->args;
    libpd_set_instance(args->inst->instance);
    libpd_bang(msg->recv);
}

int bang(instance_t *inst, char *recv)
{
    msg_t msg = { recv, NULL };
    args_t args = { inst, &msg };
    do_bang(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_float(args_t *args)
{
    msg_t *msg = (msg_t *) args->args;
    libpd_set_instance(args->inst->instance);
    libpd_float(msg->recv, *((float *) msg->data));
}

int flot(instance_t *inst, char *recv, float x)
{
    msg_t msg = { recv, &x };
    args_t args = { inst, &msg };
    do_float(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_symbol(args_t *args)
{
    msg_t *msg = (msg_t *) args->args;
    libpd_set_instance(args->inst->instance);
    libpd_symbol(msg->recv, ((char *) msg->data));
}

int symbol(instance_t *inst, char *recv, char *symbol)
{
    msg_t msg = { recv, symbol };
    args_t args = { inst, &msg };
    do_symbol(&args);

    return PDS_INSTANCE_SUCCESS;
}

void do_perform(instance_t *inst)
{
    size_t i;
    libpd_set_instance(inst->instance);
    libpd_process_short((int)(inst->blocksize / (size_t)64), inst->inputs, inst->outputs);
}

int perform(instance_t *inst)
{
    do_perform(inst);
    return PDS_INSTANCE_SUCCESS;
}

