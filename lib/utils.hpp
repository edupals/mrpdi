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

#ifndef MRPDI_UTILS
#define MRPDI_UTILS

#include "basedriver.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace lliurex
{
    namespace mrpdi
    {
        class Utils
        {
            public:
            static unsigned char get_iface(unsigned int id,driver_device_info * supported_devices);
            static void build_path(unsigned int address,unsigned char iface,char * out);
            static void inverse_interpolation(float x,float y,float * calibration,float * u,float * v);
            static int iabs(int v);
            static int ipow(int v,int n);
        
        };
        
        class CalibrationScreen
        {
            private:
                
            Display *dis;
            Window win;
            Screen * scr;
            GC gc;
            int width;
            int height;
            
            static CalibrationScreen * instance;
            
            CalibrationScreen();
            ~CalibrationScreen();
            
            public:
            
            static CalibrationScreen * get_CalibrationScreen();
            static void destroy();
        
            void step(int p);
            
        };
    }
}

#endif
