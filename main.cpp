
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

#include <IL/il.h>
#include <IL/ilu.h>
#include <iostream>

#include <list>
#include <vector>

#include <stdint.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <unistd.h>
class CannotLoadImageException {
public:
  CannotLoadImageException(){};
};
class CannotSaveImageException {
  public:
  CannotSaveImageException(){};
};

inline float dist2d(float x0,float y0,float x1,float y1)
{
    return sqrt((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

inline float average(std::vector<float>& v)
{
    float sum = 0.0f;
    for ( std::vector<float>::const_iterator it = v.begin(); it != v.end(); it++ )
    {
        sum += *it;
    }
    return sum/float(v.size());
}

class Image{
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
    Image(Image& other)
    {
      ilGenImages(1,&image);
      fmt = 0;
      ilBindImage(image);
      ilCopyImage(other.image);
    }
    Image(char * filename)
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
    void Save(char * filename)
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
    void GaussiaDiffProcessing()//ASSUMING RGB!!
    {
        //Differenza di gaussiane
        Image * blurredless = new Image(*this);
        Image * blurredmore = new Image(*this);
        blurredless->Blur(2.0);
        blurredmore->Blur(5.0);
        float *newimage = new float[w*h*3];
        float bottom = 200000;
        for ( int x = 0; x < w; x++ )
        {
            for ( int y = 0; y < h; y++ )
            {
                unsigned char blesspix[3];
                unsigned char bmorepix[3];
                blurredless->GetPixelRGB(x,y,blesspix);
                blurredmore->GetPixelRGB(x,y,bmorepix);
                unsigned int pix[3];
                for ( int i = 0; i < 3; i++ )
                    pix[i] = std::max(0,int(bmorepix[i])-int(blesspix[i]));
//                 SetPixelRGB(x,y,255-pix[0],255-pix[1],255-pix[2]);
                newimage[y*w*3+x*3+0] = 255-pix[0];
                newimage[y*w*3+x*3+1] = 255-pix[1];
                newimage[y*w*3+x*3+2] = 255-pix[2];
                if ( newimage[y*w*3+x*3+0] < bottom )
                    bottom = newimage[y*w*3+x*3+0];
                if ( newimage[y*w*3+x*3+1] < bottom )
                    bottom = newimage[y*w*3+x*3+1];
                if ( newimage[y*w*3+x*3+2] < bottom )
                    bottom = newimage[y*w*3+x*3+2];
            }
        }
        bottom += 10.0;
       // bottom = std::min(255.0,bottom);
        float amplitude = 255.0-bottom;
        for ( int x = 0; x < w; x++ )
        {
            for ( int y = 0; y < h; y++ )
            {
                SetPixelRGB(x,y,((newimage[y*w*3+x*3+0]-bottom)/amplitude)*255.0,((newimage[y*w*3+x*3+1]-bottom)/amplitude)*255.0,((newimage[y*w*3+x*3+2]-bottom)/amplitude)*255.0);
            }
        }
        delete blurredless;
        delete blurredmore;
        ilBindImage(image);        
        
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
    inline void GetPixelRGB(int x_,int y_,unsigned char * pix)
    {
      
      int x = x_ % w;
      int y = h -(y_ % h + 1);
      
      if ( datapointer )
      {
      pix[0] = datapointer[w*y*3+x*3];
      pix[1] = datapointer[w*y*3+x*3+1];
      pix[2] = datapointer[w*y*3+x*3+2];
      }else{
    printf("GetPixelRGB(%i,%i): datapointer is NULL\n",x_,y_);
      }
      //printf("%i %i %i\n",(int)pix[0],(int)pix[1],int(pix[2]));
    }
    inline void SetPixelRGB(int x_, int y_,unsigned char r,unsigned char g,unsigned char b)
    {
      int x = (x_ % w);
      int y = h -(y_ % h + 1);
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
    void ConvertToSpecialLUM()
    {
        unsigned char pix[3];
        unsigned char rgb_min = 255;
        unsigned char rgb_max = 0;
        for ( int y = 0; y < h; y++ )
        {
            for ( int x = 0; x < w; x++ )
            {
                rgb_min = 255;
                rgb_max = 0;
                double avg = 0.0;
                GetPixelRGB(x,y,pix);
                for ( int i = 0; i < 3; i++ )
                {
                    if ( pix[i] < rgb_min )
                        rgb_min = pix[i];
                    if ( pix[i] > rgb_max )
                        rgb_max = pix[i];
                    avg += pix[i];
                }
                avg /= 3.0;
               // if ( double(rgb_max-rgb_min)/avg > 0.8 )
                    SetPixelRGB(x,y,rgb_max,rgb_max,rgb_max);

                    
            }
        }
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
    inline float projectHorizLine(float H,float angle, int startx)
    {
        //Inizia al centro dell'immagine
        float xorigin = startx;
        float yorigin = H;
        float xdir = cos(angle);
        float ydir = sin(angle);
        float x1=xorigin+w/8,y1=yorigin;//,x2=xorigin-w/4,y2=yorigin;
        float sum = 0.0;
        int count = 0;
        bool black1 = false;
        bool black2 = false;
        float maxlen = w/4;
        float l = 0.0;
        while ( (x1 >= 0 && x1 < w )/* && ( x2 >= 0 && x2 < w)*/ && ( y1 >= 0 && y1 < h ) /*&& ( y2 >= 0 && y2 < h )*/ && l < maxlen)
        {
            
            x1+=xdir;
            y1+=ydir;
//             x2-=xdir;
//             y2-=ydir;
            unsigned char p;
            if ( ! black1 )
            {
                GetPixelLUM(x1,y1,&p);
                sum += p;
                black1 = p == 0;
            }
//             if ( ! black2 )
//             {
//                 GetPixelLUM(x2,y2,&p);
//                 sum += p;
//                 black2 = p == 0;
//             }
            if ( black1 || black2 )
                return 0;
            count += 2;
            l += 1.0;
        }
        if ( count > 0 )
            return sum/float(count);
        else 
        {
            printf("Project out of bounds x1=%f,y1=%f\n",x1,y1/*,x2,y2*/);
            return 0.0f;
        }
    }
    
    inline std::vector<float> projectHorizWhAvg(float angle)
    {
        std::vector<float> ret;
        ret.resize(h);
#pragma omp parallel for
        for ( int y = h/4; y < (h/4)*3; y++ )
        {
            ret[y] = 0;
            for ( int x = 0; x < 5; x++ )
            {
                ret[y] += projectHorizLine(y,angle,x*(w/5));
            }
        }
        return ret;
    }
    
    inline double projectAvg(float angle)
    {
        std::vector<float> v = projectHorizWhAvg(angle);
       // float avg = average(v);
        double diff = 0.0f;
       // float prev=255.0f;
        for ( std::vector<float>::const_iterator it = v.begin(); it != v.end(); it++ )
        {
            diff += *it; //((prev-*it)*(prev-*it));
          //  prev = *it;
        }
        return diff;
    }
    void AutoRotate()
    {
        if ( fmt != 1 )
            return;
        float startangle = -8.0f;
        double maxPROJ = DBL_MIN;
        float maxPROJangle = -8.0f;
        while ( startangle < 9.0f )
        {
            double p = projectAvg((2*3.141592653f)*(startangle/360.0f));
            if ( p > maxPROJ )
            {
                maxPROJangle = startangle;
                maxPROJ = p;
            }
            printf("Angle: %f : projavg %f\n",startangle,p);
            startangle += 1.0f;
        }
        printf("Rotate %f\n",maxPROJangle);
        ilClearColour(1.0,1.0,1.0,1.0);
        iluRotate(-maxPROJangle);
    }
    
    ~Image()
    {
      ilDeleteImages(1,&image);
    }
};
float Xfabs( float x )
{
    if ( fabs(x) > 5 )
        return fabs(x);
    else
        return 0.0f;
}
int main(int argc, char **argv) {
    ilInit();
    Image * i = new Image(argv[1]);
   /* Image * blurred = new Image(argv[1]);
    blurred->ConvertToLUM();
    blurred->Blur(30.0f);*/
    i->GaussiaDiffProcessing();
  //  i->Save("testgaussian.png");
    i->ConvertToLUM();
    ilBindImage(i->image);
    //iluSharpen(5.0,2);
    //i->scale(2.0); //Oversample
    Image * o = new Image();
    o->AllocateLUM(i->w,i->h);
    float offset = 0.0f;
    if ( argc > 3 )
        offset = atof(argv[3]);
    printf("Offset: %f\n",offset);
    //64x64 block
    int bcX = i->w/64+1;
    int bcY = i->h/64+1;
    int X,Y;
    int bX,bY;
    
    for ( int X = 0; X < i->w; X++ )
    {
        for ( int Y = 0; Y < i->h; Y++ )
        {
            unsigned char lumpix;
            i->GetPixelLUM(X,Y,&lumpix);
            if ( lumpix < 220 )
            {
                o->SetPixelLUM(X,Y,0);
            }else{
                o->SetPixelLUM(X,Y,255);
            }
            
        }
        
    }
   /* for ( bX = 0; bX < bcX; bX++)
    {
        for ( bY = 0; bY < bcY; bY++)
        {
            int offs_x = bX*64;
            int offs_y = bY*64;
            float top = FLT_MIN;
            float bottom = FLT_MAX;
            
           // float sigma = 0.0f;

            for ( Y = 0; Y < std::min(i->h-offs_y,64); Y++ )
            {
                for ( X = 0; X < std::min(i->w-offs_x,64); X++ )
                {
                    unsigned char lumpix;
                    i->GetPixelLUM(offs_x+X,offs_y+Y,&lumpix);
                    int PX=offs_x+X,PY=offs_y+Y;
                    float pix = lumpix;
                    float localtop = FLT_MIN;
                    float localbottom = FLT_MAX;
                    for ( int YY = 0; YY < 1; YY++ )
                    {
                        for ( int XX = 0; XX < 1; XX++ )
                        {
                            unsigned char lumpix_local;
                            i->GetPixelLUM(offs_x+X+XX,offs_y+Y+YY,&lumpix_local);
                            float pix_local = lumpix_local;
                            
                            if ( pix_local > localtop )
                                localtop = pix_local;
                            if ( pix_local < localbottom )
                                localbottom = pix_local;
                        }
                    }
                    
                    
                    if ( localtop > top )
                        top = localtop;
                    if ( localbottom < bottom )
                        bottom = localbottom;
                   /* if ( (X > 1 && X < std::min(i->w-offs_x,64)-1 ) &&
                         (Y > 1 && Y < std::min(i->w-offs_x,64)-1 ) )
                    {
                        
                        sigma += Xfabs(i->GetPixelF(PX-1,PY)-pix);
                        sigma += Xfabs(i->GetPixelF(PX+1,PY)-pix);
                        
                        sigma += Xfabs(i->GetPixelF(PX,PY+1)-pix);
                        sigma += Xfabs(i->GetPixelF(PX,PY-1)-pix);
                        
                        sigma += Xfabs(i->GetPixelF(PX+1,PY+1)-pix);
                        sigma += Xfabs(i->GetPixelF(PX+1,PY-1)-pix);
                        
                        sigma += Xfabs(i->GetPixelF(PX-1,PY+1)-pix);
                        sigma += Xfabs(i->GetPixelF(PX-1,PY-1)-pix);
                    }*/
               /* }
            }
            
           // sigma /= (std::min(i->h-offs_y,64)-1)*(std::min(i->w-offs_x,64)-1);
            
            /*for ( Y = 0; Y < std::min(i->h-offs_y,64); Y++ )
            {
                for ( X = 0; X < std::min(i->w-offs_x,64); X++ )
                {
                    unsigned char upix;
                    float fpix;
                    unsigned char lumpix;
                    i->GetPixelLUM(offs_x+X,offs_y+Y,&lumpix);
                    float pix = lumpix;
                    if ( pix < (top-bottom)/2.0+offset )
                        fpix = 0;
                    else fpix = 255;
                    upix = (unsigned char)fpix;
                    o->SetPixelLUM(offs_x+X,offs_y+Y,upix);
                    
                    
                }
            }
            
        }
    }*/
            
    o->AutoRotate();
    
   // o->scale(0.5);
    unlink(argv[2]);
    o->Save(argv[2]);
    return 0;
}
