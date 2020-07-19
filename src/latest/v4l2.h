/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Video for Linux 2i frame grab headers
*/

extern int V4L2_Probe(void);
extern char *V4L2_Attach(void);
extern void V4L2_Detach(void);
extern grabproc_t *V4L2_Start(int grabtype, int min_framespacing, int config,
			      reconfigproc_t *reconfig, void *enc_state);
extern void V4L2_Stop(void);
