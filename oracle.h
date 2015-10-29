#ifndef ORACLE_IMPL_H_
#define ORACLE_IMPL_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char **oracle(const char *word);
void initSeed(int seed);
void initAlloc(void *(*allocationFunction)(size_t));

void setEasyMode(void);
void setHardMode(void);

#ifdef __cplusplus
}
#endif

#endif
