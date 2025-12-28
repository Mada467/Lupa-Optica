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

// --- PROTOTIPURI FUNCTII ---
void myinit(void);
void CALLBACK display(void);
void CALLBACK myReshape(GLsizei w, GLsizei h);
void CALLBACK MutaStanga(void);
void CALLBACK MutaDreapta(void);
void CALLBACK MutaSus(void);
void CALLBACK MutaJos(void);

// --- VARIABILE GLOBALE ---
static GLfloat x = 0.0;
static GLfloat y = 0.0;
static GLfloat z = -300.0;
GLuint texturi[3];  // 0=maner, 1=ramă, 2=imagine fundal

// --- INCARCARE BMP ---
void IncarcaTextura(const char* caleFisier, int indexTextura) {
    BITMAP bmp;
    HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, caleFisier, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBmp != NULL) {
        GetObject(hBmp, sizeof(bmp), &bmp);
        glGenTextures(1, &texturi[indexTextura]);
        glBindTexture(GL_TEXTURE_2D, texturi[indexTextura]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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

    // Încărcăm toate texturile (BMP)
    IncarcaTextura("Lemn.bmp", 0);
    IncarcaTextura("Lemn.bmp", 1);
    IncarcaTextura("Saturno.bmp", 2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// Funcție pentru desenare cerc texturat (pentru zoom)
void DeseneazaCercTexturat(float centerX, float centerY, float radius, float texCenterX, float texCenterY, float texRadius) {
    int segments = 50;

    glBegin(GL_TRIANGLE_FAN);

    // Centrul cercului
    glTexCoord2f(texCenterX, texCenterY);
    glVertex2f(centerX, centerY);

    // Punctele pe circumferință
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float cx = centerX + radius * cos(angle);
        float cy = centerY + radius * sin(angle);

        float tx = texCenterX + texRadius * cos(angle);
        float ty = texCenterY + texRadius * sin(angle);

        glTexCoord2f(tx, ty);
        glVertex2f(cx, cy);
    }

    glEnd();
}

// Funcție pentru desenare textură fundal
void DeseneazaFundalTextura(float zoomFactor, float centerX, float centerY) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[2]);
    glColor3f(1.0f, 1.0f, 1.0f);

    if (zoomFactor == 1.0f) {
        // Fundal normal - întreaga imagine
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 600.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 600.0f);
        glEnd();
    }
    else {
        // Zoom - desenăm CERC exact cât lentila
        float lupaRadius = 90.0f;

        float texCenterX = centerX / 800.0f;
        float texCenterY = centerY / 600.0f;
        float texRadiusX = (lupaRadius / zoomFactor) / 800.0f;
        float texRadiusY = (lupaRadius / zoomFactor) / 600.0f;

        // Calculăm media pentru aspect ratio corect
        float texRadius = (texRadiusX + texRadiusY) / 2.0f;

        // Desenăm cercul texturat
        DeseneazaCercTexturat(centerX, centerY, lupaRadius,
            texCenterX, texCenterY, texRadius);
    }

    glDisable(GL_TEXTURE_2D);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // --- PASUL 1: DESENARE FUNDAL NORMAL ---
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    DeseneazaFundalTextura(1.0f, 0.0f, 0.0f);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // --- PASUL 2: CALCULARE POZITIE LUPA ---
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(x, y, z);

    GLdouble modelMatrix[16], projMatrix[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projMatrix);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble screenX, screenY, screenZ;
    gluProject(0, 0, 0, modelMatrix, projMatrix, viewport, &screenX, &screenY, &screenZ);

    // Calculăm zoom fix (fără variație pe Z)
    float zoomFactor = 2.0f;  // Zoom fix de 2x

    // --- PASUL 3: DESENARE ZOOM (CERC PERFECT) ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    DeseneazaFundalTextura(zoomFactor, screenX, screenY);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // --- PASUL 4: DESENARE LUPA ---
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(x, y, z);

    // MANER
    GLUquadricObj* manerLupa = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[0]);
    gluQuadricTexture(manerLupa, GL_TRUE);
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
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
    glPushMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    gluDisk(ramaLupa, 35, 40, 40, 1);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    gluDeleteQuadric(ramaLupa);

    // LENTILA
    GLUquadricObj* lentilaLupa = gluNewQuadric();
    glPushMatrix();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f);
    gluDisk(lentilaLupa, 0, 35, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(lentilaLupa);

    auxSwapBuffers();
}

// Funcții de control - DOAR deplasări
void CALLBACK MutaStanga(void) { x -= 10; }
void CALLBACK MutaDreapta(void) { x += 10; }
void CALLBACK MutaSus(void) { y += 10; }
void CALLBACK MutaJos(void) { y -= 10; }

void CALLBACK myReshape(GLsizei w, GLsizei h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA | AUX_DEPTH | AUX_STENCIL);
    auxInitPosition(100, 100, 800, 600);
    auxInitWindow("Lupa Optica - Proiect");
    myinit();

    // Control DOAR cu săgeți pentru deplasare
    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);

    auxReshapeFunc(myReshape);
    auxMainLoop(display);
    return 0;
}