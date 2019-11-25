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

#include "driverutils.hpp"

#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

unsigned char get_iface(unsigned int id,driver_device_info * supported_devices)
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

void build_path(unsigned int address,unsigned char iface,char * out)
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
