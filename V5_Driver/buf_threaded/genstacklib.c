/*
* @author:  Stefano Di Martino
* @created: 05.04.2012
* indent options: -kr -i8 -ts8 -l80 --no-tabs
*/

#include <linux/string.h>
#include <linux/slab.h> // kmalloc(), kfree()

#include "genstacklib.h"

#define stackTotalSize 4

#define check_memory(pointer) if (pointer == NULL) {\
         pr_alert("Could not allocate memory!\n");\
         return -1;\
         }


void GenStackNew(genStack * s, size_t elemSize, void (*freefn) (void *))
{
    /* Default initialization */
    s->elems = kmalloc(stackTotalSize, elemSize);
    check_memory(s->elems);

    mutex_init(&s->mutex);
    s->elemSize = elemSize;
    s->currSize = 0;
    s->freefn = freefn;
}

int GenStackPush(genStack * s, const void *elemAddr)
{
    char *pTargetAddr;

    mutex_lock(&s->mutex);

    if (s->currSize == stackTotalSize) {
	mutex_unlock(&s->mutex);
        return -1;
    }

    /* Equivalent to &s->elems[s->currSize] */
    pTargetAddr = (char *) s->elems + s->currSize * s->elemSize;

    memcpy(pTargetAddr, elemAddr, s->elemSize);
    s->currSize++;

    mutex_unlock(&s->mutex);
    
    return 0;
}

int GenStackTryPop(genStack * s, void *elemAddr)
{
    char *pSourceAddr;

    mutex_lock(&s->mutex);

    int isEmpty = s->currSize == 0;

    if (isEmpty) {
        mutex_unlock(&s->mutex);
        return -1;
    }

    /* Equivalent to &s->elems[s->currSize - 1] */
    pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

    memcpy(elemAddr, pSourceAddr, s->elemSize);
    s->currSize--;

    mutex_unlock(&s->mutex);

    return 0;
}

void GenStackPop(genStack * s, void *elemAddr)
{
    char *pSourceAddr;

    mutex_lock(&s->mutex);

    /* Equivalent to &s->elems[s->currSize - 1] */
    pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

    memcpy(elemAddr, pSourceAddr, s->elemSize);
    s->currSize--;

    mutex_unlock(&s->mutex);
}

int GenStackEmpty(const genStack *s)
{
    return s->currSize == 0;
}

int GenStackFull(const genStack *s)
{
    return s->currSize == stackTotalSize;
}

void GenStackDispose(genStack * s)
{
    if (s->freefn != NULL && s->currSize > 0) {
        char *pSourceAddr;

        for (; s->currSize > 0; s->currSize--) {
            /* Equivalent to &s->elems[s->currSize - 1] */
            pSourceAddr = (char *) s->elems + (s->currSize - 1) * s->elemSize;

            /* call free function of the client */
            s->freefn(pSourceAddr);
        }
    }

    mutex_destroy(&s->mutex);
    kfree(s->elems);
    s->elems = NULL;
}
