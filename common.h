/** common.h:
  * Define some typedefs
	* int types copied from stdint.h
	*/

#ifndef COMMON_H
#define COMMON_H

/* this is copied from GNU stdint.h */
typedef signed char		int8_t;
typedef short int			int16_t;
typedef int						int32_t;
# if __WORDSIZE == 64
typedef long int			int64_t;
# else
__extension__
typedef long long int	int64_t;
# endif

/* Unsigned.  */
typedef unsigned char				uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int				uint32_t;
# define __uint32_t_defined
#endif
#if __WORDSIZE == 64
typedef unsigned long int		uint64_t;
#else
__extension__
typedef unsigned long long int	uint64_t;
#endif

typedef struct registers
{
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} regs_t;


// functions
void memcpy(uint16_t *dest, uint16_t *src, uint32_t len);
void memset(uint8_t *dest, uint8_t val, uint32_t len);

#endif
