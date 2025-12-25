#ifndef STDDEF_H
#define STDDEF_H

#define NULL ((void*)0)

#ifndef offsetof
# define offsetof(type, field) ((size_t)(&((type*)(0))->field))
#endif

#endif
