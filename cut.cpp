/*
 * Photo scan tool
 * Copyright (C) 2013  Tiziano Bacocco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include <iostream>

#include <iostream>
#include <iostream>
#include <list>
#include <vector>
#include <IL/il.h>
#include <IL/ilu.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <SDL/SDL.h>
#include <unistd.h>
class CannotLoadImageException {
public:
    CannotLoadImageException() {};
};
class CannotSaveImageException {
public:
    CannotSaveImageException() {};
};

inline float dist2d(float x0,float y0,float x1,float y1)
{
    return sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

class Image {
public:
    ILuint image;
    int w;
    int h;
    int d;
    unsigned char * datapointer;
    int fmt;
    Image()
    {
        ilGenImages(1,&image);
        fmt = 0;


    }
    Image(const char * filename)
    {
        ilGenImages(1,&image);
        ilBindImage(image);
        if ( ! ilLoadImage(filename))
        {
            throw(CannotLoadImageException());

        }
        ConvertToRGB();
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
        datapointer = ilGetData();
        fmt = 0;
    }
    void Save(const char * filename)
    {
        ilBindImage(image);
        if (!ilSaveImage(filename) )
        {
            throw(CannotSaveImageException());


        }
    }
    void Blur(float radius)
    {
        ilBindImage(image);
        iluBlurGaussian((int)radius);
        datapointer = ilGetData();
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
    }
    void AllocateLUM(int x, int y,char * data=NULL)
    {
        ilBindImage(image);
        ilTexImage(x,y,1,1,IL_LUMINANCE,IL_UNSIGNED_BYTE,data);
        datapointer = ilGetData();
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
        fmt =  1;
    }
    void AllocateRGB(int x, int y,char * data=NULL)
    {
        ilBindImage(image);
        ilTexImage(x,y,1,3,IL_RGB,IL_UNSIGNED_BYTE,data);
        datapointer = ilGetData();
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
        fmt = 0;
    }
    void Rescale(int x , int y)
    {
        ilBindImage(image);
        iluScale(x,y,1);
        datapointer = ilGetData();
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
    }
    void GetPixelRGB(int x_,int y_,unsigned char * pix)
    {

        int x = x_ % w;
        int y = h -(y_ % h);

        if ( datapointer )
        {
            pix[0] = datapointer[w*y*3+x*3];
            pix[1] = datapointer[w*y*3+x*3+1];
            pix[2] = datapointer[w*y*3+x*3+2];
        } else {
            printf("GetPixelRGB(%i,%i): datapointer is NULL\n",x_,y_);
        }
        //printf("%i %i %i\n",(int)pix[0],(int)pix[1],int(pix[2]));
    }
    void SetPixelRGB(int x_, int y_,unsigned char r,unsigned char g,unsigned char b)
    {
        int x = (x_ % w);
        int y = (y_ % h);
        datapointer[w*y*3+x*3] = r;
        datapointer[w*y*3+x*3+1] = g;
        datapointer[w*y*3+x*3+2] = b;


    }
    inline void SetPixelLUM(int x_, int y_, char val)
    {
        int x = x_ % w;
        int y = y_ % h;
        datapointer[y*w+x] = val;


    }
    inline void GetPixelLUM(int x_, int y_,unsigned char * p)
    {
        //printf("GetPixelLUM(%i,%i)\n",x_,y_);
        int x = abs(x_) % w;
        int y = abs(y_) % h;
        p[0] = datapointer[y*w+x];

    }
    float GetPixelF(int x_, int y_)
    {
        int x = x_ % w;
        int y = y_ % h;
        return (float)datapointer[y*w+x];;
    }
    void ConvertToLUM()
    {
        ilBindImage(image);
        ilConvertImage(IL_LUMINANCE,IL_UNSIGNED_BYTE);
        datapointer = ilGetData();
        iluFlipImage();
        fmt = 1;
    }
    void ConvertToRGB()
    {
        ilBindImage(image);
        ilConvertImage(IL_RGB,IL_UNSIGNED_BYTE);
        datapointer = ilGetData();
        fmt = 0;
    }
    void scale(float fac)
    {
        ilBindImage(image);
        iluScale(w*fac,h*fac,1);
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
        datapointer = ilGetData();
    }
    void scale(int nw, int nh)
    {
        ilBindImage(image);
        iluScale(nw,nh,1);
        w = ilGetInteger(IL_IMAGE_WIDTH);
        h = ilGetInteger(IL_IMAGE_HEIGHT);
        d = ilGetInteger(IL_IMAGE_DEPTH);
        datapointer = ilGetData();
    }
    ~Image()
    {
        ilDeleteImages(1,&image);
    }
};

void drawNonFilledRect(SDL_Surface * dest,SDL_Rect * r)
{
    SDL_Rect frect;
    frect.x = r->x;
    frect.y = r->y;
    frect.h = 1;
    frect.w = r->w;
    SDL_FillRect(dest,&frect,0x0);
    frect.x = r->x;
    frect.y = r->y;
    frect.h = r->h;
    frect.w = 1;
    SDL_FillRect(dest,&frect,0x0);
    frect.x = r->x+r->w;
    frect.y = r->y;
    frect.h = r->h;
    frect.w = 1;
    SDL_FillRect(dest,&frect,0x0);
    frect.x = r->x;
    frect.y = r->y+r->h;
    frect.h = 1;
    frect.w = r->w;
    SDL_FillRect(dest,&frect,0x0);
}
int main(int argc, char ** argv)
{
    ilInit();
    std::vector<std::string> imglist;
    for ( int i = 2; i < argc; i++ )
        imglist.push_back(std::string(argv[i]));
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface * surf = SDL_SetVideoMode(1024,768,16,0x0);


    for ( std::vector<std::string>::iterator it = imglist.begin(); it != imglist.end(); it++ )
    {
        std::string dest=std::string(argv[1])+"/"+(*it);
        std::cout << (*it) << " -> " << dest << std::endl;
        try {
            Image * img = new Image((*it).c_str());
            Image * img_scaled = new Image();
            img->ConvertToRGB();
            img_scaled->AllocateRGB(img->w,img->h);
            memcpy(img_scaled->datapointer,img->datapointer,img->w*img->h*3);
            img_scaled->scale(1024,768);


            SDL_Surface * imgsurf = SDL_CreateRGBSurface(0x0,img_scaled->w,img_scaled->h,24,0xff,0xff00,0xff0000,0x00000000);
            SDL_LockSurface(imgsurf);
            memcpy(imgsurf->pixels,img_scaled->datapointer,img_scaled->w*img_scaled->h*3);

            SDL_UnlockSurface(imgsurf);
            SDL_Rect src;
            SDL_Rect dst;
            src.x = 0;
            src.y = 0;
            src.w = img_scaled->w;
            src.h = img_scaled->h;
            dst.x = 0;
            dst.y = 0;
            dst.w = 1024;
            dst.h = 768;

            SDL_Rect cutrect;
            cutrect.x = 0;
            cutrect.y = 0;
            cutrect.w = 0;
            cutrect.h = 0;
            bool mousedown = false;
            bool exitloop = false;
            while ( !exitloop)
            {
                SDL_Event event;
                
                while ( SDL_PollEvent(&event))
                {
                    if ( event.type == SDL_KEYDOWN)
                    {
                        if ( event.key.keysym.sym == SDLK_ESCAPE )
                        {
                            exit(0);
                        }
                        if ( event.key.keysym.sym == SDLK_RETURN )
                        {
                            double x_norm = double(cutrect.x)/double(img_scaled->w);
                            double y_norm = double(cutrect.y)/double(img_scaled->h);
                            double x2_norm = double(cutrect.x+cutrect.w)/double(img_scaled->w);
                            double y2_norm = double(cutrect.y+cutrect.h)/double(img_scaled->h);
                            int x = x_norm*img->w;
                            int y = y_norm*img->h;
                            int x2 = x2_norm*img->w;
                            int y2 = y2_norm*img->h;
                            if ( x2 < x || y2 < y || x2 == x || y2 == y )
                                continue;
                            Image * newimg = new Image();
                            newimg->AllocateRGB(x2-x,y2-y);
                            ilBindImage(newimg->image);
                            ilBlit(img->image,0,0,0,x,y,0,x2-x,y2-y,1);
                            newimg->Save(dest.c_str());
                            delete newimg;
                            exitloop = true;
                            break;
                        }
                    }
                    
                    if ( event.type == SDL_MOUSEBUTTONDOWN )
                    {
                        cutrect.x = event.motion.x;
                        cutrect.y = event.motion.y;
                        std::cout << "Down: X: " << cutrect.x << " Y: " << cutrect.y << std::endl;
                        mousedown = true;
                    }
                    if ( event.type == SDL_MOUSEMOTION && mousedown)
                    {
                        cutrect.w = event.motion.x-cutrect.x;
                        cutrect.h = event.motion.y-cutrect.y;
                    }
                    if ( event.type == SDL_MOUSEBUTTONUP )
                    {
                        mousedown = false;
                    }
                }
                if ( exitloop )
                    break;
                if ( SDL_BlitSurface(imgsurf,&src,surf,&dst) < 0 )
                {
                    std::cerr << "SDL_Blitsurface failed" << std::endl;
                }


                drawNonFilledRect(surf,&cutrect);

                SDL_Flip(surf);
            }
            SDL_FreeSurface(imgsurf);
            delete img;
            delete img_scaled;
        } catch( CannotLoadImageException e )
        {
            std::cerr << "Failed to load " << (*it) << std::endl;
        }
    }
    SDL_Quit();
}
