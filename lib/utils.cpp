/*
    mrpdi

    Copyright (C) 2012-2019  Enrique Medina Gremaldos <quiqueiii@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "utils.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <cstring>

#define DEBUG 0

using namespace lliurex::mrpdi;
using namespace std;

unsigned char Utils::get_iface(unsigned int id,driver_device_info * supported_devices)
{
    int n = 0;
    unsigned char value=0;
    while(supported_devices[n].id!=0xffffffff)
    {
        if(supported_devices[n].id==id)
        {
            value = supported_devices[n].iface;
            break;
        }
        n++;
    }
    
    return value;
}

void Utils::build_path(unsigned int address,unsigned char iface,char * out)
{
    unsigned int bus,dev;
    ostringstream path(ostringstream::out);

    bus=(address & 0x00ff0000)>>16;
    dev=(address & 0x0000ff00)>>8;
    path.fill('0');
    path.width(4);
    path<<hex<<bus<<":";
    path.fill('0');
    path.width(4);
    path<<hex<<dev<<":";
    path.fill('0');
    path.width(2);
    path<<hex<<(int)iface;
    
    strcpy(out,path.str().c_str());
}

void Utils::inverse_interpolation(float x,float y,float * calibration,float * u,float * v)
{
    float x0,x1,x2,x3;
    float y0,y1,y2,y3;
    
    x0=calibration[0];
    x1=calibration[2];
    x2=calibration[4];
    x3=calibration[6];
    
    y0=calibration[1];
    y1=calibration[3];
    y2=calibration[5];
    y3=calibration[7];
    
    float dx1 = x1 - x2, 	dy1 = y1 - y2;
    float dx2 = x3 - x2, 	dy2 = y3 - y2;
    float sx = x0 - x1 + x2 - x3;
    float sy = y0 - y1 + y2 - y3;
    float g = (sx * dy2 - dx2 * sy) / (dx1 * dy2 - dx2 * dy1);
    float h = (dx1 * sy - sx * dy1) / (dx1 * dy2 - dx2 * dy1);
    
    /* This variable pack solves for a u,v=[0,0 - 1,1] source
    double a = x1 - x0 + g * x1;
    double b = x3 - x0 + h * x3;
    double c = x0;
    double d = y1 - y0 + g * y1;
    double e = y3 - y0 + h * y3;
    double f = y0;
    */
    
    float A0 = 0.1 * x0 * g;
    float A1 = 0.1 * x0 * h;
    float A2 = 0.1 * y0 * g;
    float A3 = 0.1 * y0 * h;
    
    float A4 = 0.9 * x1 * g;
    float A5 = 0.1 * x1 * h;
    float A6 = 0.9 * y1 * g;
    float A7 = 0.1 * y1 * h;
    
    float A12 = 0.1 * x3 * g;
    float A13 = 0.9 * x3 * h;
    float A14 = 0.1 * y3 * g;
    float A15 = 0.9 * y3 * h;
    
    float a = (x1-x0-A0-A1+A4+A5)/0.8;
    float d = (y1-y0-A2-A3+A6+A7)/0.8;
    float b = (x3-x0-A0-A1+A12+A13)/0.8;
    float e = (y3-y0-A2-A3+A14+A15)/0.8;
    float c = -0.1*a-0.9*b+x3+A12+A13;
    float f = -0.1*d -0.9*e+y3+A14+A15;
    
    float A =     e - f * h;
    float B = c * h - b;
    float C = b * f - c * e;
    float D = f * g - d;
    float E =     a - c * g;
    float F = c * d - a * f;
    float G = d * h - e * g;
    float H = b * g - a * h;
    float I = a * e - b * d;

    *u=(A*x + B*y + C)/(G*x + H*y + I);
    *v=(D*x + E*y + F)/(G*x + H*y + I);

}

int Utils::iabs(int v)
{
    //ToDo:
    //this can be heavily optimized!
    if(v<0)return -v;
    else return v;
}

int Utils::ipow(int v,int n)
{
    if(n>1)
        return v*ipow(v,n-1);
    else
        return v;
}

CalibrationScreen * CalibrationScreen::instance = NULL;

CalibrationScreen * CalibrationScreen::get_CalibrationScreen()
{
    
    if(CalibrationScreen::instance==NULL)
    {
        #if DEBUG
        cout<<"[CalibrationScreen]: get_CalibrationScreen()"<<endl;
        #endif
        CalibrationScreen::instance=new CalibrationScreen();
    }
    
    return CalibrationScreen::instance;
}

void CalibrationScreen::destroy()
{
    if(CalibrationScreen::instance!=NULL)
    {
        #if DEBUG
        cout<<"[CalibrationScreen]: destroy()"<<endl;
        #endif
        delete CalibrationScreen::instance;
        CalibrationScreen::instance=NULL;
    }
}

CalibrationScreen::CalibrationScreen()
{
    
    XEvent xev;
    Atom wm_state;
    Atom fullscreen;
    long mask=0;
    
    dis = XOpenDisplay(NULL);
    scr = XDefaultScreenOfDisplay(dis);
    width = XWidthOfScreen(scr);
    height = XHeightOfScreen(scr);
    
    #if DEBUG
    cout<<"width:"<<dec<<width<<endl;
    cout<<"height:"<<dec<<height<<endl;
    cout<<"screen count:"<<XScreenCount(dis)<<endl;
    #endif
    
    XRRScreenResources * res;
    XRRCrtcInfo * crtc_info;
    
    res=XRRGetScreenResources(dis,XDefaultRootWindow(dis));
    
    bool found=false;
    
    for(int n=0;n<res->ncrtc;n++)
    {
        crtc_info=XRRGetCrtcInfo(dis,res,res->crtcs[n]);
        #if DEBUG
            cout<<"num of crtc: "<<n<<endl;
            cout<<"crct_info w:"<<crtc_info->width<<endl;
            cout<<"crct_info h:"<<crtc_info->height<<endl;
            cout<<"crct_info x:"<<crtc_info->x<<endl;
            cout<<"crct_info y:"<<crtc_info->y<<endl;
        #endif
        
        for(int m=0;m<crtc_info->noutput;m++)
        {
            XRROutputInfo * output_info;
            
            output_info = XRRGetOutputInfo(dis,res,crtc_info->outputs[m]);
            #if DEBUG
                cout<<"\tOutput "<<output_info->name<<endl;
            #endif
            if(output_info->connection==RR_Connected)
            {
                #if DEBUG
                    cout<<"Found a connected output"<<endl;
                #endif	
                found = true;
                break;
            }
            
        }
        
        if(found)break;
    }
    
    width = crtc_info->width;
    height = crtc_info->height;
    
    win = XCreateSimpleWindow(dis, XDefaultRootWindow(dis), crtc_info->x, 0, crtc_info->width, height, 0, WhitePixel (dis, 0), WhitePixel(dis, 0));
    
    XSelectInput(dis,win,StructureNotifyMask);
    XMapWindow(dis, win);
    gc= XCreateGC(dis, win, 0, NULL);
    
    #if DEBUG	
    cout<<"Waiting for:"<<MapNotify<<endl;
    #endif
    while(1)
    {
        XEvent e;
        XNextEvent(dis,&e);
        if(e.type==MapNotify)
            break;
    }	
    #if DEBUG
    cout<<"MapNotify received!"<<endl;
    #endif
    
    //XMapWindow(dis, win);
    //XFlush(dis);
        
    wm_state = XInternAtom(dis, "_NET_WM_STATE", False);
    fullscreen = XInternAtom(dis, "_NET_WM_STATE_FULLSCREEN", False);
    
    memset(&xev, 0, sizeof(xev));
    xev.type = ClientMessage;
    xev.xclient.window = win;
    xev.xclient.message_type = wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = 1;
    xev.xclient.data.l[1] = fullscreen;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 2;
    
    if(!XSendEvent(dis, DefaultRootWindow(dis), False,SubstructureNotifyMask | SubstructureRedirectMask, &xev))
        cerr<<"Failed to set FullScreen"<<endl;
    else
    {
        #if DEBUG 
            cout<<"Fullscreen:success"<<endl;
        #endif
    }
    
    #if DEBUG
    cout<<"Waiting for:"<<ConfigureNotify<<endl;
    #endif
    
    XEvent e;
    int maps=0;
    while(1)
    {
        if(XCheckTypedWindowEvent(dis,win,ConfigureNotify,&e))
        {
            maps++;
            sleep(1);
        }
        else
        {
            if(maps>0)break;
                else sleep(1);
        }
    }
    
    #if DEBUG
    cout<<"MapNotify received!"<<endl;	
    #endif
    
    XFlush(dis);
    
}

CalibrationScreen::~CalibrationScreen()
{
    XDestroyWindow(dis,win);
    XFlush(dis);
}

void CalibrationScreen::step(int p)
{
    float A,w,B,v;
    int x1,x2,y1,y2;
    
    A = 0.1f*width;
    B = 0.1f*height;
    w = 0.08f*width;
    v = 0.08f*height;
    
    /*	Color format is A-R-G-B	*/
    XSetForeground(dis,gc,(unsigned long)0x00FF0000);
    
    switch(p)
    {
        case 0:
            // POINT A	
            //******************
            XClearWindow(dis,win);
            
            x1 = (int) (A - (w/2.0f));
            x2 = (int) (A + (w/2.0f));
            
            y1 = (int) B ;
            y2 = (int) B ;
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            
            x1 = (int) A ;
            x2 = (int) A ;
            
            y1 = (int) (B - (v/2.0f));
            y2 = (int) (B + (v/2.0f));
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            XFlush(dis);
        
        
        break;
        
        case 1:
            // POINT B	
            //******************
            XClearWindow(dis,win);
            
            x1 = (int) width - (A - (w/2.0f));
            x2 = (int) width - (A + (w/2.0f));
            
            y1 = (int)  B ;
            y2 = (int)  B ;
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            
            x1 = (int) width - A ;
            x2 = (int) width - A ;
            
            y1 = (int) (B - (v/2.0f));
            y2 = (int) (B + (v/2.0f));
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            XFlush(dis);
            
        break;
        
        case 2:
            // POINT C
            //******************
            XClearWindow(dis,win);
            
            x1 = (int) width - (A - (w/2.0f));
            x2 = (int) width - (A + (w/2.0f));
            
            y1 = (int)  height - B ;
            y2 = (int)  height - B ;
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            
            x1 = (int) width - A ;
            x2 = (int) width - A ;
            
            y1 = (int) height - (B - (v/2.0f));
            y2 = (int) height - (B + (v/2.0f));
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            XFlush(dis);
            
        break;
        
        case 3:
            // POINT D
            //******************
            XClearWindow(dis,win);
            
            x1 = (int)  (A - (w/2.0f));
            x2 = (int)  (A + (w/2.0f));
            
            y1 = (int)  height - B ;
            y2 = (int)  height - B ;
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            
            x1 = (int)  A ;
            x2 = (int)  A ;
            
            y1 = (int) height - (B - (v/2.0f));
            y2 = (int) height - (B + (v/2.0f));
            
            XDrawLine(dis, win, gc, x1, y1, x2, y2);
            XFlush(dis);
            
        break;
    }
}
