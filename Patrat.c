#include "glos.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

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

GLUquadricObj* quadric;


void IncarcaTextura(const char* caleFisier, int indexTextura) {
    AUX_RGBImageRec* pImagine;
    pImagine = auxDIBImageLoad(caleFisier);

    if (pImagine != NULL) {
        glBindTexture(GL_TEXTURE_2D, texturi[indexTextura]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, 3, pImagine->sizeX, pImagine->sizeY,
            0, GL_RGB, GL_UNSIGNED_BYTE, pImagine->data);
        if (pImagine->data) free(pImagine->data);
        free(pImagine);
    }
    else {
        printf("EROARE: Nu am gasit fisierul: %s\n", caleFisier);
    }
}

void myinit(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, texturi);

    // Incarcare textura maner
    IncarcaTextura("C:\\Users\\madal\\Desktop\\Dragon.bmp", 0);

    // Blending pentru transparenta
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
}

void CALLBACK display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glBindTexture(GL_TEXTURE_2D, texturi[0]);

   
    glTranslatef(x, y, -300.0);

    // 1. MANERUL
    glPushMatrix();
    glColor3f(0.9f, 0.25f, 0.0f);
    glTranslatef(0.0, -35.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    gluCylinder(quadric, 5, 5, 50, 20, 20);

    // Capat 
    glColor3f(0.7f, 0.7f, 0.7f);
    glTranslatef(0.0, 0.0, 50.0);
    gluSphere(quadric, 5, 15, 15);
    glPopMatrix();

    //  2. RAMA 
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f);
    gluDisk(quadric, 35, 40, 40, 1);
    glPopMatrix();

    //  3. LENTILA 
    glPushMatrix();
    glColor4f(0.5f, 0.7f, 1.0f, 0.2f);
    gluDisk(quadric, 0, 35, 40, 1);
    glPopMatrix();

    glFlush();
}

void CALLBACK MutaStanga(void)
{
    x = x - 10;
}
void CALLBACK MutaDreapta(void)
{
    x = x + 10;
}
void CALLBACK MutaSus(void)
{
    y = y + 10;
}
void CALLBACK MutaJos(void)
{
    y = y - 10;
}

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
    auxInitWindow("Lupa pentru Zoom");

    myinit();

    auxKeyFunc(AUX_LEFT, MutaStanga);
    auxKeyFunc(AUX_RIGHT, MutaDreapta);
    auxKeyFunc(AUX_UP, MutaSus);
    auxKeyFunc(AUX_DOWN, MutaJos);

    auxReshapeFunc(myReshape);
    auxMainLoop(display);

    return 0;
    //test
    //test1
}