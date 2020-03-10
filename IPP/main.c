#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "avl.h"

typedef struct avl avl_t;

typedef struct database {
    avl_t *forests;
} db_t;

int compareStrings(void *a, void *b) {
    return strcmp((char *)a, (char *)b);
}

int printAvl(avl_t *avl) {
    if (avl != NULL) {
        printAvl(getLeft(avl));
        printf("%s\n", (char*)getKey(avl));
        printAvl(getRight(avl));
    }
}

int add(const char *forest, const char *tree, const char *animal, db_t *db) {
    avl_t *forestNode = findKey(db->forests, forest, compareStrings);
    if (forestNode == NULL) {
        forestNode = addKey(&db->forests, strdup(forest), compareStrings);
    }
}

int delete(const char *forest, const char *tree, const char *animal, db_t *db) {
    if (forest != NULL) {
        avl_t *forestNode = findKey(db->forests, forest, compareStrings);

        if (forestNode != NULL) {
            
        }
    }
}

int print(const char *forest, const char *tree, db_t *db) {
    if (forest == NULL) {
        printAvl(db->forests);
        return;
    }
    avl_t *forestAvl = findKey(db->forests, forest, compareStrings);

    if (forestAvl != NULL) {
        if (tree == NULL) {
            printAvl((avl_t *)getKey(forestAvl));
            return;
        }
        avl_t *forestAvl = findKey(db->forests, forest, compareStrings);

    }
}

int check(const char *forest, const char *tree, const char *animal, db_t *db) {
    
}

int runInstruction(char *instruction, db_t *db) {
    if (instruction[0] == '#') {
        return;
    }

    char *action = strtok(instruction, " ");
    const char *forest = strtok(NULL, " ");
    const char *tree = strtok(NULL, " ");
    const char *animal = strtok(NULL, " ");

    if (strtok(NULL, " ") != NULL) {

    }

    if (strcmp(action, "ADD")) {
        return add(forest, tree, animal, db);
    } else if (strcmp(action, "DEL")) {
        return delete(forest, tree, animal, db);
    } else if (strcmp(action, "PRINT")) {
        if (animal != NULL) {
            return 1;
        }
        return print(forest, tree, db);
    } else if (strcmp(action, "CHECK") == 0) {
        return check(forest, tree, animal, db);
    } else if (action == NULL) {
        return 0;
    } else {
        return 1;
    }
}

int main() {
    char *buffer = NULL;
    size_t bufferSize = 0;
    size_t readChars = 0;

    db_t *db = malloc(sizeof(*db));
    db->forests = NULL;

    while ((readChars = getline(&buffer, &bufferSize, stdin)) != 0) {
        buffer[readChars-1] = '\0';

        if(runInstruction(buffer, db) == 1) {
            fprintf(stderr, "ERROR\n");
        }
    }

    freeAvl(db->forests, NULL, NULL);

    free(buffer);
}