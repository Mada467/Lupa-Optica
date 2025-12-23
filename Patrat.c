#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

// Prototipuri functii
void myinit(void);
void CALLBACK display(void);
void CALLBACK myReshape(GLsizei w, GLsizei h);
void CALLBACK MutaStanga(void);
void CALLBACK MutaDreapta(void);
void CALLBACK MutaSus(void);
void CALLBACK MutaJos(void);

static GLfloat x = 0.0;
static GLfloat y = 0.0;
GLuint texturi[1];

// Incarcare textura folosind API-ul nativ Windows
void IncarcaTextura(const char* caleFisier, int indexTextura) {
    BITMAP bmp;
    HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, caleFisier, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBmp != NULL) {
        GetObject(hBmp, sizeof(bmp), &bmp);
        glGenTextures(1, &texturi[indexTextura]);
        glBindTexture(GL_TEXTURE_2D, texturi[indexTextura]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight,
            0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

        DeleteObject(hBmp);
        printf("Succes: Textura %s incarcata!\n", caleFisier);
    }
    else {
        printf("EROARE: Nu s-a putut deschide %s\n", caleFisier);
    }
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    // Incarcam textura "Dragon.bmp" din directorul proiectului
    IncarcaTextura("Dragon.bmp", 0);

    // Activam transparenta pentru lentila
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(x, y, -300.0);

  
    // 1. MANERUL (Cilindru cu textura)

    GLUquadricObj* maner = gluNewQuadric();
    gluQuadricTexture(maner, GL_TRUE); // Activam textura DOAR pentru maner

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturi[0]);
    glColor3f(1.0f, 1.0f, 1.0f);
    glTranslatef(0.0, -35.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    gluCylinder(maner, 5, 5, 50, 20, 20);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    gluDeleteQuadric(maner);

    
    // 2. CAPAT MANER 
    GLUquadricObj* capat = gluNewQuadric();
    glPushMatrix();
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(0.0, -85.0, 0.0);
    gluSphere(capat, 5, 15, 15);
    glPopMatrix();
    gluDeleteQuadric(capat);

    
    // 3. RAMA LENTILEI 
    GLUquadricObj* rama = gluNewQuadric();
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f);
    gluDisk(rama, 35, 40, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(rama);

    
    // 4. LENTILA 
   
    GLUquadricObj* lentila = gluNewQuadric();
    glPushMatrix();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f); // Transparent
    gluDisk(lentila, 0, 35, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(lentila);

    glFlush();
}

void CALLBACK MutaStanga(void) { x -= 10; }
void CALLBACK MutaDreapta(void) { x += 10; }
void CALLBACK MutaSus(void) { y += 10; }
void CALLBACK MutaJos(void) { y -= 10; }

void CALLBACK myReshape(GLsizei w, GLsizei h) {
    if (!h) return;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat)w / (GLfloat)h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    auxInitDisplayMode(AUX_SINGLE | AUX_RGBA | AUX_DEPTH);
    auxInitPosition(100, 100, 800, 600);
    auxInitWindow("Lupa - Cod Curat");

    myinit();

    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);

    auxReshapeFunc(myReshape);
    auxMainLoop(display);

    return 0;
}