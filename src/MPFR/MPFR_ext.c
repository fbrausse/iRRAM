
#include <stdlib.h>
#include <stddef.h>

#include <iRRAM/MPFR_interface.h>

void ext_mpfr_initialize(struct iRRAM_ext_mpfr_cache_t *cache)
{
	mpfr_set_default_prec(32);

	memset(cache, 0, sizeof(*cache));
}

void ext_mpfr_finalize(struct iRRAM_ext_mpfr_cache_t *cache)
{
	for (size_t i=cache->free_var_count; i; i--) {
		mpfr_clear(cache->free_vars[i-1]);
		free(cache->free_vars[i-1]);
		cache->total_freed_var_count++;
	}
	cache->free_var_count = 0;
}
