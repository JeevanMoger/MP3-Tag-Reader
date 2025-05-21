#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint;

typedef enum{
    success,
    failure
}Status;

typedef enum
{
    view,
    edit,
    help,
    unsupported
}OperationType;
 
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"

#endif