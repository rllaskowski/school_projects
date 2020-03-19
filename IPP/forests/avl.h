#ifndef __AVL_H__
#define __AVL_H__

struct avl;

struct avl * create_avl(const char *key);
struct avl * getLeft(struct avl *node);
struct avl * getRight(struct avl *node);
struct avl * findKey(struct avl *avl, const char *key);
struct avl * addKey(struct avl **avl, const char* key);
void removeKey(struct avl **avl, const char *key);
void freeAvl(struct avl *avl);
struct avl ** getValue(struct avl *avl);
const char * getKey(struct avl *avl);

#endif  // __AVL_H__