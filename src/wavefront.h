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
	struct model_polly_t *pollys;
};

// ok
struct wvfo_parser_t
{
	struct wvfo_parser_t *self;
	int state;
	size_t v_num;
	size_t vn_num;
	size_t f_num;
	
	float *v; // v_num * 3
	float *vn; // vn_num * 3

	struct wvfo_f_t *f;
	struct wvfo_f_t *last;
	
	struct model_t *target;
	// private, tempic
	int errored;
	size_t cline;
	size_t point;
};

struct wvfo_f_t
{
	size_t num;
	uint32_t *ptr; // num * 3 {ptr_v, ptr_vt, ptr_vn}
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

