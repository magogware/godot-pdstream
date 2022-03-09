#include <string.h>

#include "instance.h"

typedef struct msg_t {
    char       *recv;
    void       *data;
} msg_t;

int pd_init()
{
    return libpd_init();
}

int create(instance_t *inst, size_t blocksize, size_t samplerate, size_t ninputs, size_t noutputs, float *inputs, float *outputs)
{
    inst->blocksize   = blocksize;
    inst->samplerate  = samplerate;
    inst->ninputs     = ninputs;
    inst->noutputs    = noutputs;
    inst->patch       = NULL;
    inst->inputs      = inputs;
    inst->outputs     = outputs;

    inst->instance = libpd_new_instance();
    libpd_set_instance(inst->instance);
    
    if (!inst->instance)
        return PDS_INSTANCE_ERROR;
    return libpd_init_audio((int)inst->ninputs, (int)inst->noutputs, (int)inst->samplerate);
}

void destroy(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    if(inst->instance) {
        libpd_free_instance(inst->instance);
    }
}

int open(instance_t *inst, char *file, char *dir)
{
    if(inst->patch)
        close(inst);

    libpd_set_instance(inst->instance);
    inst->patch = libpd_openfile(file, dir);

    if (!inst->patch)
        return PDS_INSTANCE_ERROR; // TODO: report file not opened
    else
        return PDS_INSTANCE_SUCCESS;
}

void close(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    libpd_closefile(inst->patch);
    inst->patch = NULL;
}

int start_dsp(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    if (libpd_start_message(1))
        return PDS_INSTANCE_ERROR; // TODO: report length too long
    libpd_add_float(1.f);
    if (libpd_finish_message("pd", "dsp"))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int stop_dsp(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    if (libpd_start_message(1))
        return PDS_INSTANCE_ERROR; // TODO: report length too long
    libpd_add_float(0.f);
    if (libpd_finish_message("pd", "dsp"))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent
    
    return PDS_INSTANCE_SUCCESS;
}

void add_float(instance_t *inst, float x)
{
    libpd_set_instance(inst->instance);
    libpd_add_float(x);
}

void add_symbol(instance_t *inst, char *s)
{
    libpd_set_instance(inst->instance);
    libpd_add_symbol(s);
}

int finish_list(instance_t *inst, char *s)
{
    libpd_set_instance(inst->instance);
    if (libpd_finish_list(s))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int start_message(instance_t *inst, int length)
{
    libpd_set_instance(inst->instance);
    if (libpd_start_message(length))
        return PDS_INSTANCE_ERROR; // TODO: report length too long

    return PDS_INSTANCE_SUCCESS;
}

int finish_message(instance_t *inst, char *recv, char *msg)
{
    libpd_set_instance(inst->instance);
    if (libpd_finish_message(recv, msg))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int bang(instance_t *inst, char *recv)
{
    libpd_set_instance(inst->instance);
    if (libpd_bang(recv))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int flot(instance_t *inst, char *recv, float x)
{
    libpd_set_instance(inst->instance);
    if (libpd_float(recv, x))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int symbol(instance_t *inst, char *recv, char *symbol)
{
    libpd_set_instance(inst->instance);
    if (libpd_symbol(recv, symbol))
        return PDS_INSTANCE_ERROR; // TODO: report receiver name non-existent

    return PDS_INSTANCE_SUCCESS;
}

int perform(instance_t *inst)
{
    libpd_set_instance(inst->instance);
    if (libpd_process_float((int)(inst->blocksize / (size_t)64), inst->inputs, inst->outputs))
        return PDS_INSTANCE_ERROR; // TODO: report process fail
    return PDS_INSTANCE_SUCCESS;
}

