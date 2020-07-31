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
#ifndef __L_SIG_SLOT_H
#define __L_SIG_SLOT_H

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>


/************************************************************************/
#define _program_name() ({                                      \
            extern char * __progname;                           \
            __progname;                                         \
        })

#define containerof(ptr, type, member) ({                       \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);\
            (type *)( (char *)__mptr - offsetof(type,member) ); \
        })
    
#define c_unused(var)       (void)var;

#define __stringify_1(x...) #x
#define __stringify(x...)   __stringify_1(x)


/************************************************************************/
typedef pthread_mutex_t auto_lock_t;

static inline auto_lock_t * auto_lock_new() {
    auto_lock_t * lock = NULL;
    pthread_mutexattr_t  attr;

    if((lock = (auto_lock_t *)malloc(sizeof(auto_lock_t))) == NULL) {
        return NULL;
    }

    if(pthread_mutexattr_init(&attr) != 0) {
        goto exception;
    }

    if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        goto exception;
    }

    if(pthread_mutex_init(lock, &attr) != 0) {
        goto exception;
    }
    return lock;

    exception : {
        free(lock);
    }
    return NULL;
}

static inline void auto_lock_free(auto_lock_t * lock) {
    if(lock == NULL) {
        return;
    }

    pthread_mutex_destroy(lock);
    free(lock);
}

static inline void __auto_unlock(auto_lock_t ** lock) {
    pthread_mutex_unlock(*lock);
}

#define concate_name_detail(x, y)   x##y
#define concate_unique_name(x, y)   concate_name_detail(x, y)
#define make_unique(x)              concate_unique_name(x, __COUNTER__)

#define auto_lock(lock)             \
        pthread_mutex_lock(lock);   \
        auto_lock_t * make_unique(auto_lock) __attribute__((__cleanup__(__auto_unlock), __unused__)) = lock;


/************************************************************************/
struct list_node {
    struct list_node *prev;
    struct list_node *next;
};

static inline void list_initialize(struct list_node *list)
{
    list->prev = list->next = list;
}

static inline void list_delete(struct list_node *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
    item->prev = item->next = 0;
}

static inline void list_add_tail(struct list_node *list, struct list_node *item)
{
    item->prev = list->prev;
    item->next = list;
    list->prev->next = item;
    list->prev = item;
}

#define list_for_every_safe(list, node, temp_node) \
        for(node = (list)->next, temp_node = (node)->next; node != (list); node = temp_node, temp_node = (node)->next)

#define list_for_every_entry_safe(list, entry, temp_entry, type, member) \
        for(entry = containerof((list)->next, type, member), temp_entry = containerof((entry)->member.next, type, member); \
          &(entry)->member != (list); entry = temp_entry, temp_entry = containerof((temp_entry)->member.next, type, member))


/************************************************************************/
typedef struct _c_slot {
    struct list_node lnode;
    void * function;
} c_slot_t;

typedef struct _c_signal_slot {
    struct list_node slots;
    auto_lock_t    *  lock;
} signal_slot_t;

#define c_signal(_signal_name)   signal_slot_t _signal_name

#define c_signal_initialize(_object, _signal_name) do{                                                  \
            list_initialize(&((_object)->_signal_name.slots));                                          \
            (_object)->_signal_name.lock = auto_lock_new();                                             \
        }while(0);

#define c_signal_uninitialize(_object, _signal_name) do{                                                \
            c_signal_disconnect_all(_object, _signal_name);                                             \
            auto_lock_free((_object)->_signal_name.lock);                                               \
        }while(0);
        
#define c_signal_connected(_object, _signal_name, _slot) ({                                             \
            bool connected = false;                                                                     \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            auto_lock((_object)->_signal_name.lock);                                                    \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                if(slot->function == _slot) {                                                           \
                    connected = true;                                                                   \
                }                                                                                       \
            }                                                                                           \
            connected;                                                                                  \
        })

#define c_signal_connector(_object, _signal_name, _slot)                                                \
        while(c_signal_connected(_object, _signal_name, _slot) == false) {                              \
            c_slot_t * slot = NULL;                                                                     \
            auto_lock((_object)->_signal_name.lock);                                                    \
            if((slot = malloc(sizeof(c_slot_t))) == NULL) {                                             \
                /* Error occured : Maybe is out of memory */                                            \
            } else {                                                                                    \
                slot->function = _slot;                                                                 \
                list_initialize(&(slot->lnode));                                                        \
                list_add_tail(&((_object)->_signal_name.slots), &(slot->lnode));                        \
                break;                                                                                  \
            }                                                                                           \
        }


#define c_signal_disconnect(_object, _signal_name, _slot) do{                                           \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            auto_lock((_object)->_signal_name.lock);                                                    \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                if(slot->function == _slot) {                                                           \
                    list_delete(&(slot->lnode));                                                        \
                    free(slot);                                                                         \
                }                                                                                       \
            }                                                                                           \
        }while(0);

#define c_signal_disconnect_all(_object, _signal_name) do{                                              \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            auto_lock((_object)->_signal_name.lock);                                                    \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                    list_delete(&(slot->lnode));                                                        \
                    free(slot);                                                                         \
            }                                                                                           \
        }while(0);

#define c_signal_emit(_object, _signal_name, _signal_type, ...) do{                                     \
            c_slot_t *slot = NULL, *next = NULL;                                                        \
            auto_lock((_object)->_signal_name.lock);                                                    \
            list_for_every_entry_safe(&((_object)->_signal_name.slots), slot, next, c_slot_t, lnode) {  \
                if(slot->function != NULL) {                                                            \
                    ((_signal_type)(slot->function))(__VA_ARGS__);                                      \
                } else {                                                                                \
                    /* auto clear invalid slot */                                                       \
                    list_delete(&(slot->lnode)) ;                                                       \
                    free(slot);                                                                         \
                }                                                                                       \
            }                                                                                           \
        }while(0);

#endif
