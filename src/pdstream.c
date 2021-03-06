#include <gdnative_api_struct.gen.h>
#include <string.h>

#include "instance.h"

const godot_gdnative_core_api_struct *core_api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;
const godot_gdnative_ext_nativescript_1_1_api_struct *nativescript_ext_api = NULL;
godot_string res_prefix;
godot_string dir_prefix;

void error(int code, const char *function, int line)
{
    switch (code) {
        case PDS_ERR_SUCCESS :
            break;
        case PDS_ERR_INIT :
            core_api->godot_print_warning("libpd already initialised", function, "pdstream.c", line);
            break;
        case PDS_ERR_AUDIO_INIT :
            core_api->godot_print_error("Couldn't initialise audio!", function, "pdstream.c", line);
            break;
        case PDS_ERR_PROCESS :
            core_api->godot_print_error("Couldn't process audio frames!", function, "pdstream.c", line);
            break;
        case PDS_ERR_NO_RECV :
            core_api->godot_print_warning("Receiver for message doesn't exist", function, "pdstream.c", line);
            break;
        case PDS_ERR_MSG_LEN :
            core_api->godot_print_error("Message too long!", function, "pdstream.c", line);
            break;
        case PDS_ERR_FILE :
            core_api->godot_print_warning("Couldn't open patch file", function, "pdstream.c", line);
            break;
        case PDS_ERR_INIT_INST :
            core_api->godot_print_error("Couldn't initialise libpd instance!", function, "pdstream.c", line);
            break;
    }
}

char *variant_to_chars(godot_variant *s)
{
    godot_string string = core_api->godot_variant_as_string(s);
    godot_char_string chars = core_api->godot_string_ascii(&string);
    int size = core_api->godot_char_string_length(&chars);
    char *res = core_api->godot_alloc(size+1);

    strncpy(res, core_api->godot_char_string_get_data(&chars), size);
    res[size] = '\0'; // this was a fun bug to discover and fix

    core_api->godot_string_destroy(&string);
    core_api->godot_char_string_destroy(&chars);
    return res;
}

char *string_to_chars(godot_string s)
{
    godot_char_string chars = core_api->godot_string_ascii(&s);
    int size = core_api->godot_char_string_length(&chars);
    char *res = core_api->godot_alloc(size+1);

    strncpy(res, core_api->godot_char_string_get_data(&chars), size);
    res[size] = '\0';

    core_api->godot_char_string_destroy(&chars);
    return res;
}

void *pdstream_constructor(godot_object *p_instance, void *p_method_data) {
    instance_t *user_data;

    user_data = core_api->godot_alloc(sizeof(instance_t));

    return user_data;
}

void pdstream_destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
    instance_t *inst = (instance_t *) p_user_data;

    destroy(inst);
    if(inst->inputs) {
        core_api->godot_free(inst->inputs);
        inst->inputs = NULL;
    }
    if(inst->outputs) {
        core_api->godot_free(inst->outputs);
        inst->outputs = NULL;
    }
    core_api->godot_free(p_user_data);
}

godot_variant pdstream_create(godot_object *p_instance,
                            void *p_method_data,
                            void *p_user_data,
                            int p_num_args,
                            godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    float *inputs, *outputs;
    size_t blocksize;
    int64_t samplerate;
    int block_assign_line;

    blocksize  = 64;
    samplerate = 44100;

    switch (p_num_args) {
        case 0: {
            break;
        } case 1: {
            blocksize  = core_api->godot_variant_as_int(p_args[0]);
            block_assign_line = __LINE__ - 1;
            break;
        } case 2: {
            blocksize  = core_api->godot_variant_as_int(p_args[0]);
            samplerate = core_api->godot_variant_as_int(p_args[1]);
            break;
        }
    }
    
    if (blocksize % 64) {
        core_api->godot_print_error("Blocksize must be a multiple of 64!", __func__, "pdstream.c", block_assign_line);
    } else {
        inputs =  (float *)core_api->godot_alloc(blocksize * PDS_NINPUTS *  sizeof(*inst->inputs));
        outputs = (float *)core_api->godot_alloc(blocksize * PDS_NOUTPUTS * sizeof(*inst->outputs));

        error(create(inst, blocksize, samplerate, inputs, outputs), __func__, __LINE__);
    }
}


godot_variant pdstream_open(godot_object *p_instance,
                                  void *p_method_data,
                                  void *p_user_data,
                                  int p_num_args,
                                  godot_variant **p_args)
{

    // FIXME: Opening twice in a row on the same object (without a second object instantiated) crashes... Is the close function working?

    instance_t *inst = (instance_t *) p_user_data;
    godot_string full_path = core_api->godot_variant_as_string(p_args[0]); // store path and file separately for libpd
    godot_string file_str = core_api->godot_string_get_file(&full_path);
    godot_string replaced, dir_str = core_api->godot_string_get_base_dir(&full_path);
    char *file = string_to_chars(file_str); // FIXME: potential memory leak here with the string argument
    char *dir;

    // Godot uses res:// to refer to the project root, but that has no meaning here, so we replace it with ./
    replaced = core_api->godot_string_replace_first(&dir_str, res_prefix, dir_prefix);
    dir = string_to_chars(replaced);

    error(open(inst, file, dir), __func__, __LINE__);

    core_api->godot_string_destroy(&full_path);
    core_api->godot_string_destroy(&file_str);
    core_api->godot_string_destroy(&replaced);
    core_api->godot_string_destroy(&dir_str);
    core_api->godot_free(file);
    core_api->godot_free(dir);
}

godot_method_arg pdstream_open_args()
{
    godot_method_arg arg;

    godot_string arg_path_name;
    godot_variant_type arg_path_type = GODOT_VARIANT_TYPE_STRING;
    godot_property_hint arg_path_hint = GODOT_PROPERTY_HINT_FILE;
    godot_string arg_path_hint_string;

    core_api->godot_string_new(&arg_path_name);
    core_api->godot_string_parse_utf8(&arg_path_name, "patch_path");

    core_api->godot_string_new(&arg_path_hint_string);
    core_api->godot_string_parse_utf8(&arg_path_hint_string, "Path to Pd patch");

    arg.name = arg_path_name;
    arg.type = arg_path_type;
    arg.hint = arg_path_hint;
    arg.hint_string = arg_path_hint_string;

    return arg;
}

godot_variant pdstream_perform(godot_object *p_instance,
                                     void *p_method_data,
                                     void *p_user_data,
                                     int p_num_args,
                                     godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    godot_pool_vector2_array data;
    godot_vector2 frame;
    godot_variant rval;
    int i, j;
    int64_t numblocks = core_api->godot_variant_as_int(p_args[0]);
    
    core_api->godot_vector2_new(&frame, 1.0, 1.0);
    core_api->godot_pool_vector2_array_new(&data);
    
    error(start_dsp(inst), __func__, __LINE__);
    for (i = 0; i < numblocks; i++) {
        error(perform(inst), __func__, __LINE__);
        for (j = 0; j < (PDS_NOUTPUTS * inst->blocksize); j+=2) {
            core_api->godot_vector2_set_x(&frame, inst->outputs[j]);
            core_api->godot_vector2_set_y(&frame, inst->outputs[j+1]);
            core_api->godot_pool_vector2_array_append(&data, &frame);
        }
    }
    error(stop_dsp(inst), __func__, __LINE__);

    core_api->godot_variant_new_pool_vector2_array(&rval, &data);
    core_api->godot_pool_vector2_array_destroy(&data);
    return rval;
}

godot_variant pdstream_float(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    char *recv = variant_to_chars(p_args[0]);
    float msg = (float) core_api->godot_variant_as_real(p_args[1]);

    error(flot(inst, recv, msg), "pdstream_float", __LINE__);

    core_api->godot_free(recv);
}

godot_variant pdstream_bang(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    char *recv = variant_to_chars(p_args[0]);

    error(bang(inst, recv), __func__, __LINE__);

    core_api->godot_free(recv);
}

godot_variant pdstream_symbol(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    char *recv = variant_to_chars(p_args[0]);
    char *s = variant_to_chars(p_args[1]);

    error(symbol(inst, recv, s), __func__, __LINE__);

    core_api->godot_free(recv);
    core_api->godot_free(s);
}

godot_variant pdstream_add_float(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    float msg = (float) core_api->godot_variant_as_real(p_args[0]);

    add_float(inst, msg);
}

godot_variant pdstream_add_symbol(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    char *symbol = variant_to_chars(p_args[0]);

    add_symbol(inst, symbol);

    core_api->godot_free(symbol);
}

godot_variant pdstream_start_message(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    int length = core_api->godot_variant_as_int(p_args[0]);

    error(start_message(inst, length), __func__, __LINE__);
}

godot_variant pdstream_finish_message(godot_object *p_instance,
                             void *p_method_data,
                             void *p_user_data,
                             int p_num_args,
                             godot_variant **p_args)
{
    instance_t *inst = (instance_t *) p_user_data;
    char *recv = variant_to_chars(p_args[0]);
    char *msg = variant_to_chars(p_args[1]);

    error(finish_message(inst, recv, msg), __func__, __LINE__);

    core_api->godot_free(recv);
    core_api->godot_free(msg);
}

void GDN_EXPORT godot_nativescript_init(void *p_handle)
{
    godot_instance_create_func creator = { &pdstream_constructor, NULL, NULL };
    godot_instance_destroy_func destroyer = { &pdstream_destructor, NULL, NULL };

    godot_instance_method create = { &pdstream_create, NULL, NULL};
    godot_method_attributes create_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    godot_instance_method open = { &pdstream_open, NULL, NULL };
    godot_method_attributes open_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_method_arg open_args = pdstream_open_args();

    godot_instance_method perform = { &pdstream_perform, NULL, NULL };
    godot_method_attributes perform_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    godot_instance_method flot = { &pdstream_float, NULL, NULL };
    godot_method_attributes flot_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_instance_method bang = { &pdstream_bang, NULL, NULL };
    godot_method_attributes bang_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_instance_method symbol = { &pdstream_symbol, NULL, NULL };
    godot_method_attributes symbol_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    godot_instance_method start_message = { &pdstream_start_message, NULL, NULL };
    godot_method_attributes start_message_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_instance_method finish_message = { &pdstream_finish_message, NULL, NULL };
    godot_method_attributes finish_message_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_instance_method add_float = { &pdstream_add_float, NULL, NULL };
    godot_method_attributes add_float_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
    godot_instance_method add_symbol = { &pdstream_add_symbol, NULL, NULL };
    godot_method_attributes add_symbol_attributes = { GODOT_METHOD_RPC_MODE_DISABLED };

    nativescript_api->godot_nativescript_register_class(p_handle, "PdStream", "Reference",
            creator, destroyer);

    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "create",
            create_attributes, create);

    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "open",
            open_attributes, open);
    nativescript_ext_api->godot_nativescript_set_method_argument_information(p_handle, "PdStream", "open", 1, &open_args);

    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "perform",
        perform_attributes, perform);

    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "flot",
        flot_attributes, flot);
    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "bang",
        bang_attributes, bang);
    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "symbol",
        symbol_attributes, symbol);

    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "start_message",
        start_message_attributes, start_message);
    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "finish_message",
        finish_message_attributes, finish_message);
    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "add_float",
        add_float_attributes, add_float);
    nativescript_api->godot_nativescript_register_method(p_handle, "PdStream", "add_symbol",
        add_symbol_attributes, add_symbol);
    
    error(pd_init(), __func__, __LINE__);
}

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *p_options)
{
    core_api = p_options->api_struct;
    core_api->godot_string_parse_utf8(&res_prefix, "res://");
    core_api->godot_string_parse_utf8(&dir_prefix, "./");

    // Now find our extensions.
    for (int i = 0; i < core_api->num_extensions; i++) {
        switch (core_api->extensions[i]->type) {
            case GDNATIVE_EXT_NATIVESCRIPT: {
                nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)core_api->extensions[i];
                if (nativescript_api->next)
                    nativescript_ext_api = (godot_gdnative_ext_nativescript_1_1_api_struct *)nativescript_api->next;
            }; break;
            default: break;
        }
    }

    // for (int i = 0; i < core_api->num_extensions-1; i++) {
    //     if (core_api->extensions[i]->next)
    //         printf("Next in array has version %d %d, next via pointer has version %d %d\n",
    //             core_api->extensions[i+1]->version.major,
    //             core_api->extensions[i+1]->version.minor,
    //             core_api->extensions[i]->next->version.major,
    //             core_api->extensions[i]->next->version.minor);
    //     if (core_api->extensions[i]->type == GDNATIVE_EXT_NATIVESCRIPT)
    //         printf("Oh, also, this one is a Nativescript API\n");
    // }
}

void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *p_options) {
    core_api = NULL;
    nativescript_api = NULL;
    nativescript_ext_api = NULL;
}