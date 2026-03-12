#ifndef CORE_TENSOR_H
#define CORE_TENSOR_H

#include <stddef.h>

typedef enum {
    TENSOR_F32 = 0,
    TENSOR_F64 = 1
} TensorDType;

typedef struct {
    void *data;
    int   ndim;
    size_t *shape;
    size_t *stride;
    TensorDType dtype;
} Tensor;

Tensor tensor_create(int ndim, const size_t *shape, TensorDType dtype);
void   tensor_free(Tensor *t);

size_t tensor_numel(const Tensor *t);

#endif /* CORE_TENSOR_H */

