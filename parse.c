#include "barec.h"

int type(void *stmt) {
    return *(int *)stmt;
}

void gencode(void *stmt, buffer *buff) {
    switch (type(stmt)) {
    }
}