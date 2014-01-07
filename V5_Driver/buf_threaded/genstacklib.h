/*
* @author:  Stefano Di Martino
* @created: 05.04.2012
* indent options: -kr -i8 -ts8 -l80 --no-tabs
*/

#ifndef GENSTACKLIB_H
#define GENSTACKLIB_H

typedef struct {
    void *elems;                /* Points to the objects on the stack */
    size_t elemSize;            /* Element size of the element type */
    int currSize;              /* The real amount of objects on the stack */
    struct mutex mutex;
    void (*freefn) (void *);    /* free function for more complex data types */
} genStack;

/**
* Allocates the necessary memory to store 4 data objects 
* with size of elemSize.
* CAUTION: Client is responsible for disposing the stack after use.
* If you call GenStackNew() sereval times without disposing, then you
* cause memory leaks.
*
* @param *s 
*	stack to be created.
* @param elemSize
*       the element size to be pushed in
* @param void (*freefn)(void*)
*       Function pointer for objects to be freed by the client.
*       Set to NULL if you don't need to free your objects.
*/
void GenStackNew(genStack * s, size_t elemSize, void (*freefn) (void *));

/**
* This function is not thread safe! Use GenStackTryPop() instead!
* @return
*       0 if empty, else != 0
*/
bool GenStackEmpty(const genStack * s);


/**
* Pushes the data object onto the stack.
*
* @param *s 
*	the stack
* @param *elemAddr
*	address of the value to be pushed
* @return
*	-1 when stack is full, else 0.
*/
int GenStackPush(genStack * s, const void *elemAddr);

/**
* @param *s
*	the stack
* @param *elemAddr
*	Address where the poped value has to be stored
*/
void GenStackPop(genStack * s, void *elemAddr);


int GenStackFull(const genStack *s);

/**
* This functions returns immediatelly, if stack is empty.
* @param *s
*	the stack
* @param *elemAddr
*	Address where the poped value has to be stored
* @return
*       0, if pop was successful, -1 else.
*/
int GenStackTryPop(genStack * s, void *elemAddr);

/**
* Frees the allocated memory and sets s->elems to NULL
*/
void GenStackDispose(genStack * s);

#endif
