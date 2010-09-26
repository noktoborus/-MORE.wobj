#include <stdlib.h>
#include <stdio.h>
#define __USE_BSD 1
#include <string.h>

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
	ERRORE_SUICIDE,
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
	"Kill self, because I suck"
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
	// if uncomplite line
	if (!buf && numcalls < 3) return ERRORE_UNCOMPL;
	// if overset vertex
	if (numcalls >= 3 && buf) return ERRORE_EXSCESS;
	// if stupid ._.
	if (!wvps || (numcalls != 3 && (!buf || !bfsz))) return ERRORE_STUPID;
	// if is end line
	if (numcalls == 3 && !buf) return ERRORE_OK;
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
	if (ptr && num)
	{
	   	ptr[num * 3 - 3 + numcalls] = strtof (buf, NULL); 
		//printf ("VERTEX #%d:%d %f\n", num, numcalls, ptr[num * 3 - 3 + numcalls]);
	}
	return ERRORE_OK;
}

int
state_vt (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	// с vt всё сложнее, т.к. может быть от 2 до 3 значений в юните
	return ERRORE_OK;
}

int
state_f (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	int32_t v[3] = {0, 0, 0};
	char *last = NULL;
	void *tmp = NULL;
	int c = 0;
	if (!wvps || !wvps->o) return ERRORE_STUPID;
	if (!numcalls && !buf) return ERRORE_EXSCESS;
	if (numcalls && !wvps->f_curr) return ERRORE_STUPID;
	if (!buf)
	{
		if (wvps->f_curr)
		{
			wvps->f_curr->next = wvps->f;
			wvps->f = wvps->f_curr;
			wvps->f_curr = NULL;
			return ERRORE_OK;
		}
		return ERRORE_STUPID;
	}
	if (!wvps->f_curr)
	{
		wvps->f_curr = (struct wvfo_f_t*)calloc (1, sizeof (struct wvfo_f_t));
		if (!wvps->f_curr) return ERRORE_NOMEM;
	}
	// пытаемся распарсить строку
	last = buf;
	for (c = 0; c < 3 && last < buf + bfsz; c++, last++)
	{
		v[c] = strtol (last, &last, 10);
		if (v[c])
		{
			switch (c)
			{
				case 0:
					wvps->o->use |= MPOLLY_USE_VERTEX;
					break;
				case 1:
					wvps->o->use |= MPOLLY_USE_TEXTUR;
					break;
				case 2:
					wvps->o->use |= MPOLLY_USE_NORMAL;
					break;
			}
		}
	}
	// переразмечаем пространство под список
	tmp = realloc (wvps->f_curr->ptr, sizeof (v) * (wvps->f_curr->len + 1));
	if (!tmp) return ERRORE_NOMEM;
	wvps->f_curr->ptr = (int32_t*)tmp;
	// и кладём туда своё значение
	memcpy ((void*)&(wvps->f_curr->ptr[wvps->f_curr->len * 3]), (const void*)&v, sizeof (v));
	wvps->f_curr->len++;
	if (!numcalls) wvps->o->fcount++;
	return ERRORE_OK;
}

int
state_o (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	struct wvfo_o_t *o;
	if (!wvps) return ERRORE_STUPID;
	if (!buf) return ERRORE_OK;
	o = (struct wvfo_o_t*)calloc (1, sizeof (struct wvfo_o_t));
	if (!o) return ERRORE_NOMEM;
	o->next = wvps->o;
	wvps->o = o;
	if (!(o->name = strdup (buf))) return ERRORE_NOMEM;
	return ERRORE_OK;
}

int
state_usemtl (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
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

static inline int
wvfo_model_build (struct wvfo_parser_t *wvps)
{
	struct wvfo_f_t *f = NULL;
	size_t pollyn = 0;
	size_t fplen = 0;
	void *tmp = NULL;
	int nc = 0;
	int c = 0;
	int _tf = 0;
	if (!wvps || !wvps->o || (wvps->o->fcount > 0 && !wvps->f))
	   	return ERRORE_STUPID;
	if (!wvps->model)
	{
		wvps->model = (struct model_t*)calloc (1, sizeof (struct model_t));
		if (!wvps->model) return ERRORE_NOMEM;
	}
	if (wvps->o->name != NULL && wvps->model->name == NULL)
	{
		wvps->model->name = wvps->o->name;
		wvps->o->name = NULL;
	}
	while (wvps->o->fcount-- && (f = wvps->f))
	{
		// если длина блока полли не равна длине текущего блока
		if (fplen != f->len || fplen == 0)
		{
			pollyn = 0;
			// поиск нужного указателля на полли
			if (wvps->model->pollys)
			{
				do
				{
					// если нашли пустой слот
					if (!wvps->model->pollys[pollyn]) break;
					// если нашли нашу полли (с нужным размером блока)
					if (wvps->model->pollys[pollyn]->len == f->len) break;
				}
				while (pollyn++);
			}
			// если длина нулевая, или не нашли нужную полли
			// нужно разметить память
			if (pollyn == wvps->model->pollys_num)
			{
				printf ("REAC 1\n");
				tmp = realloc ((void*)wvps->model->pollys,\
					   	sizeof (struct model_polly_t*) * (pollyn + 1));
				if (!tmp) return ERRORE_NOMEM;
				wvps->model->pollys = (struct model_polly_t**)tmp;
				// занялуем память
				wvps->model->pollys[pollyn] = NULL;
				// увеличиваем счётчик полли
				wvps->model->pollys_num++;
			}
			// если слот пустой
			if (!wvps->model->pollys[pollyn])
			{
				nc = 0;
				// считаем какой размер структуры нам нужен
				if (wvps->o->use & MPOLLY_USE_VERTEX) nc++;
				if (wvps->o->use & MPOLLY_USE_TEXTUR) nc++;
				if (wvps->o->use & MPOLLY_USE_TEXTUR) nc++;
				// если нет вертексов, если НИЧЕГО НЕТ
				if (!nc) return ERRORE_SUICIDE;
				// пытаемся заполнить слот
				// выделяем память под стуктуру + небольшой хвост из 1-2 float*, если требуется
				// (под первый float* память выделяется при выделении под model_polly_t)) 
				tmp = calloc (1, sizeof (struct model_polly_t) + sizeof (float*) * (nc - 1));
				// памяти не хватило
				if (!tmp) return ERRORE_NOMEM;
				wvps->model->pollys[pollyn] = (struct model_polly_t*)tmp;
				// заполняем некоторые основные поля
				wvps->model->pollys[pollyn]->use = wvps->o->use;
				wvps->model->pollys[pollyn]->len = f->len;
			}
			// ня
			fplen = f->len;
		}
		if (wvps->model->pollys_num < pollyn || fplen != f->len) return ERRORE_SUICIDE;
		// перераспределяем память под новый узел
		nc = 0;
		if (wvps->o->use & MPOLLY_USE_VERTEX) nc++;
		if (wvps->o->use & MPOLLY_USE_TEXTUR) nc++;
		if (wvps->o->use & MPOLLY_USE_NORMAL) nc++;
		// указатель на нужный блок (до переразметки - указатель на конец массива)
		c = wvps->model->pollys[pollyn]->num * fplen * sizeof (float) * 3;
		if (!nc) return ERRORE_SUICIDE;
		while (nc--)
		{
			tmp = realloc ((void*)wvps->model->pollys[pollyn]->vertex[nc], c + (fplen * sizeof (float) * 3));
			if (!tmp) return ERRORE_NOMEM;
			// подчищаем память
			memset ((void*)&(((char*)tmp)[c]), 0, (fplen * sizeof (float) * 3));
			// заполняем базовые поля
			wvps->model->pollys[pollyn]->vertex[nc] = (float*)tmp;
			wvps->model->pollys[pollyn]->num++;
		}
		// FF
		c = wvps->model->pollys[pollyn]->num * f->len * 3;
		_tf = 0;
		fplen = -1;
		while (++fplen < f->len)
		{
			nc = 0;
			if (wvps->o->use & MPOLLY_USE_VERTEX)
			{
				if (f->ptr[fplen * 3] < 0)
					_tf = ((wvps->v_num - f->ptr[fplen * 3]) - 1);
				else _tf = (f->ptr[fplen * 3] - 1);
				if (_tf >= 0 && _tf < wvps->v_num)
				{
					_tf *= 3;
					wvps->model->pollys[pollyn]->vertex[nc][c - fplen * 3 - 3] = wvps->v[_tf + 0];
					wvps->model->pollys[pollyn]->vertex[nc][c - fplen * 3 - 2] = wvps->v[_tf + 1];
					wvps->model->pollys[pollyn]->vertex[nc][c - fplen * 3 - 1] = wvps->v[_tf + 2];
				}
				nc++;
			}
		}
		// end while
		wvps->f = f->next;
		free (f);
		f = NULL;
	}
	/*
	void *tmp;
	struct wvfo_f_t *f;
	size_t fnum;
	size_t c;
	size_t nc;
	int32_t _tf;
	if (!wvps->model)
	{
		wvps->model = (struct model_t*)calloc (1, sizeof (struct model_t));
		if (!wvps->model) return ERRORE_NOMEM;
	}
	if (wvps->f_num != wvps->model->pollys_num)
	{
		if (wvps->model->pollys_num > wvps->f_num)
		{
			while (wvps->model->pollys_num-- > wvps->f_num)
			{
				if (wvps->model->pollys[wvps->model->pollys_num])
				{
					c = 0;
					if (wvps->model->pollys[wvps->model->pollys_num]->use & MPOLLY_USE_VERTEX) c++;
					if (wvps->model->pollys[wvps->model->pollys_num]->use & MPOLLY_USE_TEXTUR) c++;
					if (wvps->model->pollys[wvps->model->pollys_num]->use & MPOLLY_USE_NORMAL) c++;
					while (c--) free (wvps->model->pollys[wvps->model->pollys_num]->vertex[c]);
				}
				free (wvps->model->pollys[wvps->model->pollys_num]);
			}
		}
		tmp = realloc ((void*)wvps->model->pollys, (sizeof (struct model_polly_t*) * wvps->f_num));
		if (!tmp) return ERRORE_NOMEM;
		wvps->model->pollys = (struct model_polly_t**)tmp;
		while (wvps->model->pollys_num < wvps->f_num)
		{
			 wvps->model->pollys[wvps->model->pollys_num++] = NULL;
		}
	}
	// обрабатываем группы f(ace)-ов
	if (wvps->f_num)
	{
		wvps->model->pollys_num = wvps->f_num;
		// не забываем, что список wvps->f "перевёрнут"
		for (f = wvps->f, fnum = wvps->f_num; fnum--, f; f = f->next)
		{
			nc = 0;
			if (f->use & MPOLLY_USE_VERTEX) nc++;
			if (f->use & MPOLLY_USE_TEXTUR) nc++;
			if (f->use & MPOLLY_USE_NORMAL) nc++;
			c = 0;
			if (wvps->model->pollys[fnum])
			{
				if (wvps->model->pollys[fnum]->use & MPOLLY_USE_VERTEX) c++;
				if (wvps->model->pollys[fnum]->use & MPOLLY_USE_TEXTUR) c++;
				if (wvps->model->pollys[fnum]->use & MPOLLY_USE_NORMAL) c++;
			}
			// проверка на необходимость переразметки структуры
			if (c != nc)
			{
				if (c > nc)
				{
					while (c-- > nc)
					{
					   	free (wvps->model->pollys[fnum]->vertex[c]);
						wvps->model->pollys[fnum]->vertex[c] = NULL;
					}
				}
				tmp = realloc ((void*)wvps->model->pollys[fnum],\
					   	sizeof (struct model_polly_t) + sizeof (float*) * (nc>0?(nc - 1):0));
				if (!tmp) return ERRORE_NOMEM;
				wvps->model->pollys[fnum] = (struct model_polly_t*)tmp;
				wvps->model->pollys[fnum]->use = f->use;
				while (c < nc)
				{
					wvps->model->pollys[fnum]->vertex[c++] = NULL;
				}
			}
			// тот же фокус, только с количеством групп в списках
			if (f->len != wvps->model->pollys[fnum]->len || 
					f->num != wvps->model->pollys[fnum]->len)
			{
				while (c--)
				{
					if (wvps->model->pollys[fnum]->vertex[c])
					{
						free (wvps->model->pollys[fnum]->vertex[c]);
						wvps->model->pollys[fnum]->vertex[c] = NULL;
					}
					wvps->model->pollys[fnum]->vertex[c] =\
					   	(float*)calloc (1, sizeof (float) * (f->num * f->len * 3));
					if (!wvps->model->pollys[fnum]->vertex[c])
						return ERRORE_NOMEM;
				}
				wvps->model->pollys[fnum]->len = f->len;
				wvps->model->pollys[fnum]->num = f->num;
			}
			_tf = 2;
			// заполняем списки
			for (nc = 0; nc < f->len * f->num; nc /= 3, nc++)
			{
				nc *= 3;
				c = 0;
				if (f->ptr[nc] && f->use & MPOLLY_USE_VERTEX)
				{
					if (f->ptr[nc] < 0)
						_tf = ((wvps->v_num - f->ptr[nc]) - 1) * 3;
					else _tf = (f->ptr[nc] - 1) * 3;
					//_tf = (_tf - 1);
					if (_tf >= 0 && _tf < (wvps->v_num * 3))
					{
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->v[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->v[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->v[_tf + 2];
					}
					else
						printf ("E %d %d\n", nc, _tf);
					c++;
				}
				if (f->ptr[nc + 1] && f->use & MPOLLY_USE_TEXTUR)
				{
					if (f->ptr[nc + 1] < 0)
						_tf = ((wvps->vt_num - f->ptr[nc + 2]) - 1) * 3;
					else _tf = (f->ptr[nc + 1] - 1) * 3;
					if (_tf >= 0 && _tf < wvps->vt_num)
					{
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->vt[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->vt[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->vt[_tf + 2];
					}
					c++;
				}
				if (f->ptr[nc + 2] && f->use & MPOLLY_USE_NORMAL)
				{
					if (f->ptr[nc + 2] < 0)
						_tf = ((wvps->vn_num - f->ptr[nc + 2]) - 1) * 3;
					else _tf = (f->ptr[nc + 2] - 1) * 3;
					if (_tf >= 0 && _tf < wvps->vt_num)
					{
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->vn[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->vn[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->vn[_tf + 2];
					}
				}
		//printf ("%p IN\n" ,malloc (10));
			}
		//printf ("%p OU\n" ,malloc (10));
		malloc (10);

		}
	}
	// TODO: feel model
	*/
	return ERRORE_OK;
}

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
				wvps->errored = state_table[wvps->state].callback (wvps, wvps->point++, NULL, 0);
				if (wvps->errored) break;
			}
			wvps->cline++;
			wvps->state = STATE_ZERO;
		}
		else
		if (wvps->state == STATE_ZERO)
		{
			wvps->point = 0;
			if (buf[r] == '#')
			{
				wvps->state = STATE_SEEK;
			}
			else
			if (sz == 1)
			{
				switch (buf[r])
				{
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
		printf ("`%s\"\n", &buf[r]);
		// TODO: free () all
	}
	else
	{
		printf ("@ v=`%d'\n", wvps->v_num);
		printf ("@@ name=`%s' fcount=`%d'\n", wvps->o->name, wvps->o->fcount);
		printf ("@@@ USE_VERTEX = %d\n", wvps->o->use & MPOLLY_USE_VERTEX);
		printf ("@@@ USE_TEXTUR = %d\n", wvps->o->use & MPOLLY_USE_TEXTUR);
		printf ("@@@ USE_NORMAL = %d\n", wvps->o->use & MPOLLY_USE_NORMAL);
		printf ("@@@ BUILD MODEL\n");
		if((wvps->errored = wvfo_model_build (wvps)) == ERRORE_OK)
		{
			model = wvps->model;
			wvps->model = NULL;
			printf ("@ BUILD OK\n");
			return model;
		}
	}
	printf ("@ END %u\n", r);
	return model;
}

