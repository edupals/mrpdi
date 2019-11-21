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

#ifndef MRPDI_BASEDRIVER
#define MRPDI_BASEDRIVER

#define DEV_STATUS_STOP 0
#define DEV_STATUS_RUNNING 1

struct driver_device_info
{
    unsigned int id;
    unsigned char iface;
    unsigned char type;
    const char * name;
};

struct driver_parameter_info
{
    unsigned int mask;
    const char * name;
};

enum event_type { EVENT_POINTER, EVENT_KEY, EVENT_STATUS, EVENT_DATA};
enum status_type {STATUS_READY,STATUS_SHUTDOWN,STATUS_COMMERROR};

struct driver_event
{
    unsigned int address;
    unsigned int id;
    enum event_type type;
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            unsigned int pointer;
            unsigned int button;
        }pointer;
        
        struct
        {
            unsigned char keycode;
            unsigned char mod;
        }key;
        
        struct
        {
            enum status_type id;
        }status;
        
        struct
        {
            unsigned int type;
            unsigned char buffer[32];
        }data;
         
    };
};

extern "C" const char * name;
extern "C" const char * version;
extern "C" driver_device_info supported_devices [];
extern "C" driver_parameter_info supported_parameters [];

extern "C" void init();
extern "C" void shutdown();

extern "C" void start(unsigned int id,unsigned int address);
extern "C" void stop(unsigned int id,unsigned int address);

extern "C" void set_parameter(const char * key,unsigned int value);
extern "C" int get_parameter(const char * key,unsigned int * value);

extern "C" unsigned int get_status(unsigned int address);

extern "C" void set_callback( void(*callback) (driver_event) );

#endif
