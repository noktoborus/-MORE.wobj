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
	return ERRORE_OK;
}

int
state_f (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
	int32_t v[3] = {0, 0, 0};
	int c = 0;
	char *last = NULL;
	void *tmp = NULL;
	size_t num = 0;
	//  face групируется по группам, в каждой группе законченный объект
	//
	// значит мы создаем wvfo_f_t, складываем туда набигающие номера v, vn, vt
	// до тех пор, пока не сменится группа
	//
	if (!wvps) return ERRORE_STUPID;
	// предположительно конец строки
	if (!buf)
	{
		// если это конец строки, то нужно сделать проверку на количество элементов
		// закрыть структуру и вообще
		//
		// получили конец строки, а начала-то не было
		// или количество строк не соотвествует конечной строке
		if (!wvps->curr || wvps->curr->len > numcalls) return ERRORE_UNCOMPL;
		//// иначе всё хорошо и нам требуется добавить текущую группу в список 
		//// fix: добавить это надо не сдесь, т.е. вносить изменения в очёредность требуется в
		//// fix: state_g, т.е. при смене группы, а не окончания строки с f(ace)
		//wvps->curr->next = wvps->f;
		//wvps->f = wvps->curr;
		//wvps->curr = NULL;
		return ERRORE_OK;
	}
	// если нет текущей структуры ._.
	if (!wvps->curr)
	{
		wvps->curr = (struct wvfo_f_t*)calloc (1, sizeof (struct wvfo_f_t));
		if (!wvps->curr) return ERRORE_NOMEM;
		// раметили, а теперь вносим в список
		wvps->curr->next = wvps->f;
		wvps->f = wvps->curr;
		wvps->f_num++;
	}
	// если это не первый блок в списке и количество элементов строки привышает допустимое
	// количество
	if (wvps->curr->num > 1 && wvps->curr->len <= numcalls) return ERRORE_EXSCESS;
	// пытаемся распарсить строку
	last = buf;
	for (c = 0; c < 3; c++)
	{
		v[c] = strtol (last, &last, 10);
		// если это первый блок в списке и число было получено
		if (wvps->curr->num < 2 && v[c]) 
		{
			switch (c)
			{
				case 0:
					wvps->curr->use |= MPOLLY_USE_VERTEX;
					break;
				case 1:
					wvps->curr->use |= MPOLLY_USE_TEXTUR;
					break;
				case 2:
					wvps->curr->use |= MPOLLY_USE_NORMAL;
					break;
			}
		}
		if (++last >= buf + bfsz) break;
	}
	if (!numcalls || wvps->curr->len == numcalls)
	{
		c = 0;
		if (wvps->curr->len == numcalls)
		{
			c = sizeof (v) * (wvps->curr->len + 1);
		}
		else
		{
			c = sizeof (v) * wvps->curr->len * (wvps->curr->num + 1);
		}
		tmp = realloc (wvps->curr->ptr, c);
		if (!tmp) return ERRORE_NOMEM;
		wvps->curr->ptr = (int32_t*)tmp;
		num = wvps->curr->num;
		if (numcalls == wvps->curr->len) wvps->curr->len++;
		if (!numcalls) wvps->curr->num++;
	}
	// если это первая строка - инкрементируем, вся группа f должна ровнять по первому элементу
	// или требуется пересмотреть весь алгоритм, как мне кажется второе более логично
	// но первое проще для реализации ._.
	// бряк
	c = 0;
	if (wvps->curr->num == 1) c = numcalls * 3;
	else
		c = ((wvps->curr->len * 3) * wvps->curr->num) - wvps->curr->len * 3 + numcalls * 3;
	memcpy ((void*)&((wvps->curr->ptr)[c]), (const void*)v, sizeof (v));
	return ERRORE_OK;
}

int
state_o (struct wvfo_parser_t *wvps, size_t numcalls, char *buf, size_t bfsz)
{
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
	void *tmp;
	struct wvfo_f_t *f;
	size_t fnum;
	size_t c;
	size_t nc;
	int32_t _tf;
	printf ("@ BUILD MODEL\n");
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
					   	(float*)calloc (1, sizeof (float) * (f->len * 3));
					if (!wvps->model->pollys[fnum]->vertex[c])
						return ERRORE_NOMEM;
				}
				wvps->model->pollys[fnum]->len = f->len;
				wvps->model->pollys[fnum]->num = f->num;
			}
			// заполняем списки
			for (nc = 0; nc < f->len * f->num; nc++)
			{
				if (!f->ptr[nc]) continue;
				c = 0;
				if (f->use & MPOLLY_USE_VERTEX)
				{
					if (f->ptr[nc] < 0)
						_tf = wvps->v_num - f->ptr[nc];
					else _tf = f->ptr[nc];
					//printf ("^ %d %d\n", nc, f->ptr[1]);
					if (_tf > 0 && _tf < wvps->v_num)
					{
						_tf = (_tf - 1) * 3;
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->v[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->v[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->v[_tf + 2];
					}
					c++;
				}
				if (f->use & MPOLLY_USE_TEXTUR)
				{
					if (f->ptr[nc] < 0)
						_tf = wvps->vt_num - f->ptr[nc];
					else _tf = f->ptr[nc];
					if (_tf > 0 && _tf < wvps->vt_num)
					{
						_tf = (_tf - 1) * 3;
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->vt[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->vt[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->vt[_tf + 2];
					}
					c++;
				}
				if (f->use & MPOLLY_USE_NORMAL)
				{
					if (f->ptr[nc] < 0)
						_tf = wvps->vn_num - f->ptr[nc];
					else _tf = f->ptr[nc];
					if (_tf > 0 && _tf < wvps->vt_num)
					{
						_tf = (_tf - 1) * 3;
						wvps->model->pollys[fnum]->vertex[c][nc] = wvps->vn[_tf];
						wvps->model->pollys[fnum]->vertex[c][nc + 1] = wvps->vn[_tf + 1];
						wvps->model->pollys[fnum]->vertex[c][nc + 2] = wvps->vn[_tf + 2];
					}
				}
			}
		}
	}
	// TODO: feel model
	printf ("@ BUILD OK\n");
	return 0;
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
		// TODO: free () all
	}
	else
	{
		int s = 0;
		int g = 0;
		s = wvps->f->num;
		g = wvps->f->len;
		printf ("len: %d %d\n", s, g);
		s *= g * 3;
		printf ("cc: %d\n", s);
		for (g = 0; g < s; g++)
		{
			if (!(g % 3)) printf ("| ");
			printf ("%d ", wvps->f->ptr[g]);
		}
		printf ("\n");
		for (s = 0; s < wvps->f->num; s++)
		{
			printf ("f ");
			for (g = 0; g < wvps->f->len; g++)
			{
			//	printf ("%d ", s * (wvps->f->len * 3) + (g * 3));
				printf ("%d ", wvps->f->ptr[s * (wvps->f->len * 3) + (g * 3)]);
			}
			printf ("\n");
		}
		if((wvps->errored = wvfo_model_build (wvps)) == ERRORE_OK)
		{
			model = wvps->model;
			wvps->model = NULL;
			return model;
		}
	}
	printf ("@ END %u\n", r);
	return model;
}

