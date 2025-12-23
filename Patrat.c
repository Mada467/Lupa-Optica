#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

// --- STRUCTURA TGA HEADER ---
#pragma pack(1)
typedef struct {
    GLbyte  identsize;
    GLbyte  colorMapType;
    GLbyte  imageType;
    unsigned short colorMapStart;
    unsigned short colorMapLength;
    unsigned char  colorMapBits;
    unsigned short xstart;
    unsigned short ystart;
    unsigned short width;
    unsigned short height;
    GLbyte  bits;
    GLbyte  descriptor;
} TGAHEADER;
#pragma pack(8)

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
GLuint texturiBMP[1];
GLbyte* pImageTGA = NULL;
GLint iWidthTGA, iHeightTGA, iComponentsTGA;
GLenum eFormatTGA;

// --- INCARCARE BMP (Textura Maner) ---
void IncarcaTextura(const char* caleFisier, int indexTextura) {
    BITMAP bmp;
    HBITMAP hBmp = (HBITMAP)LoadImageA(NULL, caleFisier, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);

    if (hBmp != NULL) {
        GetObject(hBmp, sizeof(bmp), &bmp);
        glGenTextures(1, &texturiBMP[indexTextura]);
        glBindTexture(GL_TEXTURE_2D, texturiBMP[indexTextura]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight,
            0, GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);
        DeleteObject(hBmp);
    }
}

// --- INCARCARE TGA (Imagine Fundal) ---
GLbyte* gltLoadTGA(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat) {
    FILE* pFile = fopen(szFileName, "rb");
    if (pFile == NULL) return NULL;
    TGAHEADER tgaHeader;
    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);
    *iWidth = tgaHeader.width;
    *iHeight = tgaHeader.height;
    short sDepth = tgaHeader.bits / 8;
    unsigned long lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
    GLbyte* pBits = (GLbyte*)malloc(lImageSize);
    if (pBits == NULL) { fclose(pFile); return NULL; }
    fread(pBits, lImageSize, 1, pFile);
    switch (sDepth) {
    case 3: *eFormat = GL_BGR_EXT; *iComponents = GL_RGB8; break;
    case 4: *eFormat = GL_BGRA_EXT; *iComponents = GL_RGBA8; break;
    }
    fclose(pFile);
    return pBits;
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST); // ACTIVARE TEST SABLON

    IncarcaTextura("Dragon.bmp", 0);
    pImageTGA = gltLoadTGA("Fire.tga", &iWidthTGA, &iHeightTGA, &iComponentsTGA, &eFormatTGA);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CALLBACK display(void) {
    // Resetam si buffer-ul Stencil (SABLON)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();

    // --- 1. DESENARE FUNDAL NORMAL (STARE INITIALA) ---
    glDisable(GL_STENCIL_TEST); // Nu avem nevoie de masca pentru fundalul de baza
    if (pImageTGA != NULL) {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelZoom(1.0, 1.0); // Scara normala
        glRasterPos2i(0, 0);
        glDrawPixels(iWidthTGA, iHeightTGA, eFormatTGA, GL_UNSIGNED_BYTE, pImageTGA);
        glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    // --- 2. CREARE MASCA (SABLON) PRIN LENTILA ---
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF); // Tot ce desenam acum va pune valoarea 1 in buffer
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    // Nu vrem sa coloram ecranul inca, doar sa definim zona
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    glLoadIdentity();
    glTranslatef(x, y, -300.0);
    GLUquadricObj* mascaLentila = gluNewQuadric();
    gluDisk(mascaLentila, 0, 35, 40, 1); // Aceasta este "gaura" prin care vom vedea marirea
    gluDeleteQuadric(mascaLentila);

    // Reactivam scrierea culorilor
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    // --- 3. DESENARE IMAGINE MARITA IN INTERIORUL MASTII ---
    glStencilFunc(GL_EQUAL, 1, 0xFF); // Deseneaza doar unde stencil este 1
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Nu mai modifica masca

    if (pImageTGA != NULL) {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();

        glPixelZoom(2.0, 2.0); // ZOOM 2X

        // Aliniere dinamica: imaginea marita trebuie sa se miste invers fata de lupa pentru a parea fixa
        glRasterPos2f(0.0f - (x / 1.0f), 0.0f - (y / 1.0f));

        glDrawPixels(iWidthTGA, iHeightTGA, eFormatTGA, GL_UNSIGNED_BYTE, pImageTGA);
        glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    // --- 4. DESENARE COMPONENTE LUPA (MANER SI RAMA) ---
    glDisable(GL_STENCIL_TEST); // Oprim masca pentru a vedea manerul si rama peste tot
    glLoadIdentity();
    glTranslatef(x, y, -300.0);

    // MANER
    GLUquadricObj* manerLupa = gluNewQuadric();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturiBMP[0]);
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
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f);
    gluDisk(ramaLupa, 35, 40, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(ramaLupa);

    // LENTILA (foarte transparenta pentru efect de sticla)
    GLUquadricObj* lentilaLupa = gluNewQuadric();
    glPushMatrix();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f);
    gluDisk(lentilaLupa, 0, 35, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(lentilaLupa);

    auxSwapBuffers();
}

// Controlul ramane identic
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
    // ATENTIE: Am adaugat AUX_STENCIL aici!
    auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA | AUX_DEPTH | AUX_STENCIL);
    auxInitPosition(100, 100, 800, 600);
    auxInitWindow("Lupa Pasul 4 - Stencil Zoom");
    myinit();
    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);
    auxReshapeFunc(myReshape);
    auxMainLoop(display);
    return 0;
} 