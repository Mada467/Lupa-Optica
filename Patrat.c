#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

#ifndef M_PI
#define M_PI 3.14
#endif


static GLfloat x = 0.0;
static GLfloat y = 0.0;
static GLfloat z = -300.0;


static GLfloat zoomFactor = 3.0f;       
static GLfloat factorConversie = 3.38f;

GLuint texturi[3]; // 0=maner, 1=rama, 2=imagine fundal

// --- INCARCARE TEXTURI ---
void IncarcaTextura(const char* caleFisier, int indexTextura) {
    BITMAP bmp;
    HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, caleFisier, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBmp != NULL) {
        GetObject(hBmp, sizeof(bmp), &bmp);
        glGenTextures(1, &texturi[indexTextura]);
        glBindTexture(GL_TEXTURE_2D, texturi[indexTextura]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        // REGLAJ MARGINI: Folosim REPEAT pentru a elimina liniile de taiere
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight,
            0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
        DeleteObject(hBmp);
    }
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    IncarcaTextura("Lemn.bmp", 0);
    IncarcaTextura("Lemn.bmp", 1);
    IncarcaTextura("Saturno.bmp", 2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DeseneazaFundalSimplu() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[2]);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 600.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 600.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void DeseneazaFundalPentruZoom() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[2]);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    // Coordonate extinse pentru a acoperi marginile in timpul zoom-ului
    glTexCoord2f(-1.0f, -1.0f); glVertex2f(-800.0f, -600.0f);
    glTexCoord2f(2.0f, -1.0f); glVertex2f(1600.0f, -600.0f);
    glTexCoord2f(2.0f, 2.0f); glVertex2f(1600.0f, 1200.0f);
    glTexCoord2f(-1.0f, 2.0f); glVertex2f(-800.0f, 1200.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    // --- PASUL 1: FUNDAL BAZA ---
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    DeseneazaFundalSimplu();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();

    // --- PASUL 2: MASCA STENCIL ---
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glPushMatrix();
    glTranslatef(x, y, z);
    GLUquadricObj* masca = gluNewQuadric();
    gluDisk(masca, 0, 35, 45, 1);
    gluDeleteQuadric(masca);
    glPopMatrix();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // --- PASUL 3: ZOOM 
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    // Calculam pozitia pe ecran folosind variabila de sus
    float lupaEcranX = 400.0f + (x * factorConversie);
    float lupaEcranY = 300.0f + (y * factorConversie);

    glTranslatef(lupaEcranX, lupaEcranY, 0);
    glScalef(zoomFactor, zoomFactor, 1.0f); // Zoom variabil de sus
    glTranslatef(-lupaEcranX, -lupaEcranY, 0);

    DeseneazaFundalPentruZoom();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_STENCIL_TEST);

    // --- PASUL 4: OBIECT LUPA 3D ---
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glTranslatef(x, y, z);

    // MANER
    GLUquadricObj* manerLupa = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[0]);
    gluQuadricTexture(manerLupa, GL_TRUE);
    glPushMatrix();
    glTranslatef(0.0, -35.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    gluCylinder(manerLupa, 5, 5, 50, 20, 20);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    gluDeleteQuadric(manerLupa);

    // RAMA
    GLUquadricObj* ramaLupa = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[1]);
    gluQuadricTexture(ramaLupa, GL_TRUE);
    gluDisk(ramaLupa, 35, 40, 45, 1);
    glDisable(GL_TEXTURE_2D);
    gluDeleteQuadric(ramaLupa);

    // LENTILA
    GLUquadricObj* lentilaLupa = gluNewQuadric();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f);
    gluDisk(lentilaLupa, 0, 35, 45, 1);
    gluDeleteQuadric(lentilaLupa);

    auxSwapBuffers();
}

void CALLBACK MutaStanga(void) { x -= 5; }
void CALLBACK MutaDreapta(void) { x += 5; }
void CALLBACK MutaSus(void) { y += 5; }
void CALLBACK MutaJos(void) { y -= 5; }

void CALLBACK myReshape(GLsizei w, GLsizei h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
    auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA | AUX_DEPTH | AUX_STENCIL);
    auxInitPosition(100, 100, 800, 600);
    auxInitWindow("Proiect Lupa ");
    myinit();

    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);

    auxReshapeFunc(myReshape);
    auxMainLoop(display);
    return 0;
}