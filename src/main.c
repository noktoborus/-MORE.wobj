#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <math.h>
#define GL_PI 3.1415f

#include "wavefront.h"


#if 1
#	define WVOBJFILE "rez/model04.obj"
#else
#	define WVOBJFILE "rez/cube.obj"
#endif

#define CHUNK_SZ 1024

GLfloat LPOS[] = {-20.0f, 20.0f, 20.0f, 1.0f};
GLfloat LAMB[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat LDIF[] = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat LSPE[] = {1.0f, 1.0f, 1.0f, 1.0f};
struct model_t *model;
GLfloat TZet = -20.0f;
GLfloat angX = 0.0f;
GLfloat angY = 0.0f;
GLfloat angZ = 0.0f;

int mmotion_X = 0;
int mmotion_Y = 0;

void
display ()
{
	GLenum error;

	size_t pollyn;
	size_t t;
	GLenum type;
	pollyn = 0;
	t = 0;
	type = 0;
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix ();
		glTranslatef (0.0f, 0.0f, TZet);
		glLightfv (GL_LIGHT0, GL_POSITION, LPOS);
		glPushMatrix ();
			glTranslatef (LPOS[0], LPOS[1], LPOS[2]);
			glDisable (GL_LIGHTING);
			glColor3f (1.0f, 1.0f, 1.0f);
			glutSolidSphere (1, 15, 15);
			glEnable (GL_LIGHTING);
		glPopMatrix ();
#if 1 
		glPushMatrix ();
			//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
			glRotatef (angX, 1.0f, 0.0f, 0.0f);
			glRotatef (angY, 0.0f, 1.0f, 0.0f);
			glRotatef (angZ, 0.0f, 0.0f, 1.0f);
			glPushMatrix ();
				glTranslatef (5.0f, 0.0f, 0.0f);
				glutSolidTeapot (2);
			glPopMatrix ();
			glScalef (2.0f, 2.0f, 2.0f);
	//		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	//		glutSolidSphere (2, 15, 15);
			glEnableClientState (GL_VERTEX_ARRAY);
			glEnableClientState (GL_NORMAL_ARRAY);
			pollyn = model->pollys_num;
			while (pollyn--)
			{
				switch (model->pollys[pollyn]->len)
				{
					case 1:
						type = GL_POINTS;
						break;
					case 2:
						type = GL_LINES;
						break;
					case 3:
						type = GL_TRIANGLES;
						break;
					case 4:
						type = GL_QUADS;
						break;
					default:
						type = GL_POLYGON;
				}
				//glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
//						glColor3f (1.0f, 1.0f, 1.0f);
				glVertexPointer (3, GL_FLOAT, 0, model->pollys[pollyn]->vertex[0]);
				if (model->pollys[pollyn]->use & MPOLLY_USE_NORMAL)
				{
					glNormalPointer (GL_FLOAT, 0, model->pollys[pollyn]->vertex[1]);
				}
				if (type == GL_POLYGON)
				{
					for (t = 0; t < model->pollys[pollyn]->num; t++)
					{
						glDrawArrays (type, t * model->pollys[pollyn]->len,\
								model->pollys[pollyn]->len);
					}
				}
				else
				{
					//build ();
					glDrawArrays (type, 0,\
						   model->pollys[pollyn]->num * model->pollys[pollyn]->len);
				}
			}
			glDisableClientState (GL_VERTEX_ARRAY);
			glDisableClientState (GL_NORMAL_ARRAY);
		glPopMatrix ();
#endif
	glPopMatrix ();
	glutSwapBuffers ();
	if ((error = glGetError ()))
		printf ("ERROR: OpenGL: %d, %s\n", error, gluErrorString (error));
}

void
resize (int x, int y)
{
	GLdouble asp;
	asp = (GLdouble)x / (GLdouble)y;
	glViewport (0, 0, x, y);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective (60.0f, asp, 1.0f, 600.0f);
	//glOrtho (-2.0f, 2.0f, -2.0f, 2.0f, 2.0f, -2.0f);

	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

void
timerP (int val)
{
	glutPostRedisplay ();
	glutTimerFunc (val, timerP, val);
}

void
key (unsigned char key, int x, int y)
{
	if (key == 'q') glEnable (GL_CULL_FACE);
	if (key == 'w') glDisable (GL_CULL_FACE);
	if (key == 'e') glShadeModel (GL_FLAT);
	if (key == 'r') glShadeModel (GL_SMOOTH);
	if (key == 'l') angY += 5;
	if (key == 'j') angY -= 5;
	if (key == 'i') angX += 5;
	if (key == 'k') angX -= 5;
	if (key == 'a') angZ += 5;
	if (key == 'd') angZ -= 5;
	if (angX > 360.0f) angX = 0.0f;
	if (angX < 0.0f) angX = 360.0f;
	if (angY > 360.0f) angY = 0.0f;
	if (angY < 0.0f) angY = 360.0f;
	if (angZ > 360.0f) angZ = 0.0f;
	if (angZ < 0.0f) angZ = 360.0f;
}

void
motion (int x, int y)
{
	if (mmotion_X)
	{
		if (mmotion_X > x) angY += 1.0f;
		else
		if (mmotion_X < x) angY -= 1.0f;
		mmotion_X = x;
	}
	if (mmotion_Y)
	{
		if (mmotion_Y > y) angX += 0.5f;
		else
		if (mmotion_Y < y) angX -= 0.5f;
		mmotion_Y = y;
	}
	if (angX > 360.0f) angX = 0.0f;
	if (angX < 0.0f) angX = 360.0f;
	if (angY > 360.0f) angY = 0.0f;
	if (angY < 0.0f) angY = 360.0f;
}

void
mouse (int button, int state, int x, int y)
{
	//printf ("M -> button: %d, state: %d\n", button, state);
	// buttons:
	// 	left: 0
	// 	center: 1
	// 	right: 2
	// 	wheelup: 3
	// 	wheeldown: 4
	// state:
	// 	down: 0
	// 	up: 1
	if (state == 0) // down
	{
		switch (button)
		{
			case 0:
				mmotion_X = x;
				mmotion_Y = y;
				break;
			case 3:
			   	TZet += 1;
				break;
			case 4:
			   	TZet -= 1;
				break;
		}
	}
	if (state == 1) // up
	{
		switch (button)
		{
			case 0:
				mmotion_X = 0;
				mmotion_Y = 0;
				break;
		}
	}
}

int
main (int argc, char *argv[])
{
	char *buf = NULL;
	void *tmp = NULL;
	FILE *f = NULL;
	char chunk[CHUNK_SZ];
	size_t re = 0;
	size_t sz = 0;
	size_t c = 0;
	//size_t tt = 0;
	struct wvfo_parser_t wvps;
	if ((f = fopen (WVOBJFILE, "r")))
	{
		while ((re = fread (chunk, sizeof (char), CHUNK_SZ, f)))
		{
			if (!(tmp = realloc (buf, (sz + re) + sizeof (char))))
			{
				free (buf);
				buf = NULL;
				break;
			}
			buf = tmp;
			memcpy ((void*)&(buf[sz]), (const void*)chunk, re);
			sz += re;
		}
		fclose (f);
	}
	if (buf)
	{
		printf ("# LOAD: %p %u\n", buf, sz);
		wvfo_zero (&wvps);
		model = wvfo_load (&wvps, buf, sz);
		printf ("# COMPLETE %p (%s -> %d:%d)\n", (void*)model, wvfo_error_str (wvfo_error (&wvps)),
				wvfo_state_row (&wvps), wvfo_state_col (&wvps));
	}
	if (model)
	{
		printf ("# MODEL %p\n", (void*)model);
		re = 0;
		if (model->pollys_num)
		{
			do
			{
				printf ("# POLLY #%d faces count: %d with vertexes: %d\n",\
					   	re, model->pollys[re]->num, model->pollys[re]->len);
				printf ("## OPT: ");
				if (model->pollys[re]->use & MPOLLY_USE_VERTEX)
				{
					c++;
					printf ("USE_VERTEX ");
				}
				if (model->pollys[re]->use & MPOLLY_USE_TEXTUR)
				{
					c++;
					printf ("USE_TEXTUR ");
				}
				if (model->pollys[re]->use & MPOLLY_USE_NORMAL)
				{
					c++;
					printf ("USE_NORMAL ");
				}
				printf ("\n");
				while (c--)
				{
					printf ("\n# %d\n", c);
					for (sz = 0; sz < model->pollys[re]->num * model->pollys[re]->len * 3; sz++)
					{
						if (!(sz % (model->pollys[re]->len * 3))) printf ("\n");
						if (!(sz % 3)) printf ("| ");
						printf ("%2.1f ", model->pollys[re]->vertex[c][sz]);
					}
				}
				printf ("\nSZ: %d\n## ", sz);
				printf ("\n##\n");
		/*		if (model->pollys[re]->num && model->pollys[re]->len)
				{
					//printf ("%2.1f %2.1f %2.1f\n", model->pollys[re]->vertex[0][0],
					//		model->pollys[re]->vertex[0][1],
					//		model->pollys[re]->vertex[0][2]);
					for (sz = 0; sz < model->pollys[re]->num; sz++)
					{
						printf ("# f(%d) ", sz);
						for (tt = 0; tt < model->pollys[re]->len; tt++)
						{
							printf ("%d-%d", sz * 3* model->pollys[re]->len + tt * 3, tt); 
							printf ("(%2.1f,", MPOLLY_VEX (model->pollys[re], 0, sz, tt, 0)); 
							printf (" %2.1f,", MPOLLY_VEX (model->pollys[re], 0, sz, tt, 1));
							printf (" %2.1f)", MPOLLY_VEX (model->pollys[re], 0, sz, tt, 2));
							printf (" ");
						}
						printf ("\n");
					}
				}
		*/
			}
			while (++re < model->pollys_num);
		}
		// GLut init
		printf ("glut Init\n");
		glutInit (&argc, argv);
		glutInitWindowSize (600, 600);
		glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
		glutCreateWindow ("A");
		glutDisplayFunc (display);
		glutReshapeFunc (resize);
		glutMotionFunc (motion);
		glutMouseFunc (mouse);
		glutKeyboardFunc (key);

		glutTimerFunc (10, timerP, 10);
		// GL init
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth (1.0f);
		glEnable (GL_LIGHTING);
		//glLightfv (GL_LIGHT0, GL_AMBIENT, LAMB);
		//glLightfv (GL_LIGHT0, GL_SPECULAR, LSPE);
		//glLightfv (GL_LIGHT0, GL_DIFFUSE, LDIF);
		glEnable (GL_LIGHT0);
		glEnable (GL_COLOR_MATERIAL);
		glEnable (GL_DEPTH_TEST);
		glEnable (GL_NORMALIZE);
		//glEnable (GL_CULL_FACE);
		glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glShadeModel (GL_SMOOTH);
		//glShadeModel (GL_FLAT);
		// run
		glutMainLoop ();
	}
	return 0;
}

