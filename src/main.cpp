#define _CRT_SECURE_NO_WARNINGS
//#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

#include <Windows.h>

//disable float warnings for loadjpeg
#pragma warning( disable : 4244 4305 )
#include "loadjpeg.h"

#include <stdio.h>
#include <math.h>

#include <io.h>
#include <direct.h> //getcwd

#include "../clax/src/clax.h"
#include "../clax/src/claxi.h"

#include "meta.h"

#ifndef _DEBUG
#pragma comment (lib, "../clax/lib/clax")
#endif

#define FREEGLUT_STATIC
#include <GL/glut.h>
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif

// globals, globals everywhere

const int WIDTH = 1280;
const int HEIGHT = 720;

float znear = 1.0f;
float zfar = 12000.0f;

int textures = 0;
GLuint TextureID[10];
int *texture[10];

int g_width = WIDTH;
int g_height = HEIGHT;
char g_fname[_MAX_PATH];

int timerrate = 20;

int faces = 0;

enum {
	MODE_TEXTURED = 0,
	MODE_SOLID,
	MODE_WIREFRAME,
	MODE_END
};

int mode = MODE_TEXTURED;
int modes = MODE_END;

int view = 0;
int views = 2;
int pause = 0;

float zoom, rotx, roty, rotz, ofsx, ofsy;
int g_lastx = 0;
int g_lasty = 0;
unsigned char Buttons[3] = { 0 };

c_SCENE  *scene;
c_CAMERA *cam;
c_LIGHT * lights[25];
int numlights;
int g_stensil = 1;

struct c_ADJ {
  DWORD face[3];
};

c_ADJ * adjacency = NULL;

float frames, frame;

void resettransform()
{
	zoom = 3000.0f;
	rotx = 150.0f;
	roty = 0.0f;
	rotz = 0.0f;
	ofsx = 0.0f;
	ofsy = 0.0f;
	frame = 0;
}

char * GetDocumentPath(char * buf, char * path)
{
	sprintf(buf, "");
	int max_level = 5;
	for (int i = 0; i < max_level; i++)
	{
		char dd[MAX_PATH];
		sprintf(dd, "%s%s", buf, path);
		if (GetFileAttributes(dd) != -1)
		{
			strcat(buf, path);
			return buf;
		}
		strcat(buf, "../");
	}
	return buf;
}

void timer(int value)
{
	clax_setframe(frame);
	clax_update();

	if (!pause)
		frame++;

	if (frame >= frames)
		frame = 0;
	
	NextFrame(frame); //metaballs

	glutTimerFunc(timerrate, timer, 1);
	glutPostRedisplay();
}

void drawScene()
{
	c_RGB white;
	white.r = 1.0f;
	white.g = 1.0f;
	white.b = 1.0f;

	if (textures==0 || mode!=MODE_TEXTURED )
		glDisable(GL_TEXTURE_2D);

	faces = 0;
	for (w_NODE* node = scene->world; node; node = node->next)
	{
		if (node->type == clax_obj_object)
		{
			c_OBJECT* obj = (c_OBJECT *)node->object;

			if (((obj->flags & clax_obj_hidden) == 0) && (obj->flags & clax_obj_chidden) == 0)
			{
				c_OBJECT o_copy;

				bool bMetaballs = strcmp (obj->name, "Fx01") == 0;
				bool bSmooth = bMetaballs;
				bool bTexture = mode==MODE_TEXTURED && textures!=0;

				if (bMetaballs)
				{
					memcpy(&o_copy,obj,sizeof(c_OBJECT));
					o_copy.numverts = triangles_tesselated*3;
					o_copy.numfaces = triangles_tesselated;
					o_copy.faces = mf;
					o_copy.vertices = mv;

					obj = &o_copy;

					for (int i = 0; i < obj->numverts; i++)
						mat_mulvec (obj->matrix, &obj->vertices[i].vert, &obj->vertices[i].pvert);

					c_MATRIX normat;
					mat_normalize (obj->matrix, normat);

					for (int i = 0; i < obj->numverts; i++)
						mat_mulnorm (normat, &obj->vertices[i].norm, &obj->vertices[i].pnorm);

					for (int i = 0; i < obj->numfaces; i++)
						mat_mulnorm (normat, &obj->faces[i].norm, &obj->faces[i].pnorm);
				}

				for (int i = 0; i < obj->numfaces; i++)
				{
					c_FACE * f = &obj->faces[i];
					int id = f->mat ? f->pmat->id : 0;
					c_RGB * c = f->mat ? &f->pmat->diffuse : &white;

					if (mode==MODE_TEXTURED)
						c = &white;

					glColor4f( c->r, c->g, c->b, 1.0f );

					if ( bMetaballs )
						id = textures-1;

					if ( bTexture )
						glBindTexture(GL_TEXTURE_2D, TextureID[id]);

					if ( mode!=MODE_TEXTURED )
						glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,(GLfloat*)c);

					glBegin(GL_TRIANGLES);

					c_VECTOR * n = &f->pnorm;

					if (!bSmooth)
						glNormal3f( -n->x, -n->y, -n->z);

					for ( int j=0; j<3; j++)
					{
						c_VERTEX * v = j==0 ? f->pa : ( j==1 ? f->pb : f->pc );

						if ( bMetaballs )
						{
							c_VECTOR view;
							vec_sub(&v->pvert, &cam->pos, &view);
							vec_normalize(&view, &view);

							c_VECTOR t;
							vec_cross(&view, &v->pnorm, &t);

							if ( bTexture )
								glTexCoord2f( t.x/2.0f+0.5f, t.y/2.0f+0.5f );
						}
						else
						{
							if ( bTexture )
								glTexCoord2f( v->u, 1.0f-v->v );

						}

						if ( bSmooth )
							glNormal3f( v->pnorm.x, v->pnorm.y, v->pnorm.z);

						glVertex3f( v->pvert.x, v->pvert.y, v->pvert.z );
					}

					glEnd();

					faces++;
				}
			}
		}
	}

}

float g_jitter = 0;

void DrawShadowVolumes ()
{
	for (w_NODE* node = scene->world; node; node = node->next)
	{
		if (node->type == clax_obj_object)
		{
			c_OBJECT* obj = (c_OBJECT *)node->object;
			bool bCaster = ( strcmp (obj->name, "s01") == 0 );

			if (bCaster && numlights>0)
			{
				if (adjacency==NULL)
				{
					adjacency = (c_ADJ*)malloc(obj->numfaces*sizeof(c_ADJ));

					for (int i = 0; i < obj->numfaces; i++)
					{
						c_FACE * f = &obj->faces[i];
						for (int j=0; j<3; j++)
						{
							c_VERTEX * pa = j==0 ? f->pa : ( j==1 ? f->pb : f->pc );
							c_VERTEX * pb = j==0 ? f->pb : ( j==1 ? f->pc : f->pa );
							adjacency[i].face[j] = 0;

							for (int ii = 0; ii < obj->numfaces; ii++)
							{
								c_FACE * ff = &obj->faces[ii];

								if (i!=ii) // wtf? rewrite to n^(n-1)/2
								for (int jj=0; jj<3; jj++)
								{
									c_VERTEX * paa = jj==0 ? ff->pa : ( jj==1 ? ff->pb : ff->pc );
									c_VERTEX * pbb = jj==0 ? ff->pb : ( jj==1 ? ff->pc : ff->pa );

									if (( paa==pa && pbb == pb) || ( paa==pb && pbb==pa))
										adjacency[i].face[j] = ii;
								}
							}
						}
					}
				}

				for (int k=0; k<numlights; k++)
				{
					c_LIGHT * light = lights[k];

					// recalc lightpos
					c_VECTOR LightPosition;
	//				mat_inverse (obj->matrix, invmat);
	//				mat_mulvec(invmat, &light->pos, &LightPosition);
					vec_copy(&light->pos, &LightPosition);

					c_VECTOR jit;
					vec_make(0,g_jitter,0, &jit);
					vec_add(&LightPosition, &jit, &LightPosition);

					for (int i=0; i<obj->numfaces; i++)
					{
						c_FACE * f = &obj->faces[i];
	
						c_VECTOR PolygonNormal;

						// recalculate normals, just in case
						c_VECTOR  a, b, normal;
						vec_sub (&obj->faces[i].pa->pvert, &obj->faces[i].pb->pvert, &a);
						vec_sub (&obj->faces[i].pb->pvert, &obj->faces[i].pc->pvert, &b);
						vec_cross (&a, &b, &normal);
						vec_normalize (&normal, &PolygonNormal);

						//vec_copy(&f->pnorm, &PolygonNormal); // tried to use existing pnorm, clax ruins it for some reason

						c_VECTOR AveragePolyPosition;
						vec_make(
							(f->pa->pvert.x + f->pb->pvert.x + f->pc->pvert.x)/3.0f,
							(f->pa->pvert.y + f->pb->pvert.y + f->pc->pvert.y)/3.0f,
							(f->pa->pvert.z + f->pb->pvert.z + f->pc->pvert.z)/3.0f,
							&AveragePolyPosition);

						c_VECTOR IncidentLightDir;
						vec_sub(&LightPosition, &AveragePolyPosition, &IncidentLightDir);

						if (vec_dot(&IncidentLightDir, &PolygonNormal) >= 0.0f )
							f->flags |= clax_face_visible;
						else
							f->flags &= ~clax_face_visible;
					}

					for (int i=0; i<obj->numfaces; i++)
					{
						c_FACE * f = &obj->faces[i];

						if ( f->flags & clax_face_visible )
						{
							for (int j=0; j<3; j++)
							{
								c_FACE * ff = &obj->faces[ adjacency[i].face[j] ];

								if ( !(ff->flags & clax_face_visible) )
								{
									c_VERTEX * v;
									c_VECTOR point;

									c_VERTEX * pa = j==0 ? f->pa : ( j==1 ? f->pb : f->pc );
									c_VERTEX * pb = j==0 ? f->pb : ( j==1 ? f->pc : f->pa );

									glBegin(GL_QUADS);

									float LargeNumber = 10000.0f;
									c_VECTOR lightdir;

									v = pa;
									vec_sub(&v->pvert, &LightPosition, &lightdir);
									vec_normalize(&lightdir, &lightdir);
									vec_scale(&lightdir, LargeNumber, &lightdir);
									vec_add(&v->pvert, &lightdir, &point);

									glVertex3f(point.x, point.y, point.z);
									glVertex3f(v->pvert.x, v->pvert.y, v->pvert.z);

									v = pb;
									vec_sub(&v->pvert, &LightPosition, &lightdir);
									vec_normalize(&lightdir, &lightdir);
									vec_scale(&lightdir, LargeNumber, &lightdir);
									vec_add(&v->pvert, &lightdir, &point);

									glVertex3f(v->pvert.x, v->pvert.y, v->pvert.z);
									glVertex3f(point.x, point.y, point.z);

									glEnd();
								}
							}
						}
					}
				}
			}
		}
	}
}


void drawCamera()
{
	// draw camera
	glPushMatrix();

	glTranslatef(cam->pos.x, cam->pos.y, cam->pos.z);
	glutSolidCube(10);

	glPopMatrix();
	glPushMatrix();

	glTranslatef(cam->target.x, cam->target.y, cam->target.z);
	glutSolidCube(5);

	glPopMatrix();
	glPushMatrix();

	glBegin(GL_LINES);
	glVertex3f(cam->pos.x, cam->pos.y, cam->pos.z);
	glVertex3f(cam->target.x, cam->target.y, cam->target.z);
	glEnd();
	
	glPopMatrix();
}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_width = glutGet(GLUT_WINDOW_WIDTH);
	g_height = glutGet(GLUT_WINDOW_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(cam ? cam->fov : 45.0f, (float)g_width / g_height, view ? 1.0f : znear, view ? 100000.0f : zfar);
	glScalef (1.0, -1.0, 1.0); // flip coords for 3ds

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (view==0 && cam)
	{
		float a = float(cam->roll * M_PI / 180.0f);
		gluLookAt(cam->pos.x, cam->pos.y, cam->pos.z, cam->target.x, cam->target.y, cam->target.z, sinf(a), -cosf(a), 0);
	
	} else
	{
		glTranslatef(ofsx, -ofsy, -zoom);
		glRotatef(rotx, 1, 0, 0);
		glRotatef(roty, 0, 1, 0);
		glRotatef(rotz, 0, 0, 1);
		glScalef (1.0, 1.0, 1.0);
	}


	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	if (mode==MODE_TEXTURED || mode==MODE_SOLID)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		GLfloat LightAmbient[]= { 0.5f, 0.5f, 0.5f, 1.0f };
		GLfloat LightDiffuse[]= { 0.5f, 0.5f, 0.5f, 1.0f }; 

		glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (mode == MODE_TEXTURED)
			glEnable(GL_TEXTURE_2D);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_DIFFUSE);

	} else 
	{
		//wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

#if(0)
		// antialias
		glEnable (GL_LINE_SMOOTH);
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
		glLineWidth (1.5);
#endif
	}


	if (g_stensil==0)
		drawScene();

	if (g_stensil==1)
	{
		drawScene();

		glDisable(GL_LIGHTING);

		// (taken from The Mechanics of Robust Stencil Shadows almost verbatim)
		// First, we must clear the stencil buffer, configure the stencil test so
		// that it always passes, and configure the depth test so that it passes
		// only when fragment depth values are less than those already in the
		// depth buffer. This is done using the following function calls.
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, ~0);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		// We are only going to be drawing into the stencil buffer, so we need to
		// disable writes to the color buffer and depth buffer as follows.
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		// Shadow volume faces are rendered using different stencil operations
		// depending on whether they face toward or away from the camera, so we
		// need to enable face culling with the following function call.
		glEnable(GL_CULL_FACE);

		// first pass, stencil operation decreases stencil value
		glCullFace(GL_BACK);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		DrawShadowVolumes();
	
		// second pass, stencil operation increases stencil value
		glCullFace(GL_FRONT);
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
		DrawShadowVolumes();

		// enable writing to the color and depth buffers
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);

		//draw a shadowing rectangle covering the entire screen
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glStencilFunc(GL_NOTEQUAL, 0, ~0);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
		glEnable(GL_BLEND);
		glPushMatrix();
		glLoadIdentity();
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(-1, 1,-1);
		glVertex3f(-1,-1,-1);
		glVertex3f( 1, 1,-1);
		glVertex3f( 1,-1,-1);
		glEnd();
		glPopMatrix();
		glDisable(GL_BLEND);

		// restore everything
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);
		glEnable(GL_LIGHTING);
		glDisable(GL_STENCIL_TEST);
		glShadeModel(GL_SMOOTH);
	}


	if (g_stensil==2)
	{
		drawScene();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);

		glColor4f(1.0f, 0.5f, 0.5f, 0.2f);
		glCullFace(GL_FRONT);
		DrawShadowVolumes();

		glColor4f(0.5f, 1.0f, 0.5f, 0.2f);
		glCullFace(GL_BACK);
		DrawShadowVolumes();

		glCullFace(GL_BACK);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
	}

	glutSwapBuffers();
}

void FLARE(int *texture)
{
	int i,j,dx,dy;	
	unsigned char c,k,rays[256];
	float r,a,v,rx,light,glow;
	for (i=0;i<256;i++)
		rays[i]=rand()%255;

	light=-0.02f;
	glow=-0.015f;

	for (i=0;i<256;i++)
		for (j=0;j<256;j++)
		{		
			dx=i-128; dy=j-128;
			r=sqrtf(float(dx*dx)+float(dy*dy));
			v=float((atan2f((float)dx,(float)dy)+M_PI)*128.0f/M_PI);
			a=exp(light*r*r)+exp(glow*r);	

			k=(unsigned char)v;
			v=(rays[k]+(rays[k+1]-rays[k])*(v-k));

			rx =fabs(r - 40)/2;	
			if(rx>0 && rx<=1) 
				a+=(1-rx*rx*(3-2*rx))/20;

			if (a>1) a=1;
			c = (unsigned char)(a*255.0f);
			texture[i*256+j]=c<<16|c<<8|c;
		}	
}

int load_scene (char * fname)
{
	int err = 0;

	clax_alloc_scene (&scene);

	err = clax_load_world(fname, scene);

	if (err != clax_err_ok)
	{
		printf("could not load world '%s': '%s'\n", fname, clax_geterror(err));
		return -1;
	}

	err = clax_load_motion(fname, scene);

	if (err != clax_err_ok)
	{
		printf("could not load motion: '%s'\n", clax_geterror(err));
	}

	clax_setactive_scene(scene);

	bool loadTextures = false;

	cam = NULL;
	numlights = 0;
	int objects = 0, vertices = 0, faces = 0, cameras = 0;
	for (w_NODE* node = scene->world; node; node = node->next)
	{
		c_OBJECT * obj;
		c_MATERIAL * mat;
		switch (node->type)
		{
			case clax_obj_material: mat = (c_MATERIAL *)node->object; mat->bump.file = mat->reflection.file = ""; if (mat->texture.file) loadTextures = true; break; //fixup .clx
			case clax_obj_camera: cameras++; cam = (c_CAMERA *)node->object; break;
			case clax_obj_object: obj = (c_OBJECT *)node->object; objects++; faces += obj->numfaces; vertices += obj->numverts; break;
			case clax_obj_light: lights[numlights++] = (c_LIGHT *)node->object; break;
			default: break;
		}
	}

	printf ("loaded '%s' (cam: %d, obj: %d, faces: %d, verts: %d)\n", fname, cameras, objects, faces, vertices);
	//clax_save_scene ("1.clx",scene);

	if (cam)
	{
		cam->sizeX = g_width*1.0f;
		cam->sizeY = g_height*1.0f;
		cam->aspectratio = 0.75f;
		clax_setactive_camera(cam);
		cam_update(cam);
	}

	clax_getframes(&frame, &frames);

	// disable pvert update, opengl does it in hardware
	clax_setactive_camera(0);

	frame = 0;

	if (adjacency)
		free(adjacency);

	adjacency = NULL;

	mode = MODE_TEXTURED;

	for (int i=0; i<textures; i++)
		free(texture[i]);

	textures = 0;

	for (;loadTextures;)
	{
		char buf[256];
		sprintf(buf, "tex%05d.jpg", textures);
		int w=0, h=0;
		int * data = (int*)LoadJPEG(buf, &w, &h);
		if (data)
		{
			printf ("loaded '%s'\n", buf);
			texture[textures++] = data;
		}
		else
			break;
	}

	if (textures>0)
	{
		texture[textures] = (int*)malloc(256*256*4);
		FLARE(texture[textures]);
		textures++;

	} else
	{
		mode = MODE_SOLID;
	}

	glGenTextures(textures, &TextureID[0]);

	for (int i=0; i<textures; i++)
	{
		glBindTexture(GL_TEXTURE_2D, TextureID[i]);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, texture[i]);
	}

	return 0;
}

int find_next_file(char *buffer, char *path, int step)
{
	char path_buffer[_MAX_PATH];
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	char filename[_MAX_FNAME];

	char next[_MAX_PATH];

	strcpy(filename, "");
	strcpy(next, "");

	if (path)
	{
		_fullpath(path_buffer, path, _MAX_PATH);
		_splitpath(path_buffer, drive, dir, fname, ext);
		_makepath(path_buffer, drive, dir, "", "");
		_makepath(filename, "", "", fname, ext);
	}
	else
		_getcwd(path_buffer, MAX_PATH);

	_chdir(path_buffer);

	struct _finddata_t c_file;
	int hFile;

	int j = -1;
	int i = 0;

	if ((hFile = _findfirst("*.3ds", &c_file)) == -1L)
		printf("No specified files in directory!\n");
	else
	{
		do
		{
			const char *flag = "";

			if (strcmp(filename, c_file.name) == 0)
			{
				j = i;
				flag = "> ";
			}

			if ((i == 0) || (step > 0 && i == j + step) || (step < 0 && j <= 0))
				strcpy(next, c_file.name);
			i++;

		}
		while (_findnext(hFile, &c_file) == 0);

		_findclose(hFile);
		_getcwd(path_buffer, MAX_PATH);	//remove slash if any

		strcat(path_buffer, "/");
		strcat(path_buffer, next);
		strcpy(next, path_buffer);
	}

	_splitpath(path_buffer, drive, dir, fname, ext);
	strcpy(buffer, fname);
	strcat(buffer, ext);
	return 0;
}

void load_next_scene()
{
	find_next_file(g_fname, g_fname, 1);
	load_scene(g_fname);
}

void load_prev_scene()
{
	find_next_file(g_fname, g_fname, -1);
	load_scene(g_fname);
}


#define CLAMP(t, dt, max) ( t = (t+dt>=max) ? 0 : ((t+dt<0) ? max+(t+dt) : t+dt) )

void SpecialInput(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_RIGHT: CLAMP(frame, (pause ? 1 : 8), frames); break;
	case GLUT_KEY_LEFT: CLAMP(frame, (pause ? -1 : -8), frames); break;
	case GLUT_KEY_UP: CLAMP(mode, 1, modes); break;
	case GLUT_KEY_DOWN: CLAMP(mode, -1, modes); break;
	case GLUT_KEY_PAGE_DOWN: load_next_scene(); break;
	case GLUT_KEY_PAGE_UP: load_prev_scene(); break;
	}
}


void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: exit(0);  break;
	case 'f': case 13: glutFullScreenToggle(); break;
	case 'r': resettransform(); break;
	case 'a': frame+=16; break;
	case 's': CLAMP(g_stensil, 1, 3); break;
	case ' ': pause = !pause; break;
	case 9: view = !view; break;
	}
}

void motion(int x, int y)
{
	int diffx = x - g_lastx;
	int diffy = y - g_lasty;

	g_lastx = x;
	g_lasty = y;

	if (Buttons[2])
	{
		rotx += (float)0.5f *diffy;
		rotz += (float)0.5f *diffx;
	}
	else if (Buttons[0])
	{
		ofsx += (float)5.5f *diffx;
		ofsy -= (float)5.5f *diffy;
	}

	glutPostRedisplay();
}

void wheel(int wheel, int direction, int x, int y)
{
	if (direction > 0)
		zoom = max(0.95f * zoom, 0.00001f);
	else
		zoom = min(1.05f * zoom, 100000.0f);

	glutPostRedisplay();
}


void mouse(int b, int s, int x, int y)
{
	g_lastx = x;
	g_lasty = y;
	switch (b)
	{
		case GLUT_LEFT_BUTTON: Buttons[0] = ((GLUT_DOWN == s) ? 1 : 0); break;
		case GLUT_MIDDLE_BUTTON: Buttons[1] = ((GLUT_DOWN == s) ? 1 : 0); break;
		case GLUT_RIGHT_BUTTON: Buttons[2] = ((GLUT_DOWN == s) ? 1 : 0); break;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	char buf[MAX_PATH];

	clax_init (clax_transform | clax_hierarchy | clax_domorph | clax_calcnorm);

	char * fname = 0;

	if (argc<2)
	{
		GetDocumentPath(buf, "res");
		SetCurrentDirectory(buf);
	} else
		fname = argv[1];

	find_next_file(g_fname, fname, 0);

	printf("Tab - switch cameras\n");
	printf("F/Enter - toggle fullscreen\n");
	printf("S - toggle shadows\n");
	printf("Left/Right - rewind scene\n");
	printf("Space - pause\n");
	printf("Up/Down - rendering mode\n");
	printf("PgUp/PgDn - next scene\n");

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - g_width) / 2, (glutGet(GLUT_SCREEN_HEIGHT) - g_height) / 2);
	glutInitWindowSize(g_width, g_height);

	glutCreateWindow(argv[0]);

	load_scene(g_fname);

	resettransform();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(SpecialInput);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
#ifdef FREEGLUT
	glutMouseWheelFunc(wheel);
#endif
	glutTimerFunc(timerrate, timer, 1);
	glutMainLoop();
	return 0;
}
