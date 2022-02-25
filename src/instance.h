#ifndef __PDS_INSTANCE_H__
#define __PDS_INSTANCE_H__

#define PDS_INSTANCE_ERROR -1
#define PDS_INSTANCE_SUCCESS 0

#include <z_libpd.h>

typedef struct instance_t
{
    t_pdinstance*       instance;
    size_t              blocksize;
    size_t              samplerate;
    size_t              ninputs;
    float*              inputs;
    size_t              noutputs;
    float*              outputs;
    char                file[260];
    char                folder[260];
    void*               patch;
} instance_t;

int pd_init();

int create(instance_t *inst, size_t blocksize, size_t samplerate, size_t ninputs, size_t noutputs, float *inputs, float *outputs);
int destroy(instance_t *inst);

int open(instance_t *inst, char *file, char *dir);
int close(instance_t *inst);

int start_dsp(instance_t *inst);
int stop_dsp(instance_t *inst);

int bang(instance_t *inst, char *recv);
int flot(instance_t *inst, char *recv, float x);
int symbol(instance_t *inst, char *recv, char *symbol);

int add_float(instance_t *inst, float x);
int add_symbol(instance_t *inst, char *s);
int finish_list(instance_t *inst, char *s);
int start_message(instance_t *inst, int length);
int finish_message(instance_t *inst, char *recv, char *msg);

int perform(instance_t *inst);

#endif