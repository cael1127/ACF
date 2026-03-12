#include "core/tensor.h"

#include <stdlib.h>

static size_t tensor_compute_numel(int ndim, const size_t *shape) {
    size_t n = 1;
    for (int i = 0; i < ndim; ++i) {
        n *= shape[i];
    }
    return n;
}

Tensor tensor_create(int ndim, const size_t *shape, TensorDType dtype) {
    Tensor t;
    t.ndim = ndim;
    t.dtype = dtype;

    t.shape = (size_t *)malloc(sizeof(size_t) * (size_t)ndim);
    t.stride = (size_t *)malloc(sizeof(size_t) * (size_t)ndim);

    size_t stride = 1;
    for (int i = ndim - 1; i >= 0; --i) {
        t.shape[i] = shape[i];
        t.stride[i] = stride;
        stride *= shape[i];
    }

    size_t n = tensor_compute_numel(ndim, shape);
    size_t elem_size = (dtype == TENSOR_F32) ? sizeof(float) : sizeof(double);
    t.data = malloc(n * elem_size);

    return t;
}

void tensor_free(Tensor *t) {
    if (!t) return;
    free(t->data);
    free(t->shape);
    free(t->stride);
    t->data = NULL;
    t->shape = NULL;
    t->stride = NULL;
    t->ndim = 0;
}

size_t tensor_numel(const Tensor *t) {
    return t ? tensor_compute_numel(t->ndim, t->shape) : 0;
}

