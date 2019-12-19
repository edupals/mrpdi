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

#include "core.hpp"
#include "basedriver.hpp"

#include <libusb-1.0/libusb.h>

#include <dirent.h>
#include <dlfcn.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;
using namespace lliurex::mrpdi;

#define PLUGIN_PATH "/usr/lib/mrpdi/"
#define SETTINGS_PATH "/etc/mrpdi/conf.d/"

#ifdef NDEBUG
    #define debug if(false)log
#else
    #define debug log
#endif

void pointer_callback(driver_event event)
{
    
    if(Core::getCore()->get_input_handler()!=NULL)
    {
        Core::getCore()->get_input_handler()->pointer_callback(event);
    }
}

Core * Core::instance = NULL;

/**
 * Instance getter 
 */ 
Core * Core::getCore()
{
    return Core::instance;
}

/**
 * private constructor
 */ 
Core::Core()
{
    log.set_header("[Core]");
    
    debug<<"constructor"<<endl;
    
    Core::instance=this;
    this->inputhandler=NULL;
    //this->inputhandler=new mrpdi::input::InputHandler();
}

Core::~Core()
{
    debug<<"destructor"<<endl;
}

/**
 * Private method, loads SO files and extracts needed information
 */ 
void Core::load_drivers()
{
    DIR * d;
    dirent *dirp;
    string extension(".so");
    d = opendir(PLUGIN_PATH);
    
    if (d==nullptr) {
        cerr<<"Error opening: "<<PLUGIN_PATH<<endl;
        return;
    }
    
    dirp = readdir(d);
    char ** name;
    char ** version;
    device_info * devs;
    parameter_info * params;
    void  (*init)(void);
    void  (*set_parameter)(const char *,unsigned int);
    Driver * driver;
    
    void (*set_callback)(void(*)(driver_event));
        
    debug<<"loading driver list..."<<endl;
    
    while(dirp!=nullptr)
    {
        
        string filename(dirp->d_name);
        if(filename.find(extension)!=string::npos)
        {
            
            string path(PLUGIN_PATH);
            path=path+dirp->d_name;
            
            debug<<"file:"<<path<<endl;
            
            void* handle = dlopen(path.c_str(), RTLD_LAZY);
            if(handle!=NULL)
            {
                        
                //name
                name=(char **)dlsym(handle,"name");
                //version
                version=(char **)dlsym(handle,"version");
                
                driver = new Driver();
                        
                //dlopen handle
                driver->handle=handle;
                
                        
                //supported devices
                devs=(device_info *)dlsym(handle,"supported_devices");
                int n = 0;
                while(devs[n].id!=0xffffffff)
                {
                    driver->supported_devices.push_back(devs[n]);
                    n++;
                }
                
                //supported parameters
                params=(parameter_info *)dlsym(handle,"supported_parameters");
                n=0;
                while(params[n].mask!=0xffffffff)
                {
                    driver->supported_parameters.push_back(params[n]);
                    n++;
                }
                
                init=(void (*)())dlsym(handle,"init");
                init();
                
                set_parameter=(void (*)(const char *,unsigned int))dlsym(handle,"set_parameter");
                
                set_callback=(void (*)(void(*)(driver_event)))dlsym(handle,"set_callback");
                set_callback(pointer_callback);
                
                driver->name=*name;
                driver->filename=filename;
                drivers.push_back(driver);
            }
            
        }
        dirp=readdir(d);
    }
    
    closedir(d);
}

/**
* This method should be called at first place
* drivers are loaded here
*/
void Core::init()
{
    
    debug<<"init"<<endl;
    
    load_drivers();
}

/**
* This method should be called before closing the application so
* drivers can be properly disconnected and unloaded
*/
void Core::shutdown()
{

    debug<<"shutdown"<<endl;
    
    void  (*shutdown)(void);
    
    for(int n=0;n<drivers.size();n++)
    {
        shutdown=(void (*)())dlsym(drivers[n]->handle,"shutdown");
        shutdown();
    }
    
    drivers.clear();
}

/**
 * Sets an InputHandler, this must be set before starting a device
 */ 
void Core::set_input_handler(input::BaseInputHandler * handler)
{
    this->inputhandler=handler;
}

/**
 * Gets current InputHandler, NULL if no handler set
 */ 
input::BaseInputHandler * Core::get_input_handler()
{
    return this->inputhandler;
}

/**
 * Gets a list of connected and supported devices
 */ 
void Core::update_devices(vector<connected_device_info> * out_list)
{
        
    connected_device_info cdi;
    unsigned int  (*get_status)(unsigned int);
    
    libusb_context * ctx;
    libusb_device ** list ;
    libusb_device_descriptor  desc;
    int n;
    unsigned int id;
    unsigned int address;
    
    unsigned char bus;
    unsigned char dir;
    
    out_list->clear();
    
    libusb_init(&ctx);
    
    n=libusb_get_device_list(ctx,&list);
    
    for(int i=0;i<n;i++)
    {
        libusb_get_device_descriptor(list[i], &desc);
        id = (desc.idVendor<<16) | desc.idProduct;
        
        address=0;
        bus=libusb_get_bus_number(list[i]);
        dir=libusb_get_device_address(list[i]);
        address = address | (bus<<16);
        address = address | (dir<<8);
        
        //name look up
        bool found=false;
        
        for(int m=0;m<drivers.size();m++)
        {
            for(int n=0;n<drivers[m]->supported_devices.size();n++)
            {
                if(drivers[m]->supported_devices[n].id==id)
                {
                    cdi.name=drivers[m]->supported_devices[n].name;
                    cdi.id=id;
                    cdi.address=address;
                    cdi.type=drivers[m]->supported_devices[n].type;
                    
                    get_status=(unsigned int (*)(unsigned int))dlsym(drivers[m]->handle,"get_status");
                    cdi.status=get_status(address);
                    
                    out_list->push_back(cdi);
                    
                    found=true;
                    break;
                }
            }
            
            if(found)break;
        }
        
    }
    
    libusb_free_device_list(list,1);
    libusb_exit(ctx);
    
}

/**
 * Gets a vector with the list of available parameters for
 * a given device
 */  
void Core::get_parameter_list(unsigned int id,vector<string> * out_list)
{
    unsigned int vid;
    unsigned int evid;
    int p = 0;
    
    out_list->clear();
    
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                //found a proper driver
                for(int q=0;q<drivers[n]->supported_parameters.size();q++)
                {
                    evid=drivers[n]->supported_parameters[q].mask & 0xffff0000;
                    vid = id & 0xffff0000;
                    if(drivers[n]->supported_parameters[q].mask==0 || drivers[n]->supported_parameters[q].mask==id || evid==vid)
                    {
                        out_list->push_back(drivers[n]->supported_parameters[q].name);
                    }
                }
                
            }
        }
    }
}

/**
 * Gets device name 
 */ 
string Core::get_device_name(unsigned int id)
{
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                return string(drivers[n]->supported_devices[m].name);
            }
        }
    }
    
    //this may never happen
    return string("Unknown");
}

/**
 * Turns on device
 */ 
void Core::start(unsigned int id,unsigned int address)
{
    Driver * driver = NULL;
    void  (*start)(unsigned int ,unsigned int);
    
    debug<<"starting: "<<hex<<id<<endl;
    
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                driver=drivers[n];
                break;
                
            }
        }
    }
    
    if(driver!=NULL)
    {
        
        debug<<"found capable driver:"<<driver->filename<<endl;
        
        start=(void (*)(unsigned int ,unsigned int))dlsym(driver->handle,"start");
        start(id,address);
        inputhandler->start(id,address);
    }
}

/**
 * Turns off device
 */ 
void Core::stop(unsigned int id,unsigned int address)
{
    Driver * driver = NULL;
    void  (*stop)(unsigned int ,unsigned int);
    
    debug<<"stopping: "<<hex<<id<<endl;
    
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                driver=drivers[n];
                break;
                
            }
        }
    }
    
    if(driver!=NULL)
    {
        stop=(void (*)(unsigned int ,unsigned int))dlsym(driver->handle,"stop");
        stop(id,address);
        inputhandler->stop(id,address);
    }
}

void Core::set_parameter(unsigned int id,const char * key,unsigned int value)
{
    void  (*set_parameter)(const char *,unsigned int);
    
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                set_parameter=(void (*)(const char *,unsigned int))dlsym(drivers[n]->handle,"set_parameter");
                set_parameter(key,value);
                return;
            }
        }
    }
}

int Core::get_parameter(unsigned int id,const char * key,unsigned int * value)
{
    int  (*get_parameter)(const char *,unsigned int *);
    unsigned int res;
    
    for(int n=0;n<drivers.size();n++)
    {
        for(int m=0;m<drivers[n]->supported_devices.size();m++)
        {
            if(drivers[n]->supported_devices[m].id==id)
            {
                get_parameter=(int (*)(const char *,unsigned int*))dlsym(drivers[n]->handle,"get_parameter");
                res=get_parameter(key,value);
                return res;
            }
        }
    }
    
    return -1;
}
