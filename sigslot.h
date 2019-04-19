/*
 * Copyright (c) 2019 Zhou-XiuLong
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __SIG_SLOT_H
#define __SIG_SLOT_H

#include "list.h"

typedef struct _c_slot {
    struct list_node lnode;
    void * function;
} c_slot_t;

typedef struct _c_signal_slot {
    struct list_node slots;
} signal_slot_t;

#define program_name() ({       \
    extern char * __progname;   \
    __progname;                 \
})

#define c_unused(var)   (void)var;

#define c_signal(_signal_name)   signal_slot_t _signal_name

#define c_signal_initialize(_object, _signal_name) do{                                                  \
            list_initialize(&((_object)->_signal_name.slots));                                          \
        }while(0);                                  
                                    
#define c_signal_connector(_object, _signal_name, _slot) do{                                            \
            c_slot_t * slot = NULL;                                                                     \
            if((slot = malloc(sizeof(c_slot_t))) == NULL) {                                             \
                /* Error occured : Maybe is out of memory */                                            \
            } else {                                                                                    \
                slot->function = _slot;                                                                 \
                list_initialize(&(slot->lnode));                                                        \
                list_add_tail(&((_object)->_signal_name.slots), &(slot->lnode));                        \
            }                                                                                           \
        }while(0);                                  
                                    
#define c_signal_connected(_object, _signal_name, _slot) ({                                             \
            bool connected = false;                                                                     \
            c_slot_t *slot = NULL;                                                                      \
            list_for_every_entry(&((_object)->_signal_name.slots), slot, c_slot_t, lnode) {             \
                if(slot->function == _slot) {                                                           \
                    connected = true;                                                                   \
                }                                                                                       \
            }                                                                                           \
            connected;                                                                                  \
        })                                  
                                    
#define c_signal_disconnect(_object, _signal_name, _slot) do{                                           \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                if(slot->function == _slot) {                                                           \
                    list_delete(&(slot->lnode));                                                        \
                    free(slot);                                                                         \
                }                                                                                       \
            }                                                                                           \
        }while(0);                                  
                                    
#define c_signal_disconnect_all(_object, _signal_name) do{                                              \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                    list_delete(&(slot->lnode));                                                        \
                    free(slot);                                                                         \
            }                                                                                           \
        }while(0);                                  
                                    
#define c_signal_emit(_object, _signal_name, _signal_type, ...) do{                                     \
            c_slot_t *slot = NULL;                                                                      \
            list_for_every_entry(&((_object)->_signal_name.slots), slot, c_slot_t, lnode) {             \
                if(slot->function != NULL) {                                                            \
                    ((_signal_type)(slot->function))(__VA_ARGS__);                                      \
                }                                                                                       \
            }                                                                                           \
        }while(0);

#endif
