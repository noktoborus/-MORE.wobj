#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "wavefront.h"

#define WVOBJFILE "rez/model00.obj"
#define CHUNK_SZ 1024

GLfloat colors[] = {1,1,1,  1,1,0,  1,0,0,  1,0,1,              // v0-v1-v2-v3
                    1,1,1,  1,0,1,  0,0,1,  0,1,1,              // v0-v3-v4-v5
                    1,1,1,  0,1,1,  0,1,0,  1,1,0,              // v0-v5-v6-v1
                    1,1,0,  0,1,0,  0,0,0,  1,0,0,              // v1-v6-v7-v2
                    0,0,0,  0,0,1,  1,0,1,  1,0,0,              // v7-v4-v3-v2
                    0,0,1,  0,0,0,  0,1,0,  0,1,1};             // v4-v7-v6-v5

// vertex coords array
GLfloat vertices[] = {1,1,1,  -1,1,1,  -1,-1,1,  1,-1,1,        // v0-v1-v2-v3
                      1,1,1,  1,-1,1,  1,-1,-1,  1,1,-1,        // v0-v3-v4-v5
                      1,1,1,  1,1,-1,  -1,1,-1,  -1,1,1,        // v0-v5-v6-v1
                      -1,1,1,  -1,1,-1,  -1,-1,-1,  -1,-1,1,    // v1-v6-v7-v2
                      -1,-1,-1,  1,-1,-1,  1,-1,1,  -1,-1,1,    // v7-v4-v3-v2
                      1,-1,-1,  -1,-1,-1,  -1,1,-1,  1,1,-1};   // v4-v7-v6-v5



GLfloat LPOS[] = {0.0f, 0.0f, 20.0f, 1.0f};
GLfloat LAMB[] = {0.2f, 0.2f, 0.2f, 1.0f};
GLfloat LDIF[] = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat LSPE[] = {1.0f, 1.0f, 1.0f, 1.0f};
struct model_t *model;

void
display ()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix ();
		glTranslatef (0.0f, 0.0f, -7.0f);
		glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
		// cube 1
		glPushMatrix ();
			glTranslatef (2.0f, 0.0f, 0.0f);

			glEnableClientState (GL_VERTEX_ARRAY);
			glEnableClientState (GL_COLOR_ARRAY);

			glColorPointer (3, GL_FLOAT, 0, colors);
			glVertexPointer (3, GL_FLOAT, 0, model->pollys[0]->vertex[0]);
			glDrawArrays (GL_QUADS, 0, 24);

			glDisableClientState (GL_VERTEX_ARRAY);
			glDisableClientState (GL_COLOR_ARRAY);
		glPopMatrix ();
		// cube 2
		glPushMatrix ();
			glTranslatef (-2.0f, 0.0f, 0.0f);
			glEnableClientState (GL_VERTEX_ARRAY);
			glEnableClientState (GL_COLOR_ARRAY);

			glColorPointer (3, GL_FLOAT, 0, colors);
			glVertexPointer (3, GL_FLOAT, 0, vertices);
			glDrawArrays (GL_QUADS, 0, 24);

			glDisableClientState (GL_COLOR_ARRAY);
			glDisableClientState (GL_VERTEX_ARRAY);
		glPopMatrix ();
	glPopMatrix ();
	glutSwapBuffers ();
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
motion (int x, int y)
{

}

void
mouse (int button, int state, int x, int y)
{
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
		printf ("# COMPLETE %p\n", (void*)model);
	}
	if (model)
	{
		printf ("# MODEL %p\n", (void*)model);
		re = 0;
		if (model->pollys_num)
		{
			do
			{
				printf ("# POLLY #%d polygons count: %d with vertexes: %d\n",\
					   	re, model->pollys[re]->num, model->pollys[re]->len);
				printf ("SZ: %d\n## ", sz);
				for (sz = 0; sz < model->pollys[re]->num * model->pollys[re]->len * 3; sz++)
				{
					if (!(sz % (model->pollys[re]->len * 3))) printf ("\n");
					if (!(sz % 3)) printf ("| ");
					printf ("%2.1f ", model->pollys[re]->vertex[0][sz]);
				}
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
		glutTimerFunc (10, timerP, 10);
		// GL init
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth (1.0f);
		glEnable (GL_LIGHTING);
		glEnable (GL_LIGHT0);
		glLightfv (GL_LIGHT0, GL_AMBIENT, LAMB);
		glLightfv (GL_LIGHT0, GL_SPECULAR, LSPE);
		glLightfv (GL_LIGHT0, GL_DIFFUSE, LDIF);
		glLightfv (GL_LIGHT0, GL_POSITION, LPOS);
		glEnable (GL_COLOR_MATERIAL);
		glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glShadeModel (GL_SMOOTH);
		// run
		glutMainLoop ();
	}
	return 0;
}

