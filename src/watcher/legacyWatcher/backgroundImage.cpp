#include <GL/gl.h>
#include <GL/glu.h>

#include <string.h>
#include <errno.h>

#include "backgroundImage.h"
#include "mobility.h"

using namespace watcher;

INIT_LOGGER(BackgroundImage, "BackgroundImage"); 

BackgroundImage::BackgroundImage() :
    imageData(NULL),
    width(0),
    height(0)
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

    /* grab the three elements in the header (width, height, maxval). */
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

    width = w;
    height = h;

    TRACE_EXIT_RET((imageData!=NULL?"true":"false"));
    return imageData!=NULL;
}

void BackgroundImage::setupTexture()
{
    TRACE_ENTER();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    TRACE_EXIT();
}

void BackgroundImage::drawImage(GLfloat minx, GLfloat maxx, GLfloat miny, GLfloat maxy, GLfloat z)
{
    TRACE_ENTER();

    glPushMatrix();
    glTranslatef(minx, miny, z);

    static int texSetup=0;
    if (!texSetup) 
    {
        setupTexture();
        texSetup=1;
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glColor4f(1.0,1.0,1.0,0.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
	static const GLfloat black[]={0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE, black);

    // Draw blank black background, larger than the image
    glBegin(GL_QUADS);
        glVertex3f(-maxx*2, -maxy*2, -1.0);
        glVertex3f(-maxx*2, maxy*2, -1.0);
        glVertex3f(maxx*2, maxy*2, -1.0);
        glVertex3f(maxx*2, -maxy*2, -1.0);

        glTexCoord2f(0,1); glVertex2f(minx,miny);       // This may seem backwards, but it works.
        glTexCoord2f(0,0); glVertex2f(minx,maxy);       // This may seem backwards, but it works.
        glTexCoord2f(1,0); glVertex2f(maxx,maxy);       // This may seem backwards, but it works.
        glTexCoord2f(1,1); glVertex2f(maxx,miny);       // This may seem backwards, but it works.
    glEnd();
    glDisable(GL_TEXTURE_2D); 
    glDisable(GL_LIGHTING);

    glPopAttrib();

    glPopMatrix();

    TRACE_EXIT();
}


