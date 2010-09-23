#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "wavefront.h"

#define WVOBJFILE "rez/model00.obj"
#define CHUNK_SZ 1024

int
main (int argc, char *argv[])
{
	char *buf = NULL;
	void *tmp = NULL;
	FILE *f = NULL;
	char chunk[CHUNK_SZ];
	size_t re = 0;
	size_t sz = 0;
	struct wvfo_parser_t wvps;
	struct model_t *model;
	if ((f = fopen (WVOBJFILE, "r")))
	{
		while ((re = fread (chunk, sizeof (char), CHUNK_SZ, f)))
		{
			if (!(tmp = realloc (buf, (sz + re) + sizeof (char))))
			{
				free (buf);
				buf = NULL;
				break;
			}
			buf = tmp;
			memcpy ((void*)&(buf[sz]), (const void*)chunk, re);
			sz += re;
		}
		fclose (f);
	}
	if (buf)
	{
		printf ("# LOAD: %p %u\n", buf, sz);
		wvfo_zero (&wvps);
		model = wvfo_load (&wvps, buf, sz);
		printf ("# COMPLETE %p\n", (void*)model);
	}
	return 0;
}

