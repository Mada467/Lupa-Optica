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
        printf("Succes: Textura BMP %s incarcata!\n", caleFisier);
    }
}

// --- INCARCARE TGA (Imagine Fundal) ---
GLbyte* gltLoadTGA(const char* szFileName, GLint* iWidth, GLint* iHeight, GLint* iComponents, GLenum* eFormat) {
    FILE* pFile;
    TGAHEADER tgaHeader;
    unsigned long lImageSize;
    short sDepth;
    GLbyte* pBits = NULL;

    pFile = fopen(szFileName, "rb");
    if (pFile == NULL) return NULL;

    fread(&tgaHeader, sizeof(TGAHEADER), 1, pFile);

    *iWidth = tgaHeader.width;
    *iHeight = tgaHeader.height;
    sDepth = tgaHeader.bits / 8;

    lImageSize = tgaHeader.width * tgaHeader.height * sDepth;
    pBits = (GLbyte*)malloc(lImageSize);
    if (pBits == NULL) { fclose(pFile); return NULL; }

    if (fread(pBits, lImageSize, 1, pFile) != 1) {
        free(pBits);
        fclose(pFile);
        return NULL;
    }

    switch (sDepth) {
    case 3: *eFormat = GL_BGR_EXT; *iComponents = GL_RGB8; break;
    case 4: *eFormat = GL_BGRA_EXT; *iComponents = GL_RGBA8; break;
    case 1: *eFormat = GL_LUMINANCE; *iComponents = GL_LUMINANCE8; break;
    }

    fclose(pFile);
    return pBits;
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    IncarcaTextura("Dragon.bmp", 0);

    // Încarcă imaginea TGA
    pImageTGA = gltLoadTGA("Fire.tga", &iWidthTGA, &iHeightTGA, &iComponentsTGA, &eFormatTGA);
    if (pImageTGA) printf("Succes: TGA incarcat (%dx%d)!\n", iWidthTGA, iHeightTGA);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- PASUL 1: DESENARE IMAGINE TGA (FUNDAL) ---
    if (pImageTGA != NULL) {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glRasterPos2i(0, 0);
        glDrawPixels(iWidthTGA, iHeightTGA, eFormatTGA, GL_UNSIGNED_BYTE, pImageTGA);

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    // --- PASUL 2: DESENARE LUPA 3D ---
    glLoadIdentity();
    glTranslatef(x, y, -300.0);

    // 1. MANERUL LUPEI (Cilindru cu textura)
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

    // 2. RAMA LUPEI (Disc gri)
    GLUquadricObj* ramaLupa = gluNewQuadric();
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f);
    gluDisk(ramaLupa, 35, 40, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(ramaLupa);

    // 3. LENTILA LUPEI (Disc semi-transparent)
    GLUquadricObj* lentilaLupa = gluNewQuadric();
    glPushMatrix();
    glColor4f(0.5f, 0.7f, 1.0f, 0.3f);
    gluDisk(lentilaLupa, 0, 35, 40, 1);
    glPopMatrix();
    gluDeleteQuadric(lentilaLupa);

    auxSwapBuffers();
}

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
    auxInitDisplayMode(AUX_DOUBLE | AUX_RGBA | AUX_DEPTH);
    auxInitPosition(100, 100, 800, 600);
    auxInitWindow("Lupa Finala - Nume Sugestive");
    myinit();
    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);
    auxReshapeFunc(myReshape);
    auxMainLoop(display);
    return 0;
}