// common.h:
// Define some typedefs
// int types copied from stdint.h

#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

/* this is copied from GNU stdint.h */
typedef signed char		int8_t;
typedef short int			int16_t;
typedef int						int32_t;
typedef unsigned char				uint8_t;
typedef unsigned short int	uint16_t;
typedef unsigned int				uint32_t, pde_t, pte_t;

#define min(a, b) ((a) < (b) ? (a) : (b));
#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b));

extern void panic(const char *message, const char *file, uint32_t line);
extern void panic_assert(const char *file, uint32_t line, const char *desc);


#endif
