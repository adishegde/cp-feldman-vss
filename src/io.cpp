#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "io.hpp"

extern int errno;

void syserr(char const *location) {
    fprintf(stderr, "%s - %s\n", location, strerror(errno));
}

void apperr(char const *location, char const *mssg) {
    fprintf(stderr, "%s - %s\n", location, mssg);
}
