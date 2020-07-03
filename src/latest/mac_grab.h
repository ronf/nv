/*
	Netvideo version 4.0
	Written by Ron Frederick <ronf@timeheart.net>

	Mac video frame grab headers
*/

extern int Mac_Probe(void);
extern char *Mac_Attach(void);
extern void Mac_Detach(void);
extern grabproc_t *Mac_Start(int grabtype, int min_framespacing, int config,
			     reconfigproc_t *reconfig, void *enc_state);
extern void Mac_Stop(void);
