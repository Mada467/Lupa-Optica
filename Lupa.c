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

// --- VARIABILE GLOBALE ---
static GLfloat x = 0.0;
static GLfloat y = 0.0;
static GLfloat z = -300.0;

static GLfloat zoomFactor = 2.0f;

// FIX 1: Factorul corect pentru Z=-300 si FOV=45 este ~2.414
static GLfloat factorConversie = 2.414f;

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

        // FIX 2: Folosim GL_CLAMP pentru a NU repeta marginile imaginii la zoom
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight,
            0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
        DeleteObject(hBmp);
    }
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    // Incarcam texturile (asigura-te ca fisierele BMP sunt langa exe)
    IncarcaTextura("TexturaLupa.bmp", 0);
    IncarcaTextura("TexturaLupa.bmp", 1);
    IncarcaTextura("TrackListCosaNuestra.bmp", 2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void DeseneazaFundalSimplu() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[2]);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    // Coordonate standard 0-1 pentru textura, 0-800/600 pentru ecran
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 600.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 600.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    // --- PASUL 1: FUNDAL BAZA (Imaginea normala) ---
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    DeseneazaFundalSimplu();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();


    // --- PASUL 2: MASCA STENCIL (Forma rotunda a lentilei) ---
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // Nu desenam nimic vizibil, doar in stencil

    glPushMatrix();
    glTranslatef(x, y, z);
    GLUquadricObj* masca = gluNewQuadric();
    gluDisk(masca, 0, 35, 45, 1); // Discul interior
    gluDeleteQuadric(masca);
    glPopMatrix();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Reactivam culorile


    // --- PASUL 3: ZOOM (Desenam fundalul din nou, dar scalat, doar unde e masca) ---
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();

    // Calculam centrul lupei pe ecranul 2D
    // Adaugam 400/300 pentru ca (0,0,z) e centrul ecranului
    float lupaEcranX = 400.0f + (x * factorConversie);
    float lupaEcranY = 300.0f + (y * factorConversie);

    // Mutam in centrul lupei -> Scalam -> Mutam inapoi
    glTranslatef(lupaEcranX, lupaEcranY, 0);
    glScalef(zoomFactor, zoomFactor, 1.0f);
    glTranslatef(-lupaEcranX, -lupaEcranY, 0);

    // Desenam acelasi fundal standard. De restul se ocupa glScalef si Stencil-ul
    DeseneazaFundalSimplu();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_STENCIL_TEST);


    // --- PASUL 4: OBIECT LUPA 3D (Maner, Rama, Lentila sticla) ---
    // Aceasta parte a ramas exact cum ai cerut
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

    // LENTILA (Sticla transparenta)
    GLUquadricObj* lentilaLupa = gluNewQuadric();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f); // Albastrui transparent
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
    auxInitWindow("Proiect Lupa Final");
    myinit();

    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);

    auxReshapeFunc(myReshape);
    auxMainLoop(display);
    return 0;
}