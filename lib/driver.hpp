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

#ifndef MRPDI_DRIVER
#define MRPDI_DRIVER

#include <vector>
#include <string>

namespace lliurex
{
    namespace mrpdi
    {
        struct device_info
        {
            unsigned int id;
            unsigned char iface;
            unsigned char type;
            char * name;
        };
        
        struct parameter_info
        {
            unsigned int mask;
            char * name;
        };
        
        class Driver
        {
            public:
            
            void * handle;
            std::string name;
            std::string filename;
            std::vector<device_info> supported_devices;
            std::vector<parameter_info> supported_parameters;
            
        };
    }
}

#endif
