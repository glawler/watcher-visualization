#include <GL/glu.h>
#include "skybox.h"
#include "bitmap.h"

using namespace watcher;
using namespace std;

INIT_LOGGER(Skybox, "Skybox"); 

Skybox &Skybox::getSkybox()  
{
    TRACE_ENTER();
    static Skybox theOneAndOnlySkyBox;
    TRACE_EXIT();
    return theOneAndOnlySkyBox;
}

Skybox::Skybox() : 
    width(100.0),
    textureIds(new unsigned int[6])
{
    TRACE_ENTER();
    TRACE_EXIT();
}

Skybox::~Skybox()
{
    TRACE_ENTER();
    delete [] textureIds;
    TRACE_EXIT();
}

void Skybox::drawSkybox(GLfloat camX, GLfloat camY, GLfloat camZ) 
{
    TRACE_ENTER();

    // Store the current matrix
    glPushMatrix();

    GLfloat x,y,z,width,height,length;
    width=100;
    height=100;
    length=100;
    x=camX-width/2;
    y=camY-height/2;
    z=camZ-length/2;

    // Enable/Disable features
    glPushAttrib(GL_ENABLE_BIT | GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_LIGHTING);
    // glDisable(GL_BLEND);

    // Draw Front side
    glBindTexture(GL_TEXTURE_2D, textureIds[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,       y,        z+length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,       y+height, z+length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z+length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,        z+length);
    glEnd();

    // Draw Back side
    glBindTexture(GL_TEXTURE_2D, textureIds[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,        z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,       y+height, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,       y,        z);
    glEnd();

    // Draw Left side
    glBindTexture(GL_TEXTURE_2D, textureIds[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,       y+height, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,       y+height, z+length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,       y,        z+length);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,       y,        z);
    glEnd();

    // Draw Right side
    glBindTexture(GL_TEXTURE_2D, textureIds[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y,        z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y,        z+length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y+height, z+length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y+height, z);
    glEnd();

    // Draw Up side
    glBindTexture(GL_TEXTURE_2D, textureIds[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x+width, y+height, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x+width, y+height, z+length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x,       y+height, z+length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x,       y+height, z);
    glEnd();

    // Draw Down side
    glBindTexture(GL_TEXTURE_2D, textureIds[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x,       y,        z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x,       y,        z+length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x+width, y,        z+length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x+width, y,        z);
    glEnd();

    // Restore enable bits and matrix
    glPopAttrib();
    glPopMatrix();

    TRACE_EXIT();
}

bool Skybox::loadSkyboxFiles()
{
    TRACE_ENTER();
    if (!loadBMPFile(0, "images/front.bmp")) return false;
    if (!loadBMPFile(1, "images/back.bmp")) return false;
    if (!loadBMPFile(2, "images/left.bmp")) return false;
    if (!loadBMPFile(4, "images/right.bmp")) return false;
    if (!loadBMPFile(5, "images/up.bmp")) return false;
    if (!loadBMPFile(5, "images/down.bmp")) return false;
    TRACE_EXIT();
    return true;
}

bool Skybox::loadBMPFile(const unsigned int textureId, const char *filename)
{
    TRACE_ENTER();
    BITMAPINFO *bmpInfo;

    GLubyte *imageData=LoadDIBitmap(filename, &bmpInfo);

    if (!imageData)
    {
        LOG_ERROR("Error loading skybox BMP image file " << filename);
        return false;
    }

    LOG_DEBUG("Successfully loaded BMP image data:");
    LOG_DEBUG("     w=" << bmpInfo->bmiHeader.biWidth << " h=" << bmpInfo->bmiHeader.biHeight);
    LOG_DEBUG("     size=" << bmpInfo->bmiHeader.biSizeImage << " depth=" << bmpInfo->bmiHeader.biBitCount);
    LOG_DEBUG("     pixels/meter: x=" << bmpInfo->bmiHeader.biXPelsPerMeter << " y=" << bmpInfo->bmiHeader.biYPelsPerMeter); 

    glGenTextures(1, &textureIds[textureId]);                                                                                 
    glBindTexture(GL_TEXTURE_2D, textureIds[textureId]);
    gluBuild2DMipmaps(GL_TEXTURE_2D,3,bmpInfo->bmiHeader.biWidth,bmpInfo->bmiHeader.biHeight,GL_RGB,GL_UNSIGNED_BYTE,imageData); 
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    free(bmpInfo);
    free(imageData); 

    TRACE_EXIT();
    return true;
}


