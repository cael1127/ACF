#ifndef CORE_GPU_H
#define CORE_GPU_H

/* GPU integration hook.
   For now only exposes a probe; future kernels such as a GPU-backed
   FlashAttention implementation can be registered alongside this. */

int gpu_is_available(void);

#endif /* CORE_GPU_H */

