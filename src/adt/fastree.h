#include "fast.h"

#define __fastree_type(t_name) Fastree_##t_name
#define __fastree_node(t_name) FastreeNode_##t_name

#define make_fastree_directives(t_name, t_key, t_compare)                                                                                      \
    typedef struct __fastree_node(t_name)                                                                                                      \
    {                                                                                                                                          \
        t_key value;                                                                                                                           \
        struct __fastree_node(t_name) * left;                                                                                                  \
        struct __fastree_node(t_name) * right;                                                                                                 \
        int height;                                                                                                                            \
    }                                                                                                                                          \
    __fastree_node(t_name);                                                                                                                    \
                                                                                                                                               \
    typedef struct                                                                                                                             \
    {                                                                                                                                          \
        int length;                                                                                                                            \
        __fastree_node(t_name) * head;                                                                                                         \
    } __fastree_type(t_name);                                                                                                                  \
                                                                                                                                               \
    inline static int __fastree_##t_name##_balance_factor(__fastree_node(t_name) * node)                                                       \
    {                                                                                                                                          \
        if (node == NULL)                                                                                                                      \
            return 0;                                                                                                                          \
        int a = node->left ? node->left->height : 0;                                                                                           \
        int b = node->right ? node->right->height : 0;                                                                                         \
        return a - b;                                                                                                                          \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static int __fastree_##t_name##_height(__fastree_node(t_name) * node)                                                               \
    {                                                                                                                                          \
        if (node == NULL)                                                                                                                      \
            return 0;                                                                                                                          \
        int a = node->left ? node->left->height : 0;                                                                                           \
        int b = node->right ? node->right->height : 0;                                                                                         \
        return 1 + (a > b ? a : b);                                                                                                            \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_rr(__fastree_node(t_name) * A)                                                 \
    {                                                                                                                                          \
        __fastree_node(t_name) *B = A->left;                                                                                                   \
                                                                                                                                               \
        __fastree_node(t_name) *Br = B->right;                                                                                                 \
        B->right = A;                                                                                                                          \
        A->left = Br;                                                                                                                          \
                                                                                                                                               \
        A->height = __fastree_##t_name##_height(A);                                                                                            \
        B->height = __fastree_##t_name##_height(B);                                                                                            \
                                                                                                                                               \
        return B;                                                                                                                              \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_rl(__fastree_node(t_name) * A)                                                 \
    {                                                                                                                                          \
        __fastree_node(t_name) *B = A->right;                                                                                                  \
                                                                                                                                               \
        __fastree_node(t_name) *Bl = B->left;                                                                                                  \
        B->left = A;                                                                                                                           \
        A->right = Bl;                                                                                                                         \
                                                                                                                                               \
        A->height = __fastree_##t_name##_height(A);                                                                                            \
        B->height = __fastree_##t_name##_height(B);                                                                                            \
                                                                                                                                               \
        return B;                                                                                                                              \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_rlr(__fastree_node(t_name) * A)                                                \
    {                                                                                                                                          \
        __fastree_node(t_name) *B = A->left;                                                                                                   \
        __fastree_node(t_name) *C = B->right;                                                                                                  \
                                                                                                                                               \
        A->left = C->right;                                                                                                                    \
        B->right = C->left;                                                                                                                    \
        C->left = B;                                                                                                                           \
        C->right = A;                                                                                                                          \
                                                                                                                                               \
        B->height = __fastree_##t_name##_height(B);                                                                                            \
        A->height = __fastree_##t_name##_height(A);                                                                                            \
        C->height = __fastree_##t_name##_height(C);                                                                                            \
                                                                                                                                               \
        return C;                                                                                                                              \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_rrl(__fastree_node(t_name) * A)                                                \
    {                                                                                                                                          \
        __fastree_node(t_name) *B = A->right;                                                                                                  \
        __fastree_node(t_name) *C = B->left;                                                                                                   \
                                                                                                                                               \
        A->right = C->left;                                                                                                                    \
        B->left = C->right;                                                                                                                    \
        C->left = A;                                                                                                                           \
        C->right = B;                                                                                                                          \
                                                                                                                                               \
        B->height = __fastree_##t_name##_height(B);                                                                                            \
        A->height = __fastree_##t_name##_height(A);                                                                                            \
        C->height = __fastree_##t_name##_height(C);                                                                                            \
                                                                                                                                               \
        return C;                                                                                                                              \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_balance(__fastree_node(t_name) * A)                                            \
    {                                                                                                                                          \
        int balance = __fastree_##t_name##_balance_factor(A);                                                                                  \
        if (balance == 2 && __fastree_##t_name##_balance_factor(A->left) == 1)                                                                 \
        {                                                                                                                                      \
            return __fastree_##t_name##_rr(A);                                                                                                 \
        }                                                                                                                                      \
        else if (balance == 2 && __fastree_##t_name##_balance_factor(A->left) == -1)                                                           \
        {                                                                                                                                      \
            return __fastree_##t_name##_rlr(A);                                                                                                \
        }                                                                                                                                      \
        else if (balance == -2 && __fastree_##t_name##_balance_factor(A->right) == 1)                                                          \
        {                                                                                                                                      \
            return __fastree_##t_name##_rrl(A);                                                                                                \
        }                                                                                                                                      \
        else if (balance == -2 && __fastree_##t_name##_balance_factor(A->right) == -1)                                                         \
        {                                                                                                                                      \
            return __fastree_##t_name##_rl(A);                                                                                                 \
        }                                                                                                                                      \
        return A;                                                                                                                              \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_node(t_name) * __fastree_##t_name##_add(__fastree_type(t_name) * self, __fastree_node(t_name) * A, t_key newValue) \
    {                                                                                                                                          \
        if (A == NULL)                                                                                                                         \
        {                                                                                                                                      \
            self->length++;                                                                                                                    \
            __fastree_node(t_name) *new = xxmalloc(sizeof(__fastree_node(t_name)));                                                            \
            new->left = NULL;                                                                                                                  \
            new->right = NULL;                                                                                                                 \
            new->height = 0;                                                                                                                   \
            new->value = newValue;                                                                                                             \
            return new;                                                                                                                        \
        }                                                                                                                                      \
        int cmp = t_compare(newValue, A->value);                                                                                               \
        if (cmp > 0)                                                                                                                           \
        {                                                                                                                                      \
            A->right = __fastree_##t_name##_add(self, A->right, newValue);                                                                     \
        }                                                                                                                                      \
        else if (cmp < 0)                                                                                                                      \
        {                                                                                                                                      \
            A->left = __fastree_##t_name##_add(self, A->left, newValue);                                                                       \
        }                                                                                                                                      \
        A->height = __fastree_##t_name##_height(A);                                                                                            \
        return __fastree_##t_name##_balance(A);                                                                                                \
    }                                                                                                                                          \
    inline static void fastree_##t_name##_add(__fastree_type(t_name) * self, t_key value)                                                      \
    {                                                                                                                                          \
        self->head = __fastree_##t_name##_add(self, self->head, value);                                                                        \
    }                                                                                                                                          \
                                                                                                                                               \
    inline __fastree_node(t_name) * __fastree_##t_name##_remove(__fastree_type(t_name) * self, __fastree_node(t_name) * A, const t_key *value) \
    {                                                                                                                                          \
        if (A == NULL)                                                                                                                         \
            return A;                                                                                                                          \
        int cmp = t_compare(value, A->value);                                                                                                  \
        if (cmp > 0)                                                                                                                           \
        {                                                                                                                                      \
            A->right = __fastree_##t_name##_remove(self, A->right, value);                                                                     \
            A->height = __fastree_##t_name##_height(A);                                                                                        \
        }                                                                                                                                      \
        else if (cmp < 0)                                                                                                                      \
        {                                                                                                                                      \
            A->left = __fastree_##t_name##_remove(self, A->left, value);                                                                       \
            A->height = __fastree_##t_name##_height(A);                                                                                        \
        }                                                                                                                                      \
        else if (A->left == NULL)                                                                                                              \
        {                                                                                                                                      \
            __fastree_node(t_name) *right = A->right;                                                                                          \
            if (right != NULL)                                                                                                                 \
                right->height = __fastree_##t_name##_height(right);                                                                            \
            xxfree(A, sizeof(__fastree_node(t_name)));                                                                                         \
            self->length--;                                                                                                                    \
            A = right;                                                                                                                         \
        }                                                                                                                                      \
        else if (A->right == NULL)                                                                                                             \
        {                                                                                                                                      \
            __fastree_node(t_name) *left = A->left;                                                                                            \
            if (left != NULL)                                                                                                                  \
                left->height = __fastree_##t_name##_height(left);                                                                              \
            xxfree(A, sizeof(__fastree_node(t_name)));                                                                                         \
            self->length--;                                                                                                                    \
            A = left;                                                                                                                          \
        }                                                                                                                                      \
        else                                                                                                                                   \
        {                                                                                                                                      \
            __fastree_node(t_name) *successor = A->right, *parent = A;                                                                         \
            while (successor->left != NULL)                                                                                                    \
            {                                                                                                                                  \
                parent = successor;                                                                                                            \
                successor = successor->left;                                                                                                   \
            }                                                                                                                                  \
            if (parent == A)                                                                                                                   \
            {                                                                                                                                  \
                parent->right = successor->right;                                                                                              \
            }                                                                                                                                  \
            else                                                                                                                               \
            {                                                                                                                                  \
                parent->left = successor->right;                                                                                               \
            }                                                                                                                                  \
            parent->height = __fastree_##t_name##_height(parent);                                                                              \
            A->height = __fastree_##t_name##_height(A);                                                                                        \
            A->value = successor->value;                                                                                                       \
            xxfree(successor, sizeof(__fastree_node(t_name)));                                                                                 \
            self->length--;                                                                                                                    \
        }                                                                                                                                      \
        return __fastree_##t_name##_balance(A);                                                                                                \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static void fastree_##t_name##_remove(__fastree_type(t_name) * self, t_key value)                                                   \
    {                                                                                                                                          \
        self->head = __fastree_##t_name##_remove(self, self->head, value);                                                                     \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static bool fastree_##t_name##_has(__fastree_type(t_name) * self, t_key value)                                                      \
    {                                                                                                                                          \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static __fastree_type(t_name) * fastree_##t_name##_init()                                                                           \
    {                                                                                                                                          \
        __fastree_type(t_name) *self = (__fastree_type(t_name) *)xxmalloc(sizeof(__fastree_type(t_name)));                                     \
        self->length = 0;                                                                                                                      \
        self->head = NULL;                                                                                                                     \
        return self;                                                                                                                           \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static void __fastree_##t_name##_destroy(__fastree_node(t_name) * node)                                                             \
    {                                                                                                                                          \
        if (node == NULL)                                                                                                                      \
            return;                                                                                                                            \
        __fastree_node(t_name) *left = node->left;                                                                                             \
        __fastree_node(t_name) *right = node->right;                                                                                           \
        xxfree(node, sizeof(__fastree_node(t_name)));                                                                                          \
        __fastree_##t_name##_destroy(left);                                                                                                    \
        __fastree_##t_name##_destroy(right);                                                                                                   \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static void fastree_##t_name##_clear(__fastree_type(t_name) * self)                                                                 \
    {                                                                                                                                          \
        __fastree_##t_name##_destroy(self->head);                                                                                              \
        self->length = 0;                                                                                                                      \
    }                                                                                                                                          \
                                                                                                                                               \
    inline static void fastree_##t_name##_destroy(__fastree_type(t_name) * self)                                                               \
    {                                                                                                                                          \
        __fastree_##t_name##_destroy(self->head);                                                                                              \
        xxfree(self, sizeof(__fastree_type(t_name)));                                                                                          \
    }