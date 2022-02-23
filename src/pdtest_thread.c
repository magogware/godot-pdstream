/*
  this tests pd's currently *experimental* multi instance support 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <z_libpd.h>
#include <wavfile.h>

#define LIBPD_TEST_NINSTANCES   2
#define LIBPD_TEST_NLOOPS       10000

static const char *dir = "./patches";
static const char *files[LIBPD_TEST_NINSTANCES] = {"sawtooth.pd", "osc.pd"};
static const char *outfiles[LIBPD_TEST_NINSTANCES] = {"sawtooth.wav", "osc.wav"};

typedef struct l_instance
{
    t_pdinstance*       l_pd;
    int8_t              l_id;
    size_t              l_blocksize;
    size_t              l_samplerate;
    size_t              l_ninputs;
    short*              l_inputs;
    size_t              l_noutputs;
    short*              l_outputs;
    char                l_file[MAXPDSTRING];
    char                l_folder[MAXPDSTRING];
    void*               l_patch;
    char                l_wavname[MAXPDSTRING];
    FILE*               l_wavfile;
    
    pthread_t           l_thd;
} t_libpd_instance;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doinit(t_libpd_instance* inst)
{
    inst->l_pd = libpd_new_instance();
    libpd_set_instance(inst->l_pd);
    assert(inst->l_pd && "pd instance can't be allocated.");
    libpd_init_audio((int)inst->l_ninputs, (int)inst->l_noutputs, (int)inst->l_samplerate);
    return NULL;
}

static void libpd_instance_init(t_libpd_instance* inst,
                                size_t blksize, size_t samplerate, size_t nins, size_t nouts)
{
    
    inst->l_blocksize   = blksize;
    inst->l_samplerate  = samplerate;
    inst->l_ninputs     = nins;
    inst->l_noutputs    = nouts;
    inst->l_patch       = NULL;
    
    assert(blksize && nins && nouts && "block size, number of inputs and number of outputs must be positives");
    inst->l_inputs      = (short *)malloc(blksize * nins * sizeof(*inst->l_inputs));
    assert(inst->l_inputs && "inputs can't be allocated.");
    inst->l_outputs      = (short *)malloc(blksize * nouts * sizeof(*inst->l_outputs));
    assert(inst->l_outputs && "outputs can't be allocated.");
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doinit, inst) &&
           "libpd_instance_init thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_dofree(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    if(inst->l_pd) {
        libpd_free_instance(inst->l_pd); }
    return NULL;
}

static void libpd_instance_free(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dofree, inst) &&
           "thread creation error.");
    pthread_join(inst->l_thd, NULL);
    if(inst->l_inputs)
    {
        free(inst->l_inputs);
        inst->l_inputs = NULL;
        inst->l_ninputs = 0;
    }
    if(inst->l_outputs)
    {
        free(inst->l_outputs);
        inst->l_outputs = NULL;
        inst->l_noutputs = 0;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_dodsp_start(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(1.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_start(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_start, inst) &&
           "libpd_instance_dsp_start thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

static void* libpd_instance_dodsp_stop(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    libpd_start_message(1);
    libpd_add_float(0.f);
    libpd_finish_message("pd", "dsp");
    return NULL;
}

static void libpd_instance_dsp_stop(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_dodsp_stop, inst) &&
           "libpd_instance_dsp_stop thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doclose(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    assert(inst->l_patch && "patch not loaded so can't be closed");
    libpd_closefile(inst->l_patch);
    wavfile_close(inst->l_wavfile);
    return NULL;
}

static void libpd_instance_close(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doclose, inst) &&
           "libpd_instance_close thread creation error.");
    pthread_join(inst->l_thd, NULL);
    inst->l_patch = NULL;
}

static void* libpd_instance_doopen(t_libpd_instance* inst)
{
    libpd_set_instance(inst->l_pd);
    assert((inst->l_patch = libpd_openfile(inst->l_file, inst->l_folder)) &&
           "patch can't be loaded");
    inst->l_wavfile = wavfile_open(inst->l_wavname);
    return NULL;
}

static void libpd_instance_open(t_libpd_instance* inst, const char **file, const char *folder, const char **outs)
{
    if(inst->l_patch) {
        libpd_instance_close(inst); }
    strncpy(inst->l_file, file[inst->l_id], MAXPDSTRING);
    strncpy(inst->l_folder, folder, MAXPDSTRING);
    strncpy(inst->l_wavname, outs[inst->l_id], MAXPDSTRING);
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doopen, inst) &&
           "libpd_instance_open thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* libpd_instance_doperform(t_libpd_instance* inst)
{
    size_t i;
    libpd_set_instance(inst->l_pd);
    libpd_process_short((int)(inst->l_blocksize / (size_t)64), inst->l_inputs, inst->l_outputs);
    wavfile_write(inst->l_wavfile, inst->l_outputs, inst->l_blocksize);
    // for(i = 0; i < inst->l_blocksize; ++i) {
    //     int result   = (int)inst->l_outputs[i];
    //     int expected = i%2 ? ((i-1)/2)%64 * -1 : (i/2)%64;
    //     assert(result == expected && "DSP results are wrong"); }
    return NULL;
}

static void libpd_instance_perform(t_libpd_instance* inst)
{
    assert(!pthread_create(&inst->l_thd, NULL, (void *)libpd_instance_doperform, inst) &&
           "libpd_instance_perform thread creation error.");
    pthread_join(inst->l_thd, NULL);
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

static void* multi_instance_run(t_libpd_instance* inst)
{
    size_t i;
    libpd_instance_init(inst, 64, WAVFILE_SAMPLES_PER_SECOND, 1, 1);
    libpd_instance_open(inst, files, dir, outfiles);
    libpd_instance_dsp_start(inst);
    for(i = 0; i < LIBPD_TEST_NLOOPS; ++i) {
        libpd_instance_perform(inst); }
    libpd_instance_dsp_stop(inst);
    libpd_instance_close(inst);
    
    libpd_instance_free(inst);
    return NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    size_t i;
    pthread_t threads[LIBPD_TEST_NINSTANCES];
    t_libpd_instance instance[LIBPD_TEST_NINSTANCES];

    libpd_init();
        
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        (instance+i)->l_id = i;
        assert(!pthread_create(threads+i, NULL, (void *)multi_instance_run, instance+i) &&
               "multi_instance_run thread creation error.");
    }
    for(i = 0; i < LIBPD_TEST_NINSTANCES; ++i)
    {
        pthread_join(threads[i], NULL);
    }
    return 0;
}
