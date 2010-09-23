#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "wavefront.h"

void
wvfo_zero (struct wvfo_parser_t *wvps)
{
	if (wvps->self == wvps)
	{
		// free all

	}
	// simple put zero
	memset ((void*)wvps, 0, sizeof (struct wvfo_parser_t));
	// set self (for future)
	wvps->self = wvps;
}

static inline size_t
_get_next_ (char *buf, size_t bfsz, size_t *offset)
{
	size_t r = *offset;
	while (*offset < bfsz)
	{
		if (r == *offset && (buf[r] == ' '/* || (r > 0 && buf[r] == '\n' && buf[r - 1] == '\\')*/))
		{
			r++;
			(*offset)++;
		}
		else
		if (buf[*offset] != ' ')
		{
			if (buf[*offset] == '\n')
			{
				if (r == (*offset))
					(*offset)++;
				break;
			}
			(*offset)++;
		}
		else break;
	}
	return r;
}

enum error_enume_t
{
	ERRORE_OK,
	ERRORE_STUPID,
	ERRORE_KEYP,
	ERRORE_UKEY,
	ERRORE_NOMEM,
	ERRORE_EXSCESS,
	ERRORE_UNCOMPL,
	// it\s last
	ERRORE_LAST,
};

char *error_table[] =
{
	"All Good",
	"Unknown error",
	"Unknown key parse error",
	"Unknown key",
	"Need more free memory",
	"Exscess value",
	"Uncomplete line",
	// its last
	"Unknown error value",
};

enum wvfo_state
{
	// control group 
	STATE_ZERO,
	// have callback
	STATE_V,
	STATE_VN,
	STATE_VT,
	STATE_F,
	STATE_O,
	STATE_USEMTL,
	// control group, last
	STATE_SEEK,
};

int
state_v_n (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	float *ptr = NULL;
	size_t num = 0;
	if (!wvps || !buf || !bfsz) return ERRORE_STUPID;
	if (numcalls >= 3 && buf) return ERRORE_EXSCESS;
	if (!buf && numcalls < 3) return ERRORE_UNCOMPL;
	if (wvps->state == STATE_V)
	{
		if (numcalls == 0)
		{
			ptr = (float*)realloc ((void*)wvps->v, sizeof (float) * ((wvps->v_num + 1) * 3));
			if (!ptr) return ERRORE_NOMEM;
			wvps->v = ptr;
			wvps->v_num++;
		}
		else ptr = wvps->v;
		num = wvps->v_num;
	}
	else
	{
		if (numcalls == 0)
		{
			ptr = (float*)realloc ((void*)wvps->vn, sizeof (float) * ((wvps->v_num + 1) * 3));
			if (!ptr) return ERRORE_NOMEM;
			wvps->vn = ptr;
			wvps->vn_num++;
		}
		else ptr = wvps->vn;
		num = wvps->vn_num;
	}
	if (ptr && num) ptr[num * 3 - 3 + numcalls] = strtof (buf, NULL); 
	return ERRORE_OK;
}

int
state_vt (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	if (!wvps || !buf || !bfsz) return ERRORE_STUPID;
	return ERRORE_OK;
}

int
state_f (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	if (!wvps || !buf || !bfsz) return ERRORE_STUPID;
	return ERRORE_OK;
}

int
state_o (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	if (!wvps || !buf || !bfsz) return ERRORE_STUPID;
	return ERRORE_OK;
}

int
state_usemtl (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	if (!wvps || !buf || !bfsz) return ERRORE_STUPID;
	return ERRORE_OK;
}

struct state_table_t
{
	char *name;
	int (*callback) (struct wvfo_parser_t*, size_t, char*, size_t);
} state_table[] =
{
	{"STATE_ZERO", NULL},
	{"v", state_v_n},
	{"vn", state_v_n},
	{"vt", state_vt},
	{"f", state_f},
	{"o", state_o},
	{"usemtl", state_usemtl},
	{"STATE_SEEK", NULL},
};

struct model_t *
wvfo_load (struct wvfo_parser_t *wvps, char *buf, size_t bfsz)
{
	struct model_t *model = NULL;
	// num of state calls
	wvps->point = 0;
	// line count
	wvps->cline = 1;
	// string size
	size_t sz = 0;
	// return value
	size_t r = 0;
	// offset value
	size_t offset = 0;
	// little fix :3
	char last = '0';
	wvps->state = STATE_ZERO;
	if (wvps->self != wvps) return NULL;
	printf ("@ BEGIN %p %u\n", buf, bfsz);
	while ((r = _get_next_ (buf, bfsz, &offset)) < bfsz && (sz = offset - r) != 0)
	{
		last = buf[offset];
		buf[offset] = '\0';
		if (buf[r] == '\n')
		{
			if (wvps->state < STATE_SEEK && state_table[wvps->state].callback)
			{
				state_table[wvps->state].callback (wvps, wvps->point++, NULL, 0);
			}
			wvps->cline++;
			wvps->state = STATE_ZERO;
		}
		else
		if (wvps->state == STATE_ZERO)
		{
			wvps->point = ERRORE_OK;
			if (sz == 1)
			{
				switch (buf[r])
				{
					case '#':
						wvps->state = STATE_SEEK;
						break;
					case 'v':
						wvps->state = STATE_V;
						break;
					case 'f':
						wvps->state = STATE_F;
						break;
					case 'o':
						wvps->state = STATE_O;
						break;
				}
			}
			else
			if (sz == 2)
			{
				if (!strcmp ("vn", &(buf[r])))
					wvps->state = STATE_VN;
				else
				if (!strcmp ("vt", &(buf[r])))
					wvps->state = STATE_VT;
			}
			else
			{
				if (!strcmp ("usemtl", &(buf[r])))
					wvps->state = STATE_USEMTL;
			}

			if (wvps->state == STATE_ZERO)
			{
				wvps->errored = ERRORE_UKEY;
				break;
			}
		}
		else
		if (wvps->state < STATE_SEEK && state_table[wvps->state].callback)
		{
			//printf ("%3d: %s\n", wvps->cline, state_table[state].name);
			wvps->errored = state_table[wvps->state].callback (wvps, wvps->point++,\
				   	&(buf[r]), sz);
			if (wvps->errored) break;
		}

		buf[offset] = last;
	}
	if (wvps->errored)
	{
		if (wvps->errored > ERRORE_LAST) wvps->errored = ERRORE_LAST;
		printf ("WVPS: %d (%s) line %d point %d (%s:%p)\n", wvps->errored,\
			   	error_table[wvps->errored], wvps->cline, wvps->point,\
			   	state_table[wvps->state].name, (void*)&(state_table[wvps->state].callback));
		// TODO: free all here
	}
	else
	{
		// TODO: feel model here
	}
	printf ("@ END %u\n", r);
	return model;
}

