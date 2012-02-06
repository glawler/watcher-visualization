/* Copyright 2009 SPARTA, Inc., dba Cobham Analytic Solutions
 * 
 * This file is part of WATCHER.
 * 
 *     WATCHER is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Affero General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     WATCHER is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU Affero General Public License for more details.
 * 
 *     You should have received a copy of the GNU Affero General Public License
 *     along with Watcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/** 
 * @file backgroundImage.cpp
 * @author Geoff Lawler <geoff.lawler@cobham.com>
 * @date 2009-07-15 
 */
#include <GL/gl.h>
#include <GL/glu.h>
#include <boost/filesystem.hpp>

#include <SDL_image.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "backgroundImage.h"
#include "bitmap.h"
#include "logger.h"

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
    minx(0.0),
    miny(0.0),
    xoffset(0.0),  	// use image defaults 
    yoffset(0.0), 	// use image defaults 
    z(0.0),
    imageWidth(0),
    imageHeight(0),
    imageFile("")
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
    TRACE_EXIT();
}

bool BackgroundImage::loadImageFile(const std::string &filename)
{
	TRACE_ENTER(); 
	loadSDLSurfaceTexture(filename); 
	TRACE_EXIT(); 
    return true;
}

void BackgroundImage::loadSDLSurfaceTexture(const std::string &filename) {
	TRACE_ENTER(); 
	if (!boost::filesystem::exists(filename)) { 
		LOG_WARN("Unable to load backgroun image " << filename << " as the file does not exist."); 
		imageWidth=0;
		imageHeight=0; 
		imageFile="";
		imageLoaded=false; 
		return;
	}
	LOG_INFO("Loading background image from file: " << filename); 
	SDL_Surface* surface = IMG_Load(filename.c_str());
	glGenTextures(1, &textureIntID);
	glBindTexture(GL_TEXTURE_2D, textureIntID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	GLenum textureFormat=GL_RGBA;
	/* Set the color mode */
	switch (surface->format->BytesPerPixel) {
		case 4:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				textureFormat = GL_BGRA;
			else
				textureFormat = GL_RGBA;
			break;

		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				textureFormat = GL_BGR;
			else
				textureFormat = GL_RGB;
			break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);
    imageWidth=surface->w; 
    imageHeight=surface->h; 
    imageFile=filename;
	imageLoaded=true; 
	SDL_FreeSurface(surface);
	TRACE_EXIT(); 
}

void BackgroundImage::drawImage()
{
    TRACE_ENTER();

    if (!imageLoaded) {
        TRACE_EXIT();
        return;
    }

	glPushMatrix(); 
	glTranslatef(0,0,z); 
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_TEXTURE_2D); 
	glDisable(GL_LIGHTING); 
	glDisable(GL_BLEND); 
	// glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glBindTexture(GL_TEXTURE_2D, textureIntID);
	glBegin(GL_QUADS);
	glTexCoord2f(0,0); glVertex2f(minx        ,miny+yoffset);
	glTexCoord2f(1,0); glVertex2f(minx+xoffset,miny+yoffset);
	glTexCoord2f(1,1); glVertex2f(minx+xoffset,miny);
	glTexCoord2f(0,1); glVertex2f(minx        ,miny);
	glEnd();
	glPopAttrib(); 
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

