/****************************************************************************/
/*  bolter_decode.h -- Return codes from Bolter_Decode subroutine           */
/****************************************************************************/

#define VxSUCCESS	0	/* Packet successfully decoded */
#define VxEXTRAPIXEL	1	/* Extra pixel past end of pixmap */
#define VxBADADDRESS	2	/* Bad address in video data */
#define VxUNTERMINATED	3	/* Packet not terminated */
#define VxBADHEADER	4	/* Bad video header format */
#define VxBADLENGTH	5	/* Unreasonable packet length */
#define VxNONMOTION	6	/* Non-motion video packet found */

#define BOLTER_WIDTH	256
#define BOLTER_HEIGHT	200

extern void Bolter_Decode(/* vidimage_t *image, char *data, int len */);
