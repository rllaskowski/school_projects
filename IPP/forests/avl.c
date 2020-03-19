#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "avl.h"

typedef struct avl {
    struct avl *left;
    struct avl *right;
    size_t maxLeft;
    size_t maxRight;
    char *key;
    struct avl *value;
} avl_t;

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

static avl_t * createAvl(const char *key, avl_t *value) {
    avl_t *newNode = malloc(sizeof(*newNode));

    if (newNode) {
        newNode->key = strdup(key);
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->maxLeft = 0;
        newNode->maxRight = 0;
    }
    return newNode;
}

static void leftRotate(avl_t **avl) {
    avl_t *node = *avl;
    avl_t *rightLeft = (*avl)->right->left;

    *avl = (*avl)->right;
    (*avl)->left = node;
    node->right = rightLeft;

    node->maxRight = (*avl)->maxLeft;
    (*avl)->maxLeft = max(node->maxLeft, node->maxRight)+1;
}

static void rightRotate(avl_t **avl) {
    avl_t *node = *avl;
    avl_t *leftRightNode = (*avl)->left->right;

    *avl = (*avl)->left;
    (*avl)->right = node;
    node->left = leftRightNode;

    node->maxLeft = (*avl)->maxRight;
    (*avl)->maxRight = max(node->maxLeft, node->maxRight)+1;
}

static void leftRightRotate(avl_t **avl) {
    leftRotate(&((*avl)->left));
    rightRotate(avl);
}

static void rightLeftRotate(avl_t **avl) {
    rightRotate(&((*avl)->right));
    leftRotate(avl);
}

static void fixAvl(avl_t **avl) {
    avl_t *node = *avl;
    
    if (node->maxLeft > node->maxRight+1) {
        if (node->left->maxLeft > node->left->maxRight) {
            rightRotate(avl);
        } else {
            leftRightRotate(avl);
        }
    } else if (node->maxRight > node->maxLeft+1) {
        if (node->right->maxRight > node->right->maxLeft) {
            leftRotate(avl);
        } else {
            rightLeftRotate(avl);
        }
    }
}

avl_t * addKey(avl_t **avl, const char* key) {
    avl_t *node = *avl;
    avl_t *result = NULL;

    if (node != NULL) {
        int cmp = strcmp(key, node->key);

        if (cmp == 0) {
            result = node;
        } else if (cmp < 0) {
            result = addKey(&node->left, key);
            node->maxLeft = max(node->left->maxLeft, node->left->maxRight)+1;
        } else {
            result = addKey(&node->right, key);
            node->maxRight = max(node->right->maxLeft, node->right->maxRight)+1;
        }

        fixAvl(avl);
    } else {
        *avl = createAvl(key, NULL);
        result = *avl;
    }
    return result;
}

avl_t * findKey(avl_t *avl, const char *key) {
    if (avl != NULL) {
        int cmp = strcmp(key, avl->key);

        if (cmp == 0) {
            return avl;
        } else if (cmp < 0) {
            return findKey(avl->left, key);
        } else {
            return findKey(avl->right, key);
        }
    }
    return NULL;
}

avl_t ** getValue(avl_t *node) {
    if (node != NULL) {
        return &node->value;
    }
    return NULL;
}

const char * getKey(avl_t *node) {
    if (node != NULL) {
        return node->key;
    }
    return NULL;
}

avl_t * getLeft(avl_t *node) {
    if (node != NULL) {
        return node->left;
    }
    return NULL;
}

avl_t * getRight(avl_t *node) {
    if (node != NULL) {
        return node->right;
    }
    return NULL;
}

static avl_t * findMin(avl_t *avl) {
    if (avl != NULL) {
        if (avl->left == NULL) {
            return avl;
        } else {
            return findMin(avl->left);
        }
    }
    return NULL;
}

void removeKey(avl_t **avl, const char *key) {
    if (avl != NULL && *avl != NULL) {
        avl_t *node = *avl;

        int cmp = strcmp(key, node->key);

        if (cmp == 0) {
            freeAvl(node->value);
            node->value = NULL;
            free(node->key);

            if (node->left == NULL) {
                *avl = node->right;
                free(node);

            } else if (node->right == NULL) {
                *avl = node->left;
                free(node);
            } else {
                avl_t *minNode = findMin(node->right);
                node->key = strdup(minNode->key);
                node->value = minNode->value;
                minNode->value =  NULL;

                removeKey(&node->right, minNode->key);
            }
        } else {
            if (cmp < 0) {
                removeKey(&(node->left), key);
                node->maxLeft = 0;

                if (node->left != NULL) {
                    node->maxLeft = max(node->left->maxLeft, node->left->maxRight)+1;
                }
            } else {
                removeKey(&(node->right), key);
                node->maxRight = 0;

                if (node->right != NULL) {
                    node->maxRight = max(node->right->maxLeft, node->right->maxRight)+1;
                }
            }
            fixAvl(avl);
        }
    }
}

void freeAvl(avl_t *avl) {
    if (avl != NULL) {
        freeAvl(avl->left);
        freeAvl(avl->right);
        freeAvl(avl->value);
        free(avl->key);
        free(avl);
    }
}
