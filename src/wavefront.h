#ifndef _WaVeFrOnT_h_
#define _WaVeFrOnT_h_
#include <stdint.h>
#include <stdlib.h>

// not in here
#define MPOLLY_USE_VERTEX 1
#define MPOLLY_USE_TEXTUR 2
#define MPOLLY_USE_NORMAL 4
struct model_polly_t
{
	uint8_t use;
	// sizeof (unit) == sizeof (float) * 3
	//
	// sizeof (len) == sizeof(unit) * n
	// где n - количество элементов в одной строке
	size_t len;
	// sizeof (num) == sizeof (len) * n
	// где n - количество строк
	size_t num;
	// :3
	// vertex, normal, texture
	float *vertex[1];
};

struct model_polly_t *mpolly_alloc (uint8_t use);
void mpolly_free (struct model_polly_t *polly);

struct model_t
{
	size_t pollys_num;
	struct model_polly_t **pollys;
};

// ok
struct wvfo_parser_t
{
	struct wvfo_parser_t *self;
	int state;
	size_t v_num;
	size_t vn_num;
	size_t vt_num;
	size_t f_num;
	
	float *v; // v_num * 3
	float *vn; // vn_num * 3
	float *vt;

	struct wvfo_f_t *f;
	struct wvfo_f_t *curr;
	
	struct model_t *model;
	// private, tempic
	int errored;
	size_t cline;
	size_t point;
};

struct wvfo_f_t
{
	uint8_t use;
	// unit size == (sizeof (int32_t) * 3)
	size_t len; // count of vertex groups (* unit size))
	size_t num; // size of array (* (len * unit size))
	int32_t *ptr;
	struct wvfo_f_t *next;
};

// init
void wvfo_zero (struct wvfo_parser_t *wvps);
// load && parse
struct model_t *wvfo_load (struct wvfo_parser_t *wvps, char *buffer, size_t bfsz);
// error handle
int wvfo_error (struct wvfo_parser_t *wvps);
// fin handle (use after error catch?)
int wvfo_isfin (struct wvfo_parser_t *wvps);

#endif // _WaVeFrOnT_h_

