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
	size_t tt = 0;
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
	if (model)
	{
		printf ("# MODEL %p\n", (void*)model);
		re = 0;
		if (model->pollys_num)
		{
			do
			{
				printf ("# POLLY #%d polygons count: %d with vertexes: %d\n",\
					   	re, model->pollys[re]->num, model->pollys[re]->len);
				sz = model->pollys[re]->num;
				if (sz && model->pollys[re]->len)
				{
					tt = 0;
					while (sz--)
					{
						printf ("# f ");
						for (tt = 0; tt < model->pollys[re]->len; tt++)
						{
							printf ("(%3.2f,", model->pollys[re]->vertex[0][sz * tt + 0]);
							printf (" %3.2f,", model->pollys[re]->vertex[0][sz * tt + 1]);
							printf (" %3.2f)", model->pollys[re]->vertex[0][sz * tt + 2]);
							printf (" ");
						}
						printf ("\n");
					}
				}
			}
			while (++re < model->pollys_num);
		}
	}
	return 0;
}

