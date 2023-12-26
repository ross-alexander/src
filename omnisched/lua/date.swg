%module Date
%{
#include <time.h>
#include "common.h"
%}

struct gloop {
       int year;
       time_t tick;
};
