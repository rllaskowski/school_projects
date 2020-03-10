#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "avl.h"

typedef struct avl {
    struct avl *left;
    struct avl *right;
    size_t maxLeft;
    size_t maxRight;
    void *key;
    void *value;
} avl_t;

int max(int a, int b) {
    if (a > b) {
        return a;
    }
    return b;
}

avl_t * createAvl(void *key, void *value) {
    avl_t *newNode = malloc(sizeof(*newNode));

    if (newNode) {
        newNode->key = key;
        newNode->value = value;
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->maxLeft = 0;
        newNode->maxRight = 0;
    }
    return newNode;
}

void leftRotate(avl_t **avl) {
    avl_t *node = *avl;
    avl_t *rightLeft = (*avl)->right->left;

    *avl = (*avl)->right;
    (*avl)->left = node;
    node->right = rightLeft;

    node->maxRight = (*avl)->maxLeft;
    (*avl)->maxLeft = max(node->maxLeft, node->maxRight)+1;
}

void rightRotate(avl_t **avl) {
    avl_t *node = *avl;
    avl_t *leftRightNode = (*avl)->left->right;

    *avl = (*avl)->left;
    (*avl)->right = node;
    node->left = leftRightNode;

    node->maxLeft = (*avl)->maxRight;
    (*avl)->maxRight = max(node->maxLeft, node->maxRight)+1;
}

void leftRightRotate(avl_t **avl) {
    leftRotate(&((*avl)->left));
    rightRotate(avl);
}

void rightLeftRotate(avl_t **avl) {
    rightRotate(&((*avl)->right));
    leftRotate(avl);
}

void fixAvl(avl_t **avl) {
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

avl_t * addKey(avl_t **avl, void* key, int (*cmpFun)(void *, void *)) {
    avl_t *node = *avl;
    avl_t *result = NULL;

    if (node != NULL) {
        int cmp = cmpFun(key, node->key);

        if (cmp == 0) {
            result = node;
        } else if (cmp < 0) {
            result = addKey(&node->left, key, cmpFun);
            node->maxLeft = max(node->left->maxLeft, node->left->maxRight)+1;
        } else {
            result = addKey(&node->right, key, cmpFun);
            node->maxRight = max(node->right->maxLeft, node->right->maxRight)+1;
        }

        fixAvl(avl);
    } else {
        *avl = createAvl(key, NULL);
        result = *avl;
    }
    return result;
}

avl_t * findKey(avl_t *avl, void *key, int (*cmpFun)(void *, void *)) {
    if (avl != NULL) {
        int cmp = cmpFun(avl->key, key);

        if (cmp == 0) {
            return avl;
        } else if (cmp < -1) {
            return findKey(avl->left, key, cmpFun);
        } else {
            return findKey(avl->right, key, cmpFun);
        }
    }
    return NULL;
}

void setValue(avl_t *avl, void *value) {
    if (avl != NULL) {
        avl->value = value;
    }
}

void * getValue(avl_t *node) {
    if (node != NULL) {
        return node->value;
    }
    return NULL;
}

void * getKey(avl_t *node) {
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

void freeAvl(avl_t *avl, void (*freeKeyFun)(void *), void (*freeValueFun)(void *)) {
    if (avl != NULL) {
        freeAvl(avl->left, freeKeyFun, freeValueFun);
        freeAvl(avl->right, freeKeyFun, freeValueFun);

        if (freeKeyFun != NULL) {
            freeKeyFun(avl->key);
        }

        if (freeValueFun != NULL) {
            freeValueFun(avl->value);
        }

        free(avl);
    }
}
