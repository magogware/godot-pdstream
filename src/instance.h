#ifndef __PDS_INSTANCE_H__
#define __PDS_INSTANCE_H__

#define PDS_ERR_SUCCESS 0
#define PDS_ERR_INIT -1
#define PDS_ERR_AUDIO_INIT -2
#define PDS_ERR_PROCESS -3
#define PDS_ERR_NO_RECV -4
#define PDS_ERR_MSG_LEN -5
#define PDS_ERR_FILE -6
#define PDS_ERR_INIT_INST -7

#define PDS_NINPUTS 2
#define PDS_NOUTPUTS 2

#include <z_libpd.h>

typedef struct instance_t
{
    t_pdinstance*       instance;
    size_t              blocksize;
    size_t              samplerate;
    float*              inputs;
    float*              outputs;
    char                file[260];
    char                folder[260];
    void*               patch;
} instance_t;

int pd_init();

int create(instance_t *inst, size_t blocksize, size_t samplerate, float *inputs, float *outputs);
void destroy(instance_t *inst);

int open(instance_t *inst, char *file, char *dir);
void close(instance_t *inst);

int start_dsp(instance_t *inst);
int stop_dsp(instance_t *inst);

int bang(instance_t *inst, char *recv);
int flot(instance_t *inst, char *recv, float x);
int symbol(instance_t *inst, char *recv, char *symbol);

void add_float(instance_t *inst, float x);
void add_symbol(instance_t *inst, char *s);
int finish_list(instance_t *inst, char *s);
int start_message(instance_t *inst, int length);
int finish_message(instance_t *inst, char *recv, char *msg);

int perform(instance_t *inst);

#endif