

#ifndef __COMMON2_H_
#define __COMMON2_H_	1


/*
 * General Purpose Utilities
 */
#define min(X, Y)				\
	({ typeof (X) __x = (X);		\
		typeof (Y) __y = (Y);		\
		(__x < __y) ? __x : __y; })

#define max(X, Y)				\
	({ typeof (X) __x = (X);		\
		typeof (Y) __y = (Y);		\
		(__x > __y) ? __x : __y; })

#define MIN(x, y)  min(x, y)
#define MAX(x, y)  max(x, y)

/*
 * Output a debug text when condition "cond" is met. The "cond" should be
 * computed by a preprocessor in the best case, allowing for the best
 * optimization.
 */
#define debug_cond(cond, fmt, args...)		\
	do {					\
		if (cond)			\
			kprintf(fmt, ##args);	\
	} while (0)

#define debug(fmt, args...)			\
	debug_cond(_DEBUG, fmt, ##args)
	
#define ROUND(a,b)		(((a) + (b) - 1) & ~((b) - 1))

#define ALIGN(x,a)		__ALIGN_MASK((x),(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))
	
/*
 * The ALLOC_CACHE_ALIGN_BUFFER macro is used to allocate a buffer on the
 * stack that meets the minimum architecture alignment requirements for DMA.
 * Such a buffer is useful for DMA operations where flushing and invalidating
 * the cache before and after a read and/or write operation is required for
 * correct operations.
 *
 * When called the macro creates an array on the stack that is sized such
 * that:
 *
 * 1) The beginning of the array can be advanced enough to be aligned.
 *
 * 2) The size of the aligned portion of the array is a multiple of the minimum
 *    architecture alignment required for DMA.
 *
 * 3) The aligned portion contains enough space for the original number of
 *    elements requested.
 *
 * The macro then creates a pointer to the aligned portion of this array and
 * assigns to the pointer the address of the first element in the aligned
 * portion of the array.
 *
 * Calling the macro as:
 *
 *     ALLOC_CACHE_ALIGN_BUFFER(uint32_t, buffer, 1024);
 *
 * Will result in something similar to saying:
 *
 *     uint32_t    buffer[1024];
 *
 * The following differences exist:
 *
 * 1) The resulting buffer is guaranteed to be aligned to the value of
 *    ARCH_DMA_MINALIGN.
 *
 * 2) The buffer variable created by the macro is a pointer to the specified
 *    type, and NOT an array of the specified type.  This can be very important
 *    if you want the address of the buffer, which you probably do, to pass it
 *    to the DMA hardware.  The value of &buffer is different in the two cases.
 *    In the macro case it will be the address of the pointer, not the address
 *    of the space reserved for the buffer.  However, in the second case it
 *    would be the address of the buffer.  So if you are replacing hard coded
 *    stack buffers with this macro you need to make sure you remove the & from
 *    the locations where you are taking the address of the buffer.
 *
 * Note that the size parameter is the number of array elements to allocate,
 * not the number of bytes.
 *
 * This macro can not be used outside of function scope, or for the creation
 * of a function scoped static buffer.  It can not be used to create a cache
 * line aligned global buffer.
 */
#define ALLOC_ALIGN_BUFFER(type, name, size, align)			\
	char __##name[ROUND(size * sizeof(type), align) + (align - 1)];	\
									\
	type *name = (type *) ALIGN((uintptr_t)__##name, align)
#define ALLOC_CACHE_ALIGN_BUFFER(type, name, size)			\
	ALLOC_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)


/*
 * DEFINE_CACHE_ALIGN_BUFFER() is similar to ALLOC_CACHE_ALIGN_BUFFER, but it's
 * purpose is to allow allocating aligned buffers outside of function scope.
 * Usage of this macro shall be avoided or used with extreme care!
 */
#define DEFINE_ALIGN_BUFFER(type, name, size, align)			\
	static char __##name[roundup(size * sizeof(type), align)]	\
			__attribute__((aligned(align)));				\
															\
	static type *name = (type *)__##name
#define DEFINE_CACHE_ALIGN_BUFFER(type, name, size)			\
	DEFINE_ALIGN_BUFFER(type, name, size, ARCH_DMA_MINALIGN)

#define get_unaligned  __get_unaligned_le
#define put_unaligned  __put_unaligned_le

#define __force __attribute__((force))

#define isprint(x) (1)

#if BYTE_ORDER == LITTLE_ENDIAN
# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)
# define cpu_to_be16(x)		uswap_16(x)
# define cpu_to_be32(x)		uswap_32(x)
# define cpu_to_be64(x)		uswap_64(x)
# define be16_to_cpu(x)		uswap_16(x)
# define be32_to_cpu(x)		uswap_32(x)
# define be64_to_cpu(x)		uswap_64(x)
#else
# define cpu_to_le16(x)		uswap_16(x)
# define cpu_to_le32(x)		uswap_32(x)
# define cpu_to_le64(x)		uswap_64(x)
# define le16_to_cpu(x)		uswap_16(x)
# define le32_to_cpu(x)		uswap_32(x)
# define le64_to_cpu(x)		uswap_64(x)
# define cpu_to_be16(x)		(x)
# define cpu_to_be32(x)		(x)
# define cpu_to_be64(x)		(x)
# define be16_to_cpu(x)		(x)
# define be32_to_cpu(x)		(x)
# define be64_to_cpu(x)		(x)
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#define cpu_to_le64s(x) do { (void)(x); } while (0)
#define le64_to_cpus(x) do { (void)(x); } while (0)
#define cpu_to_le32s(x) do { (void)(x); } while (0)
#define le32_to_cpus(x) do { (void)(x); } while (0)
#define cpu_to_le16s(x) do { (void)(x); } while (0)
#define le16_to_cpus(x) do { (void)(x); } while (0)
#define cpu_to_be64s(x) swab64s((x))
#define be64_to_cpus(x) swab64s((x))
#define cpu_to_be32s(x) swab32s((x))
#define be32_to_cpus(x) swab32s((x))
#define cpu_to_be16s(x) swab16s((x))
#define be16_to_cpus(x) swab16s((x))
#endif

#endif
