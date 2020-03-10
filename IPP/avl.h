#ifndef __AVL_H__
#define __AVL_H__

struct avl;

struct avl * create_avl(const char *key);
struct avl * getLeft(struct avl *node);
struct avl * getRight(struct avl *node);
struct avl * findKey(struct avl *avl, void *key, int (*cmpFun)(void *, void *));
struct avl * addKey(struct avl **avl, void* key, int (*cmp)(void *, void *));
void freeAvl(struct avl *avl, void (*freeKeyFun)(void *), void (*freeValueFun)(void *));
void setValue(struct avl *avl, void *value);
void * getValue(struct avl *avl);
void * getKey(struct avl *avl);

#endif  // __AVL_H__