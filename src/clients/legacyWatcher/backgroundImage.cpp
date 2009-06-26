#include <GL/gl.h>
#include <GL/glu.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "backgroundImage.h"
#include "bitmap.h"

using namespace watcher;

INIT_LOGGER(BackgroundImage, "BackgroundImage"); 

BackgroundImage &BackgroundImage::getInstance()
{
    TRACE_ENTER();
    static BackgroundImage theoneandonlybgimageinstanceyoubetcha;
    TRACE_EXIT();
    return theoneandonlybgimageinstanceyoubetcha;
}

BackgroundImage::BackgroundImage() :
    imageLoaded(false),
    imageCenter(false),
    imageData(NULL),
    minx(0.0),
    miny(0.0),
    xoffset(350.0),    // pulled outta thin air
    yoffset(400.0),    // pulled outta thin air
    z(0.0),
    imageWidth(0),
    imageHeight(0),
    imageFormat(GL_BITMAP),
    imageType(GL_UNSIGNED_BYTE)
{
    TRACE_ENTER();

    envColor[0]=0.0; 
    envColor[1]=0.0; 
    envColor[2]=0.0; 
    envColor[3]=0.0; 

    borderColor[0]=0.0; 
    borderColor[1]=0.0; 
    borderColor[2]=0.0; 
    borderColor[3]=0.0; 

    TRACE_EXIT();
}

BackgroundImage::~BackgroundImage()
{
    TRACE_ENTER();

    if (imageData)
        free(imageData);
    imageData=NULL; 

    TRACE_EXIT();
}

bool BackgroundImage::loadBMPFile(const char *filename)
{
    TRACE_ENTER();
    BITMAPINFO *bmpInfo;

    if (imageData)
        free(imageData);

    imageData=LoadDIBitmap(filename, &bmpInfo);

    if (!imageData)
    {
        LOG_ERROR("Error loading BMP image file " << filename);
        return false;
    }

    imageWidth=bmpInfo->bmiHeader.biWidth;
    imageHeight=bmpInfo->bmiHeader.biHeight;

    // default to using image w and h
    xoffset=imageWidth;
    yoffset=imageHeight;

    imageFormat=GL_RGB;
    imageType=GL_UNSIGNED_BYTE;

    LOG_DEBUG("Successfully loaded BMP image data:");
    LOG_DEBUG("     w=" << imageWidth << " h=" << imageHeight);
    LOG_DEBUG("     size=" << bmpInfo->bmiHeader.biSizeImage << " depth=" << bmpInfo->bmiHeader.biBitCount);
    LOG_DEBUG("     pixels/meter: x=" << bmpInfo->bmiHeader.biXPelsPerMeter << " y=" << bmpInfo->bmiHeader.biYPelsPerMeter); 

    free(bmpInfo);

    setupTexture();

    TRACE_EXIT();
    return true;
}

bool BackgroundImage::loadPPMFile(const char *filename)
{
    TRACE_ENTER();

    FILE* fp;
    int i, w, h, d;
    char head[70];          /* max line <= 70 in PPM (per spec). */

    fp = fopen(filename, "rb");
    if (!fp) {
        LOG_ERROR("Error opening image file, " << filename << strerror(errno)); 
        return false;
    }

    /* grab first two chars of the file and make sure that it has the
     *        correct magic cookie for a raw PPM file. */
    if (NULL==fgets(head, 70, fp))
    {
        LOG_WARN("Error reading image file " << filename << ":" << strerror(errno)); 
        fclose(fp); 
        TRACE_EXIT_RET(false);
        return false;
    }
    if (strncmp(head, "P6", 2)) {
        LOG_ERROR(filename << ": Not a raw PPM file"); 
        fclose(fp); 
        TRACE_EXIT_RET(false);
        return false;
    }

    /* grab the three elements in the header (imageWidth, imageHeight, maxval). */
    i = 0;
    while(i < 3) {
        if (NULL==fgets(head, 70, fp))
        {
            LOG_ERROR("Error while reading image file: " << filename << ":" << strerror(errno)); 
            TRACE_EXIT_RET(false);
            fclose(fp); 
            return false;
        }
        if (head[0] == '#')     /* skip comments. */
            continue;
        if (i == 0)
            i += sscanf(head, "%d %d %d", &w, &h, &d);
        else if (i == 1)
            i += sscanf(head, "%d %d", &h, &d);
        else if (i == 2)
            i += sscanf(head, "%d", &d);
    }

    /* grab all the image data in one fell swoop. */
    if (imageData) 
        free(imageData); 
    imageData = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
    size_t bytesRead=fread(imageData, sizeof(unsigned char), w*h*3, fp);
    fclose(fp);
    if (bytesRead!=(size_t)w*h*3)
    {
        LOG_ERROR("Error reading image data from file " << filename);
        TRACE_EXIT_RET(false);
        return false;
    }

    imageWidth = w;
    imageHeight = h;

    // default to using image w and h
    xoffset=imageWidth;
    yoffset=imageHeight;

    imageFormat=GL_RGB;
    imageType=GL_UNSIGNED_BYTE;

    setupTexture();

    TRACE_EXIT_RET((imageData!=NULL?"true":"false"));
    return imageData!=NULL;
}

void BackgroundImage::setupTexture()
{
    TRACE_ENTER();

    glPushMatrix();
    {
        // if ppm
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        // glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor);
        // glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // else BMP
        // glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        // static GLuint textureInt;
        // glGenTextures(1, &textureInt); 
        // glBindTexture(GL_TEXTURE_2D, textureInt); 
        // glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        // glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imageWidth, imageHeight, imageFormat, imageType, imageData);

        // glDeleteTextures(1, &textureInt);

        static GLuint textureIntID;
        glGenTextures(1, &textureIntID);
        glBindTexture(GL_TEXTURE_2D, textureIntID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);    //The flag is set to TRUE
        // glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imageWidth, imageHeight, 0, imageFormat, imageType, imageData);  //When this is called, the GPU generates all mipmaps

        free(imageData);
        imageData=NULL;

        imageLoaded=true;
    }
    glPopMatrix();

    TRACE_EXIT();
}

void BackgroundImage::drawImage()
{
    TRACE_ENTER();

    if (!imageLoaded)
    {
        TRACE_EXIT();
        return;
    }

    glPushMatrix();
    {
        glTranslatef(0, 0, z);
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        {
            glEnable(GL_TEXTURE_2D); 
            glDisable(GL_LIGHTING);
            glDisable(GL_BLEND); 
            glBegin(GL_QUADS);
            {
                glTexCoord2f(0,0); glVertex2f(minx        ,miny);
                glTexCoord2f(1,0); glVertex2f(minx+xoffset,miny);
                glTexCoord2f(1,1); glVertex2f(minx+xoffset,miny+yoffset);
                glTexCoord2f(0,1); glVertex2f(minx        ,miny+yoffset);
            }
            glEnd();
        }
        glPopAttrib();
    }
    glPopMatrix();

    TRACE_EXIT();
}

void BackgroundImage::setDrawingCoords(GLfloat minx_, GLfloat width_, GLfloat miny_, GLfloat height_, GLfloat z_)
{
    TRACE_ENTER();
    minx=minx_;
    xoffset=width_==0?imageWidth:width_;  // if not set, use image width as offset
    miny=miny_;
    yoffset=height_==0?miny+imageHeight:height_; // If not set, use image height as offset.
    z=z_;
    TRACE_EXIT();
}
void BackgroundImage::getDrawingCoords(GLfloat &minx_, GLfloat &width_, GLfloat &miny_, GLfloat &height_, GLfloat &z_)
{
    TRACE_ENTER();
    minx_=minx;
    width_=xoffset;
    miny_=miny;
    height_=yoffset;
    z_=z;
    TRACE_EXIT();
}

void BackgroundImage::centerImage(bool val)
{
    TRACE_ENTER();
    imageCenter=val;
    TRACE_EXIT();
}

bool BackgroundImage::centerImage() const
{
    TRACE_ENTER();
    TRACE_EXIT_RET( (imageCenter?"true":"false") );
    return imageCenter;
}

