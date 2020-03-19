#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "avl.h"

#define INPUT_ERROR 8
#define ERROR 4
#define SUCCESS 2
#define IGNORE 1
#define MAX_ARGS 3

typedef struct avl avl_t;

typedef struct instruction {
    const char *action;
    const char *args[MAX_ARGS];
    size_t argz;
} instruction_t;

static void printAvl(avl_t *avl) {
    if (avl != NULL) {
        printAvl(getLeft(avl));
        printf("%s\n", getKey(avl));
        printAvl(getRight(avl));
    }
}

static bool check(avl_t *avl, const char *keys[MAX_ARGS], int depth) {
    if (depth == MAX_ARGS || keys[depth] == NULL) {
        return true;
    }

    if (avl == NULL) {
        return false;
    }

    if (strcmp("*", keys[depth]) == 0) {
         if (check(*getValue(avl), keys, depth+1)) {
            return true;
        }
        if (check(getLeft(avl), keys, depth) ||
                check(getRight(avl), keys, depth)) {
            return true;
        }
    } else {
        avl_t *node = findKey(avl, keys[depth]);

        if (node != NULL) {
            return check(*getValue(node), keys, depth+1);
        }
    }

    return false;
}

static int add(avl_t **avl, const char *keys[MAX_ARGS], int depth) {
    if (depth < MAX_ARGS && keys[depth] != NULL) {
        avl_t *node = addKey(avl, keys[depth]);

        if (node == NULL) {
            return ERROR;
        } else {
            return add(getValue(node), keys, depth+1);
        }
    }

    return SUCCESS;
}

static void print(avl_t *avl, const char *keys[MAX_ARGS], int depth) {
    if (depth == MAX_ARGS || keys[depth] == NULL) {
        printAvl(avl);
    } else {
        avl_t *node = findKey(avl, keys[depth]);

        if (node != NULL) {
            print(*getValue(node), keys, depth+1);
        }
    }
}

static void delete(avl_t **avl, const char *keys[MAX_ARGS], int depth) {
    if (avl != NULL) {
        if (depth == 0 && keys[0] == NULL) {
            freeAvl(*avl);
            *avl = NULL;
        } else if (depth == 2 || keys[depth+1] == NULL) {
            removeKey(avl, keys[depth]);
        } else {
            avl_t *node = findKey(*avl, keys[depth]); 
            delete(getValue(node), keys, depth+1); 
        }
    }
}

static int parseLine(char *line, instruction_t *instruction) {
     if (line[0] == '#') {
        return IGNORE;
    }

    instruction->action = strtok(line, " ");
    
    if (instruction->action == NULL) {
        // Blank line
        return IGNORE;
    }

    instruction->argz = 0;

    for (size_t i = 0; i < MAX_ARGS; ++i) {
        instruction->args[i] = strtok(NULL, " ");
        instruction->argz += instruction->args[i] == NULL ? 0 : 1;
    }

    if (strtok(NULL, " ") != NULL) {
        // # of args is greater than MAX_ARGS
        return INPUT_ERROR;
    }
    return SUCCESS;
}

static int runInstruction(instruction_t *instruction, avl_t **db) {
    int flag = SUCCESS;

    if (strcmp(instruction->action, "ADD") == 0) {
        if (instruction->argz > 0) {
            flag = add(db, instruction->args, 0);

            if (flag == SUCCESS) {
                printf("OK\n");
            }
        } else {
            flag = INPUT_ERROR;
        }
    } else if (strcmp(instruction->action, "DEL") == 0) {
        delete(db, instruction->args, 0);
        printf("OK\n");
    } else if (strcmp(instruction->action, "PRINT") == 0) {
        if (instruction->argz < MAX_ARGS) {
            print(*db, instruction->args, 0);
        } else {
            flag = INPUT_ERROR;
        }
    } else if (strcmp(instruction->action, "CHECK") == 0) {
        if (instruction->argz > 0 && strcmp("*", instruction->args[instruction->argz-1]) != 0) {
            if (check(*db, instruction->args, 0)) {
                printf("YES\n");
            } else {
                printf("NO\n");
            }
        } else {
            flag = INPUT_ERROR;
        }
    } else {
        flag = INPUT_ERROR;
    }
    
    return flag;
}

int main() {
    char *buffer = NULL;
    size_t bufferSize = 0;
    int readChars = 0;

    avl_t *db = NULL;
    instruction_t instruction;

    while ((readChars = getline(&buffer, &bufferSize, stdin)) > 0) {
        buffer[readChars-1] = '\0';

        int flag = parseLine(buffer, &instruction);
        
        if (flag != IGNORE) {
            if (flag == INPUT_ERROR) {
                fprintf(stderr, "ERROR\n");
            } else if(runInstruction(&instruction, &db) == INPUT_ERROR) {
                fprintf(stderr, "ERROR\n");
            }
        }
    }

    freeAvl(db);
    free(buffer);

    return 0;
}