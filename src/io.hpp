#ifndef _IO_H
#define _IO_H

#include <unistd.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)

void syserr(char const *location);
void apperr(char const *location, char const *mssg);

#endif
