/*CLAXI.H*********************************************************************

   Clax: Portable 3DStudio keyframer library, version 0.10
	 see inclosed LICENSE.TXT for licensing terms.

	 for documentation, refer to CLAX.TXT

   author           : BoRZoM
   file created     : 16/04/97
   file description : clax internal include file

*****************************************************************************/

/* compilation defines */

#define CLAX_SWAP_YZ          /* swap y and z when loading 3ds file     */
#define CLAX_DEBUG            /* debug mode                             */

/* externals */

extern c_SCENE  *clax_scene;
extern c_CAMERA *clax_camera;
extern int32     clax_flags;

/* structures, types */

typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned long  dword;
typedef signed char    sbyte;
typedef signed short   sword;
typedef signed long    sdword;

/* clax fileformat drivers */

int32 clax_load_mesh_3DS   (FILE *f);
int32 clax_load_motion_3DS (FILE *f);
//int32 clax_load_mesh_CLX   (FILE *f);
//int32 clax_load_motion_CLX (FILE *f);
//int32 clax_save_CLX        (FILE *f);

