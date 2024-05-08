#if !defined(CCML_IMPL)
#define CCML_IMPL

#define CCML_SRCS_MAX 2
#define CCML_TYPE_MAX 3
#define CCML_DIMS_MAX 4

#define CCML_CHAR_MAX 100
#define CCML_NODE_MAX 256

#define __STDC_WANT_IEC_60559_TYPES_EXT__
#include <float.h>

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off

//
//  ████████╗███████╗███╗   ██╗███████╗ ██████╗ ██████╗
//  ╚══██╔══╝██╔════╝████╗  ██║██╔════╝██╔═══██╗██╔══██╗
//     ██║   █████╗  ██╔██╗ ██║███████╗██║   ██║██████╔╝
//     ██║   ██╔══╝  ██║╚██╗██║╚════██║██║   ██║██╔══██╗
//     ██║   ███████╗██║ ╚████║███████║╚██████╔╝██║  ██║
//     ╚═╝   ╚══════╝╚═╝  ╚═══╝╚══════╝ ╚═════╝ ╚═╝  ╚═╝
//

// clang-format on

typedef enum ccml_type {
    CCML_TYPE_FP16,
    CCML_TYPE_FP32,
    CCML_TYPE_FP64
} ccml_type;

const static int ccml_type_sizes[CCML_TYPE_MAX] = {
#if defined(FLT16_MAX)
    [CCML_TYPE_FP16] = sizeof(_Float16),
#else
    [CCML_TYPE_FP16] = sizeof(float),
#endif
    [CCML_TYPE_FP32] = sizeof(float),
    [CCML_TYPE_FP64] = sizeof(double)
};

typedef enum ccml_buff {
    CCML_BUFF_NONE,
    CCML_BUFF_INTR,
    CCML_BUFF_LOAD,
    CCML_BUFF_SAVE
} ccml_buff;

typedef enum ccml_oper {
    CCML_OPER_NONE,
    CCML_OPER_LOG,
    CCML_OPER_EXP,
    CCML_OPER_SIN,
    CCML_OPER_REC,
    CCML_OPER_SQRT,
    CCML_OPER_ADD,
    CCML_OPER_MUL,
    CCML_OPER_SUM,
    CCML_OPER_RESHAPE,
    CCML_OPER_PERMUTE
} ccml_oper;

typedef struct ccml_tensor ccml_tensor;

typedef struct ccml_tensor {
    ccml_type type;
    ccml_oper oper;

    ccml_buff buff;

    int shape[CCML_DIMS_MAX];
    int stride[CCML_DIMS_MAX];

    ccml_tensor * src[CCML_SRCS_MAX];
    ccml_tensor * grad;

    bool has_grad;
    int index;
    void * data;
} ccml_tensor;

//
//  ██╗███╗   ██╗██╗████████╗
//  ██║████╗  ██║██║╚══██╔══╝
//  ██║██╔██╗ ██║██║   ██║
//  ██║██║╚██╗██║██║   ██║
//  ██║██║ ╚████║██║   ██║
//  ╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝
//
//  ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
//  ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//  █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
//  ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
//  ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
//  ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
//

static ccml_tensor * ccml_new_tensor_impl(ccml_type type, int shape[CCML_DIMS_MAX]) {
    ccml_tensor * result = malloc(sizeof(ccml_tensor));

    *result = (ccml_tensor){
       /*.type       =*/ type,
       /*.oper       =*/ CCML_OPER_NONE,
       /*.buff       =*/ CCML_BUFF_NONE,
       /*.shape      =*/ {shape[0], shape[1], shape[2], shape[3]},
       /*.stride     =*/ {shape[1] * shape[2] * shape[3],
                          shape[2] * shape[3], shape[3], 1},
       /*.src        =*/ {NULL},
       /*.grad       =*/ NULL,
       /*.has_grad   =*/ false,
       /*.index      =*/ -1,
       /*.data       =*/ NULL,
    };

    return result;
}

static ccml_tensor * ccml_new_tensor_1d(ccml_type type, int ne0, bool has_grad) {
    int shape[CCML_DIMS_MAX] = {ne0, 1, 1, 1};

    ccml_tensor * result = ccml_new_tensor_impl(type, shape);
    result->buff = CCML_BUFF_LOAD;
    result->has_grad = has_grad;

    return result;
}

static ccml_tensor * ccml_new_tensor_2d(ccml_type type, int ne0, int ne1, bool has_grad) {
    int shape[CCML_DIMS_MAX] = {ne0, ne1, 1, 1};

    ccml_tensor * result = ccml_new_tensor_impl(type, shape);
    result->buff = CCML_BUFF_LOAD;
    result->has_grad = has_grad;

    return result;
}

static ccml_tensor * ccml_new_tensor_3d(ccml_type type, int ne0, int ne1, int ne2, bool has_grad) {
    int shape[CCML_DIMS_MAX] = {ne0, ne1, ne2, 1};

    ccml_tensor * result = ccml_new_tensor_impl(type, shape);
    result->buff = CCML_BUFF_LOAD;
    result->has_grad = has_grad;

    return result;
}

static ccml_tensor * ccml_new_tensor_4d(ccml_type type, int ne0, int ne1, int ne2, int ne3,
                                 bool has_grad) {
    int shape[CCML_DIMS_MAX] = {ne0, ne1, ne2, ne3};

    ccml_tensor * result = ccml_new_tensor_impl(type, shape);
    result->buff = CCML_BUFF_LOAD;
    result->has_grad = has_grad;

    return result;
}

static int ccml_size(ccml_tensor * tensor) {
    return tensor->shape[0] * tensor->shape[1] * tensor->shape[2] * tensor->shape[3];
}

static void ccml_tensor_set(ccml_tensor * tensor, float * data) {
    tensor->type = CCML_TYPE_FP32;
    tensor->buff = CCML_BUFF_INTR;

    int size = ccml_size(tensor);
    tensor->data = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        *((float*)tensor->data + i) = data[i];
    }
}

static void ccml_tensor_ones(ccml_tensor * tensor) {
    tensor->type = CCML_TYPE_FP32;
    tensor->buff = CCML_BUFF_INTR;

    int size = ccml_size(tensor);
    tensor->data = malloc(size * sizeof(float));
    for (int i = 0; i < size; i++) {
        *((float*)tensor->data + i) = 1.0f;
    }
}

//
//  ███╗   ███╗██╗███████╗ ██████╗
//  ████╗ ████║██║██╔════╝██╔════╝
//  ██╔████╔██║██║███████╗██║
//  ██║╚██╔╝██║██║╚════██║██║
//  ██║ ╚═╝ ██║██║███████║╚██████╗
//  ╚═╝     ╚═╝╚═╝╚══════╝ ╚═════╝
//
//  ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
//  ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//  █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
//  ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
//  ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
//  ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
//

static bool ccml_can_broadcast(ccml_tensor * lhs, ccml_tensor * rhs) {
    if (rhs == NULL || lhs == NULL)
        return true;
    for (int i = 0; i < CCML_DIMS_MAX; i++) {
        if (lhs->shape[i] != rhs->shape[i] && lhs->shape[i] != 1 && rhs->shape[i] != 1) {
            return false;
        }
    }

    return true;
}

static bool ccml_broadcasted(ccml_tensor * lhs, ccml_tensor * rhs) {
    return lhs->shape[0] != rhs->shape[0] || lhs->shape[1] != rhs->shape[1] ||
           lhs->shape[2] != rhs->shape[2] || lhs->shape[3] != rhs->shape[3];
}

static bool ccml_has_buffer(ccml_tensor * tensor) {
    return tensor->buff != CCML_BUFF_NONE;
}

static int ccml_n_dim(ccml_tensor * tensor) {
    int last_dim = 0;
    for (int i = 0; i < CCML_DIMS_MAX; i++) {
        if(tensor->shape[i] != 1) last_dim = i;
    }
    return last_dim == 0 ? 1 : last_dim + 1;
}

static bool ccml_is_vector(ccml_tensor * tensor) {
    return tensor->shape[1] == 1 && tensor->shape[2] == 1 &&
           tensor->shape[3] == 1;
}

static bool ccml_is_matrix(ccml_tensor * tensor) {
    return tensor->shape[0] != 1 && tensor->shape[1] != 1 &&
           tensor->shape[2] == 1 && tensor->shape[3] == 1;
}

//
//   ██████╗ ██████╗ ██████╗ ███████╗
//  ██╔════╝██╔═══██╗██╔══██╗██╔════╝
//  ██║     ██║   ██║██████╔╝█████╗
//  ██║     ██║   ██║██╔══██╗██╔══╝
//  ╚██████╗╚██████╔╝██║  ██║███████╗
//   ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
//
//   ██████╗ ██████╗ ███████╗██████╗  █████╗ ████████╗██╗ ██████╗ ███╗   ██╗███████╗
//  ██╔═══██╗██╔══██╗██╔════╝██╔══██╗██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//  ██║   ██║██████╔╝█████╗  ██████╔╝███████║   ██║   ██║██║   ██║██╔██╗ ██║███████╗
//  ██║   ██║██╔═══╝ ██╔══╝  ██╔══██╗██╔══██║   ██║   ██║██║   ██║██║╚██╗██║╚════██║
//  ╚██████╔╝██║     ███████╗██║  ██║██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║███████║
//   ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
//

#define CCML_UNARY_OPERATION(function, operation)                                              \
static ccml_tensor * function(ccml_tensor * tensor) {                                          \
    ccml_tensor * result = ccml_new_tensor_impl(tensor->type, tensor->shape);                  \
                                                                                               \
    for (int i = 0; i < CCML_DIMS_MAX; i++) {                                                  \
        result->stride[i] = tensor->stride[i];                                                 \
    }                                                                                          \
                                                                                               \
    result->oper = operation;                                                                  \
    result->src[0] = tensor;                                                                   \
    result->has_grad = tensor->has_grad;                                                       \
                                                                                               \
    return result;                                                                             \
}

CCML_UNARY_OPERATION(ccml_log, CCML_OPER_LOG);
CCML_UNARY_OPERATION(ccml_exp, CCML_OPER_EXP);
CCML_UNARY_OPERATION(ccml_sin, CCML_OPER_SIN);
CCML_UNARY_OPERATION(ccml_rec, CCML_OPER_REC);
CCML_UNARY_OPERATION(ccml_sqrt, CCML_OPER_SQRT);

static ccml_tensor * ccml_add(ccml_tensor * lhs, ccml_tensor * rhs) {
    assert(ccml_can_broadcast(lhs, rhs));

    bool null_input = lhs == NULL || rhs == NULL;
    ccml_tensor * non_null = lhs != NULL ? lhs : rhs;

    int shape[CCML_DIMS_MAX] = {0};
    for (int i = 0; i < CCML_DIMS_MAX; i++) {
        shape[i] = null_input ? non_null->shape[i] :
            (lhs->shape[i] + rhs->shape[i] + abs(lhs->shape[i] - rhs->shape[i])) / 2;
    }

    ccml_tensor * result = ccml_new_tensor_impl(lhs->type, shape);

    result->oper = CCML_OPER_ADD;
    result->src[0] = null_input ? result : lhs;
    result->src[1] = null_input ? non_null : rhs;
    result->has_grad = null_input ? non_null->has_grad : lhs->has_grad || rhs->has_grad;

    return result;
}

static ccml_tensor * ccml_mul(ccml_tensor * lhs, ccml_tensor * rhs) {
    assert(ccml_can_broadcast(lhs, rhs));

    bool null_input = lhs == NULL || rhs == NULL;
    ccml_tensor * non_null = lhs != NULL ? lhs : rhs;

    int shape[CCML_DIMS_MAX] = {0};
    for (int i = 0; i < CCML_DIMS_MAX; i++) {
        shape[i] = null_input ? non_null->shape[i] :
            (lhs->shape[i] + rhs->shape[i] + abs(lhs->shape[i] - rhs->shape[i])) / 2;
    }

    ccml_tensor * result = ccml_new_tensor_impl(lhs->type, shape);

    result->oper = CCML_OPER_MUL;
    result->src[0] = null_input ? result : lhs;
    result->src[1] = null_input ? non_null : rhs;
    result->has_grad = null_input ? non_null->has_grad : lhs->has_grad || rhs->has_grad;

    return result;
}

static ccml_tensor * ccml_reshape(ccml_tensor * tensor, int shape[CCML_DIMS_MAX]) {
    int size = ccml_size(tensor);
    int new_size = shape[0] * shape[1] * shape[2] * shape[3];
    assert(size == new_size && "reshaped and source tensor must have the same size");

    ccml_tensor * result = ccml_new_tensor_impl(tensor->type, shape);

    result->oper = CCML_OPER_RESHAPE;
    result->buff = CCML_BUFF_INTR;
    result->src[0] = tensor;
    result->has_grad = tensor->has_grad;

    return result;
}

static ccml_tensor * ccml_permute(ccml_tensor * tensor, int perm[CCML_DIMS_MAX]) {
    ccml_tensor * result = ccml_new_tensor_impl(tensor->type, tensor->shape);
    int n_dim = ccml_n_dim(tensor);
    for (int i = 0; i < n_dim; i++) {
        result->shape[i] = tensor->shape[perm[i]];
        result->stride[i] = tensor->stride[perm[i]];
    }

    result->oper = CCML_OPER_PERMUTE;
    result->buff = CCML_BUFF_INTR;
    result->src[0] = tensor;
    result->has_grad = tensor->has_grad;

    return result;
}

static ccml_tensor * ccml_sum(ccml_tensor * tensor, int n_axes, int axes[CCML_DIMS_MAX]) {
    assert(n_axes > 0 && n_axes < CCML_DIMS_MAX);

    int shape[CCML_DIMS_MAX] = {1, 1, 1, 1};
    for (int i = 0; i < CCML_DIMS_MAX; i++) {
        shape[i] = tensor->shape[i];
    }

    for (int i = 0; i < n_axes; i++) {
        shape[axes[i]] = 1;
    }

    ccml_tensor * result = ccml_new_tensor_impl(tensor->type, shape);

    result->oper = CCML_OPER_SUM;
    result->buff = CCML_BUFF_INTR;
    result->src[0] = tensor;
    result->has_grad = tensor->has_grad;

    return result;
}

//
//  ███╗   ██╗ ██████╗ ███╗   ██╗       ██████╗ ██████╗ ██████╗ ███████╗
//  ████╗  ██║██╔═══██╗████╗  ██║      ██╔════╝██╔═══██╗██╔══██╗██╔════╝
//  ██╔██╗ ██║██║   ██║██╔██╗ ██║█████╗██║     ██║   ██║██████╔╝█████╗
//  ██║╚██╗██║██║   ██║██║╚██╗██║╚════╝██║     ██║   ██║██╔══██╗██╔══╝
//  ██║ ╚████║╚██████╔╝██║ ╚████║      ╚██████╗╚██████╔╝██║  ██║███████╗
//  ╚═╝  ╚═══╝ ╚═════╝ ╚═╝  ╚═══╝       ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝
//
//   ██████╗ ██████╗ ███████╗██████╗  █████╗ ████████╗██╗ ██████╗ ███╗   ██╗███████╗
//  ██╔═══██╗██╔══██╗██╔════╝██╔══██╗██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
//  ██║   ██║██████╔╝█████╗  ██████╔╝███████║   ██║   ██║██║   ██║██╔██╗ ██║███████╗
//  ██║   ██║██╔═══╝ ██╔══╝  ██╔══██╗██╔══██║   ██║   ██║██║   ██║██║╚██╗██║╚════██║
//  ╚██████╔╝██║     ███████╗██║  ██║██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║███████║
//   ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝
//

static ccml_tensor * ccml_neg(ccml_tensor * tensor) {
    return ccml_log(ccml_rec(ccml_exp(tensor)));
}

static ccml_tensor * ccml_square(ccml_tensor * tensor) {
    return ccml_mul(tensor, tensor);
}

static ccml_tensor * ccml_sub(ccml_tensor * lhs, ccml_tensor * rhs) {
    return ccml_add(lhs, ccml_neg(rhs));
}

static ccml_tensor * ccml_div(ccml_tensor * lhs, ccml_tensor * rhs) {
    return ccml_mul(lhs, ccml_rec(rhs));
}

static ccml_tensor * ccml_cos(ccml_tensor * tensor) {
    ccml_tensor * pi_2 = ccml_new_tensor_1d(tensor->type, 1, false);
    ccml_tensor_set(pi_2, (float[]){M_PI_2});

    return ccml_sin(ccml_add(tensor, pi_2));
}

static ccml_tensor * ccml_tanh(ccml_tensor * tensor) {
    return ccml_div(ccml_sub(ccml_exp(tensor), ccml_exp(ccml_neg(tensor))),
                    ccml_add(ccml_exp(tensor), ccml_exp(ccml_neg(tensor))));
}

static ccml_tensor * ccml_matmul(ccml_tensor * lhs, ccml_tensor * rhs) {
    assert(ccml_is_matrix(lhs));
    assert(ccml_is_matrix(rhs));

    assert(lhs->shape[1] == rhs->shape[0]);

    ccml_tensor * lhs_r = ccml_reshape(lhs, (int[]){lhs->shape[0], lhs->shape[1], 1, 1});
    ccml_tensor * rhs_r = ccml_reshape(rhs, (int[]){1, rhs->shape[0], rhs->shape[1], 1});

    ccml_tensor * mul = ccml_mul(lhs_r, rhs_r);
    ccml_tensor * sum = ccml_sum(mul, 1, (int[]){1});

    return sum;
}

//
//  ██╗  ██╗ █████╗ ███████╗██╗  ██╗███╗   ███╗ █████╗ ██████╗
//  ██║  ██║██╔══██╗██╔════╝██║  ██║████╗ ████║██╔══██╗██╔══██╗
//  ███████║███████║███████╗███████║██╔████╔██║███████║██████╔╝
//  ██╔══██║██╔══██║╚════██║██╔══██║██║╚██╔╝██║██╔══██║██╔═══╝
//  ██║  ██║██║  ██║███████║██║  ██║██║ ╚═╝ ██║██║  ██║██║
//  ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝╚═╝
//

#define CCML_FNV_PRIME 1099511628211LU
#define CCML_FNV_OFFSET 14695981039346656037LU

typedef struct ccml_hashmap_entry {
    uintptr_t key;
    int value;
} ccml_hashmap_entry;

typedef struct ccml_hashmap {
    int used;
    ccml_hashmap_entry * entries;
    int capacity;
} ccml_hashmap;

static uint64_t ccml_hash_key(void * key) {
    uint64_t hash = CCML_FNV_OFFSET;
    hash ^= (uint64_t)(uintptr_t)key;
    hash *= CCML_FNV_PRIME;
    return hash;
}

static ccml_hashmap * ccml_new_hashmap() {
    int capacity = CCML_NODE_MAX;

    ccml_hashmap * map = malloc(sizeof(ccml_hashmap));
    *map = (ccml_hashmap){
        .used = 0,
        .entries = malloc(sizeof(ccml_hashmap_entry) * capacity),
        .capacity = capacity,
    };

    for (int i = 0; i < capacity; i++) {
        map->entries[i].key = 0;
        map->entries[i].value = -1;
    }

    return map;
}

static int ccml_hashmap_get(ccml_hashmap * map, void * key) {
    if (key == NULL) {
        return -1;
    }

    uint64_t hash = ccml_hash_key(key);
    int index = (int)(hash & (uint64_t)(map->capacity - 1));

    while (map->entries[index].key != 0) {
        if ((uintptr_t)key == map->entries[index].key) {
            return map->entries[index].value;
        }

        index++;
        if (index >= map->capacity) {
            index = 0;
        }
    }

    return -1;
};

static void ccml_hashmap_set(ccml_hashmap * map, void * key, int value) {
    if (map->used >= map->capacity) {
        assert(false && "hashmap size overflow");
    }

    uint64_t hash = ccml_hash_key(key);
    int index = (int)(hash & (int)(map->capacity - 1));

    while (map->entries[index].key != 0) {
        if ((uintptr_t)key == map->entries[index].key) {
            // Found key (it already exists), update value.
            map->entries[index].value = value;
            return;
        }
        // Key wasn't in this slot, move to next (linear
        // probing).
        index++;
        if (index >= map->capacity) {
            index = 0;
        }
    }

    map->entries[index].key = (uintptr_t)key;
    map->entries[index].value = value;
    map->used++;
}

//
//   ██████╗ ██████╗  █████╗ ██████╗ ██╗  ██╗
//  ██╔════╝ ██╔══██╗██╔══██╗██╔══██╗██║  ██║
//  ██║  ███╗██████╔╝███████║██████╔╝███████║
//  ██║   ██║██╔══██╗██╔══██║██╔═══╝ ██╔══██║
//  ╚██████╔╝██║  ██║██║  ██║██║     ██║  ██║
//   ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝
//

typedef struct ccml_graph {
    int n_nodes;
    ccml_tensor * nodes[CCML_NODE_MAX];
    ccml_hashmap * map;
} ccml_graph;

static void ccml_graph_forward(struct ccml_graph * graph, ccml_tensor * tensor,
                               int * node_counter) {
    if (tensor == NULL) return;

    // also checking if tensor has itself as a child to prevent (infinite)
    // cycles

    if (tensor != tensor->src[0] && ccml_hashmap_get(graph->map, tensor->src[0]) == -1) {
        ccml_graph_forward(graph, tensor->src[0], node_counter);
    }
    if (tensor != tensor->src[1] && ccml_hashmap_get(graph->map, tensor->src[1]) == -1) {
        ccml_graph_forward(graph, tensor->src[1], node_counter);
    }

    if (ccml_hashmap_get(graph->map, tensor) == -1) {
        tensor->index = *node_counter;
        graph->nodes[*node_counter] = tensor;
        ccml_hashmap_set(graph->map, tensor, (*node_counter)++);
    }
}

static void ccml_graph_backward(ccml_graph * graph, ccml_tensor * root) {
    if (root->has_grad == false) return;

    // in this loop create gradient tensors corresponding to each tensor
    // that requires gradient tracking, and set their buffers to the correct
    // option respectively (because, for example, a intermediary tensor w/o
    // a buffer also needs an intermediary gradient tensor w/o a buffer)

    ccml_tensor * queue[CCML_NODE_MAX] = {NULL};
    int queue_start = 0;
    int queue_end = 0;
    queue[queue_end++] = root;

    while (queue_end != queue_start) {
        ccml_tensor * tensor = queue[queue_start++];
        if (tensor->has_grad == true) {
            // processing node here

            // declaring partials d(tensor)/d(tensor->src[0]) and
            // d(tensor)/d(tensor->src[1])

            ccml_tensor * partial_0 = NULL;
            ccml_tensor * partial_1 = NULL;

            ccml_tensor * one = ccml_new_tensor_1d(tensor->type, 1, false);
            ccml_tensor * two = ccml_new_tensor_1d(tensor->type, 1, false);

            ccml_tensor_set(one, (float[]){1.0f});
            ccml_tensor_set(two, (float[]){2.0f});

            // calculating partials

            switch (tensor->oper) {
                case CCML_OPER_NONE:
                    break;
                case CCML_OPER_LOG:
                    partial_0 = ccml_rec(tensor->src[0]); break;
                case CCML_OPER_EXP:
                    partial_0 = ccml_exp(tensor->src[0]); break;
                case CCML_OPER_SIN:
                    partial_0 = ccml_cos(tensor->src[0]); break;
                case CCML_OPER_REC:
                    partial_0 = ccml_neg(ccml_rec(ccml_square(tensor->src[0]))); break;
                case CCML_OPER_SQRT:
                    partial_0 = ccml_rec(ccml_mul(two, ccml_sqrt(tensor->src[0]))); break;
                case CCML_OPER_ADD:
                    partial_0 = one; partial_1 = one; break;
                case CCML_OPER_MUL:
                    partial_0 = tensor->src[1]; partial_1 = tensor->src[0]; break;
                case CCML_OPER_RESHAPE:
                    partial_0 = one; break;
                case CCML_OPER_PERMUTE:
                    partial_0 = one; break;
                case CCML_OPER_SUM:
                    partial_0 = one; break;
                default:
                    assert(false && "unknown variant of ccml_oper");
            }

            // multiplying tensor->grad by partials and adding them to
            // the gradients of the tensor's children (we have to do a
            // mini DFS traversal w/ ccml_graph_forward() since the gradient
            // calculation forms a mini sub-graph that needs to be tra-
            // versed separately)

            if (tensor->src[0] != NULL) {
                tensor->src[0]->grad = ccml_add(ccml_mul(tensor->grad, partial_0), NULL);
                ccml_graph_forward(graph, tensor->src[0]->grad, &graph->n_nodes);
                queue[queue_end++] = tensor->src[0];
            }
            if (tensor->src[1] != NULL) {
                tensor->src[1]->grad = ccml_add(ccml_mul(tensor->grad, partial_1), NULL);
                ccml_graph_forward(graph, tensor->src[1]->grad, &graph->n_nodes);
                queue[queue_end++] = tensor->src[1];
            }
        }
    }
}

static void ccml_graph_node_buffers(struct ccml_graph * graph) {
    for (int i = 0; i < graph->n_nodes; i++) {
        ccml_tensor * tensor = graph->nodes[i];
        int size = ccml_size(tensor);

        if (ccml_has_buffer(tensor) && tensor->data == NULL) {
            tensor->data = malloc(size * ccml_type_sizes[tensor->type]);
            if (tensor->grad != NULL && tensor->grad->data == NULL) {
                tensor->grad->buff = CCML_BUFF_SAVE;
                tensor->grad->data = malloc(size * ccml_type_sizes[tensor->type]);
            }
        }
    }
}

// clang-format off

static ccml_graph * ccml_new_graph(ccml_tensor * root) {
    root->data = malloc(ccml_size(root) * sizeof(ccml_type_sizes[root->type]));
    root->buff = CCML_BUFF_SAVE;

    if (root->has_grad == true) {
        ccml_tensor * grad = ccml_new_tensor_1d(root->type, 1, false);
        ccml_tensor_ones(grad);
        root->grad = grad;
    }

    struct ccml_graph * graph = malloc(sizeof(struct ccml_graph));

    *graph = (struct ccml_graph){
        /*.n_nodes =*/ 0,
        /*.nodes   =*/ {NULL},
        /*.map     =*/ ccml_new_hashmap()
    };

    ccml_graph_forward(graph, root, &graph->n_nodes);
    ccml_graph_backward(graph, root);

    ccml_graph_node_buffers(graph);

    return graph;
};

// clang-format on

static void ccml_graph_free(ccml_graph * graph) {
    for (int i = 0; i < graph->n_nodes; i++) {
        ccml_tensor * tensor = graph->nodes[i];

        // only freeing when tensor isn't of type reshape/permute, because those tensors
        // just use their children's data pointer, so we avoid a double free this way :)
        if (tensor->oper != CCML_OPER_RESHAPE && tensor->oper != CCML_OPER_PERMUTE) {
            free(tensor->data);
        }

        free(tensor);
    }

    free(graph);
}

//
//  ██╗███╗   ██╗██████╗ ██╗ ██████╗███████╗███████╗
//  ██║████╗  ██║██╔══██╗██║██╔════╝██╔════╝██╔════╝
//  ██║██╔██╗ ██║██║  ██║██║██║     █████╗  ███████╗
//  ██║██║╚██╗██║██║  ██║██║██║     ██╔══╝  ╚════██║
//  ██║██║ ╚████║██████╔╝██║╚██████╗███████╗███████║
//  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝ ╚═════╝╚══════╝╚══════╝
//

// both reduction and broadcasting index functions look terrible, there has to be a cleaner
// and concise way to express this

#define ccml_index(tensor, condition) ({                                         \
    int size = CCML_CHAR_MAX * CCML_DIMS_MAX;                                    \
    char * result = malloc(size * sizeof(char));                                 \
    *result = '\0';                                                              \
    strncat(result, "[", size);                                                  \
                                                                                 \
    for (int i = 0; i < ccml_n_dim(tensor); i++) {                               \
        snprintf(result + strlen(result), size - strlen(result), "%sid%d*%d*%d", \
                 i != 0 && i != ccml_n_dim(tensor) ? "+" : "", i,                \
                 tensor->stride[i], (condition) ? 1 : 0);                        \
    }                                                                            \
                                                                                 \
    strncat(result + strlen(result), "]", size - strlen(result));                \
    if (ccml_has_buffer(tensor) == false) snprintf(result, size, "");            \
                                                                                 \
    result;                                                                      \
})

//
//  ██████╗  █████╗  ██████╗██╗  ██╗███████╗███╗   ██╗██████╗
//  ██╔══██╗██╔══██╗██╔════╝██║ ██╔╝██╔════╝████╗  ██║██╔══██╗
//  ██████╔╝███████║██║     █████╔╝ █████╗  ██╔██╗ ██║██║  ██║
//  ██╔══██╗██╔══██║██║     ██╔═██╗ ██╔══╝  ██║╚██╗██║██║  ██║
//  ██████╔╝██║  ██║╚██████╗██║  ██╗███████╗██║ ╚████║██████╔╝
//  ╚═════╝ ╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═══╝╚═════╝
//
//   ██████╗██╗   ██╗██████╗  █████╗
//  ██╔════╝██║   ██║██╔══██╗██╔══██╗
//  ██║     ██║   ██║██║  ██║███████║
//  ██║     ██║   ██║██║  ██║██╔══██║
//  ╚██████╗╚██████╔╝██████╔╝██║  ██║
//   ╚═════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝
//

static const char * ccml_oper_to_string_cuda(ccml_oper oper) {
    switch (oper) {
        case CCML_OPER_LOG:
            return "log";
        case CCML_OPER_EXP:
            return "exp";
        case CCML_OPER_SIN:
            return "sin";
        case CCML_OPER_REC:
            return "1/";
        case CCML_OPER_SQRT:
            return "sqrt";
        case CCML_OPER_ADD:
            return "+";
        case CCML_OPER_MUL:
            return "*";
        default:
            assert(false && "no meaningful conversion to string exists");
    }
}

static const char * ccml_type_to_string_cuda(ccml_type type) {
    switch (type) {
        case CCML_TYPE_FP16:
            return "__half ";
        case CCML_TYPE_FP32:
            return "float ";
        case CCML_TYPE_FP64:
            return "double ";
        default:
            assert(false && "unknown variant of ccml_type");
    }
}

static const char * ccml_parser_cuda(struct ccml_graph * graph) {
    int size = CCML_NODE_MAX * CCML_CHAR_MAX;
    char * kernel = malloc(size * sizeof(char));
    char * string = kernel;
    *string = '\0';

    // adding includes and kernel function signature to the
    // kernel string

    string += snprintf(string, size - (kernel - string), "#include <cuda_fp16.h>\n"
                       "\n__global__ void ccml_kernel(");

    // adding kernel input parameters to the kernel string

    int n_kernel_parameters = 0;
    int largest_tensor = 0;

    for (int i = 0; i < graph->n_nodes; i++) {
        ccml_tensor * tensor = graph->nodes[i];
        if (ccml_size(tensor) > ccml_size(graph->nodes[largest_tensor]))
            largest_tensor = i;

        if (tensor->buff != CCML_BUFF_NONE && ccml_size(tensor) != 1) {
            if (n_kernel_parameters == 0) {
                string += snprintf(string, size - (kernel - string), "%s* data_%d",
                                   ccml_type_to_string_cuda(tensor->type), i);
                n_kernel_parameters++;
            } else {
                string += snprintf(string, size - (kernel - string), ", %s* data_%d",
                                   ccml_type_to_string_cuda(tensor->type), i);
                n_kernel_parameters++;
            }
        }
    }

    string += snprintf(string, size - (kernel - string), ") {\n"
                       "\tint id0 = blockIdx.x * blockDim.x + threadIdx.x; /* %d */\n"
                       "\tint id1 = blockIdx.y * blockDim.y + threadIdx.y; /* %d */\n"
                       "\tint id2 = blockIdx.z * blockDim.z + threadIdx.z; /* %d */\n"
                       "\tfor (int id3 = 0; id3 < %d; id3++) {\n",
                       graph->nodes[largest_tensor]->shape[0],
                       graph->nodes[largest_tensor]->shape[1],
                       graph->nodes[largest_tensor]->shape[2],
                       graph->nodes[largest_tensor]->shape[3]);

    for (int i = 0; i < graph->n_nodes; i++) {
        ccml_tensor * tensor = graph->nodes[i];

        switch (tensor->oper) {
            case CCML_OPER_NONE:
                // tensor data is embeddeable directly into the kernel string
                if (ccml_size(tensor) == 1) {
                    string += snprintf(string, size - (kernel - string), "\t\t%sdata_%d = %f;\n",
                                       ccml_type_to_string_cuda(tensor->type), i, *(float *)tensor->data);
                }
                break;
            case CCML_OPER_LOG:
            case CCML_OPER_EXP:
            case CCML_OPER_SIN:
            case CCML_OPER_REC:
            case CCML_OPER_SQRT:
                string += snprintf(string, size - (kernel - string), "\t\t%sdata_%d%s = ",
                                   ccml_has_buffer(tensor) ? "" : ccml_type_to_string_cuda(tensor->type),
                                   tensor->index, ccml_index(tensor, true));
                string += snprintf(string, size - (kernel - string), "%s(data_%d%s);\n",
                                   ccml_oper_to_string_cuda(tensor->oper),
                                   tensor->src[0]->index, ccml_index(tensor->src[0], true));
                break;
            case CCML_OPER_ADD:
            case CCML_OPER_MUL:
                string += snprintf(string, size - (kernel - string), "\t\t%sdata_%d%s = ",
                                   ccml_has_buffer(tensor) ? "" : ccml_type_to_string_cuda(tensor->type),
                                   tensor->index, ccml_index(tensor, true));
                string += snprintf(string, size - (kernel - string), "data_%d%s %s ",
                                   tensor->src[0]->index, ccml_index(tensor->src[0], tensor->src[0]->shape[i] != 1),
                                   ccml_oper_to_string_cuda(tensor->oper));
                string += snprintf(string, size - (kernel - string), "data_%d%s;\n",
                                   tensor->src[1]->index, ccml_index(tensor->src[1], tensor->src[1]->shape[i] != 1));
                break;
            case CCML_OPER_RESHAPE:
            case CCML_OPER_PERMUTE:
                string += snprintf(string, size - (kernel - string), "\t\t%s* data_%d = ",
                                   ccml_type_to_string_cuda(tensor->type), tensor->index);
                string += snprintf(string, size - (kernel - string), "data_%d;\n",
                                   tensor->src[0]->index);
                break;
            case CCML_OPER_SUM:
                string += snprintf(string, size - (kernel - string), "\t\tdata_%d%s += ",
                                   tensor->index, ccml_index(tensor, tensor->shape[i] != 1));
                string += snprintf(string, size - (kernel - string), "data_%d%s;\n",
                                   tensor->src[0]->index, ccml_index(tensor->src[0], tensor->shape[i] != tensor->src[0]->shape[i]));
                break;
            default: assert(false && "unknown variant of ccml_oper");
        }
    }

    // adding the closing braces/brackets :)
    string += snprintf(string, size - (kernel - string), "\t}\n}");
    return kernel;
}

#endif