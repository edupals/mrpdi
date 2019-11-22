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

#include "input.hpp"
#include "core.hpp"
#include "utils.hpp"

#include <linux/uinput.h>
#include <linux/input.h>

#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>

//Deprecated
#define STYLUS_DEVICE_ID        0x02 
#define TOUCH_DEVICE_ID         0x03 
#define CURSOR_DEVICE_ID        0x06 
#define ERASER_DEVICE_ID        0x0A 
#define PAD_DEVICE_ID           0x0F

#define DEBUG 0

using namespace std;
using namespace lliurex::mrpdi;
using namespace lliurex::mrpdi::input;

/**
* Constructor
*/  
InputHandler::InputHandler()
{
    this->calibration_step=-1;
}

/**
* Destructor
*/ 
InputHandler::~InputHandler()
{
    
}

/**
* Setter and getter for device settings map
*/
void InputHandler::set_settings(map<unsigned int,DeviceSettingsEntry> settings)
{
    settings_map=settings;
}

map<unsigned int,DeviceSettingsEntry> InputHandler::get_settings()
{
    return settings_map;
}

/**
* This method is call with drivers callback events
*/ 
void InputHandler::pointer_callback(driver_event event)
{
    
    switch(event.type)
    {
        case EVENT_POINTER:
            if(calibration_step==-1)
            {
                for(int n=0;n<devices.size();n++)
                {
                    if(devices[n]->get_address()==event.address)
                    {
                        if(devices[n]->get_pointer()==event.pointer.pointer)
                        {
                            devices[n]->set_position(event.pointer.x,event.pointer.y);
                            devices[n]->set_pressure(event.pointer.z);
                            devices[n]->set_button(0,event.pointer.button & 0x01);
                            devices[n]->set_button(1,(event.pointer.button & 0x02)>>1);
                            devices[n]->set_button(2,(event.pointer.button & 0x04)>>2);
                            devices[n]->update();
                            break;
                        }
                    }
                }
            }
            else
            {
                
                for(int n=0;n<devices.size();n++)
                {
                    
                    if(devices[n]->get_address()==calibration_address)
                    {
                        if(devices[n]->get_pointer()==event.pointer.pointer)
                        {
                            
                            if((event.pointer.button & 0x01)==0 && calibration_press==1)
                            {
                                #if DEBUG
                                cout<<"Calibration STEP:"<<calibration_step<<" pointer["<<event.pointer.pointer<<"]"<<endl;
                                #endif
                                
                                
                                calibration_points[calibration_step*2]=event.pointer.x;
                                calibration_points[(calibration_step*2)+1]=event.pointer.y;
                                calibration_step++;
                                
                                CalibrationScreen::get_CalibrationScreen()->step(calibration_step);
                            }
                            calibration_press=(event.pointer.button & 0x01);
                            
                        
                            
                            if(calibration_step==4)
                            {
                                #if DEBUG
                                cout<<"Calibration completed!"<<endl;
                                #endif
                                devices[n]->set_calibration(calibration_points);
                                CalibrationScreen::destroy();
                                calibration_step=-1;
                                
                                //store calibration settings
                                for(int nn=0;nn<8;nn++)
                                {
                                    settings_map[devices[n]->get_id()].calibration[nn]=calibration_points[nn];
                                }
                            }
                            else 
                            {
                                //NAN	
                            }
                            
                        }
                    }
                }
                
            }
            

        break;
        
        case EVENT_KEY:
            #if DEBUG
            cout<<"Key CALLBACK:"<<event.address<<endl;
            #endif
        break;
        
        case EVENT_DATA:
            #if DEBUG
            cout<<"Data CALLBACK:"<<event.address<<endl;
            #endif
        break;
        
        case EVENT_STATUS:
            #if DEBUG
            cout<<"Status CALLBACK:"<<event.address<<endl;
            #endif
            switch(event.status.id)
            {
                case STATUS_READY:
                    #if DEBUG
                    cout<<"device is READY"<<endl;
                    #endif
                break;
                
                case STATUS_COMMERROR:
                    cout<<"device communication error"<<endl;
                break;
                
                case STATUS_SHUTDOWN:
                    #if DEBUG
                    cout<<"device is shut down"<<endl;
                    #endif
                break;
            }
        break;
    }
}

/**
* Call when a device is started, so InputHandler can enable its uinput device
* @param id target device id, 0xvendorproduct
* @param address target device usb address
*/ 
void InputHandler::start(unsigned int id,unsigned int address)
{
    #if DEBUG
    cout<<"InputHandler::start"<<endl;
    #endif
    vector<string> param_list;
    AbsolutePointer * dev;
    unsigned int num_pointers=0;
    unsigned int calibrate=0;
    unsigned int pressure=0;
    unsigned int flags = 0;
    
    //Override default driver settings
    map<unsigned int,DeviceSettingsEntry>::iterator it;
    it = settings_map.find(id);
    if(it!=settings_map.end())
    {
        map<string,unsigned int>::iterator ppointer;
        for(ppointer = settings_map[id].params.begin();ppointer!=settings_map[id].params.end();ppointer++)
        {
            string pname = ppointer->first;
            unsigned int pvalue = ppointer->second;
            #if DEBUG
                cout<<"* set_parameter("<<hex<<id<<","<<pname<<","<<dec<<pvalue<<")"<<endl;
            #endif
            Core::getCore()->set_parameter(id,pname.c_str(),pvalue);
            
        }
        
    }
    
    Core::getCore()->get_parameter_list(id,&param_list);
    
    for(int n=0;n<param_list.size();n++)
    {
        if(param_list[n].find(".pointers")!=string::npos)
        {
            Core::getCore()->get_parameter(id,param_list[n].c_str(),&num_pointers);
        }
        
        if(param_list[n].find(".calibrate")!=string::npos)
        {
            Core::getCore()->get_parameter(id,param_list[n].c_str(),&calibrate);
        }
        
        if(param_list[n].find(".pressure")!=string::npos)
        {
            Core::getCore()->get_parameter(id,param_list[n].c_str(),&pressure);
        }
    }
    
    if(num_pointers>0)
    {
        #if DEBUG
        cout<<"InputHandler: Available pointers-> "<<num_pointers<<endl;
        #endif
        
        string dev_name = Core::getCore()->get_device_name(id);
        
        if(pressure==1)
            flags=flags | PointerFlags::Pressure;
        
        for(int n=0;n<num_pointers;n++)
        {
            stringstream ssdev;
            
            ssdev <<"mrpdi::"<<dev_name<<"::"<<n;
            dev = new AbsolutePointer(ssdev.str(),id,address,n,flags);
            devices.push_back(dev);
            dev->start();
        }
        
        //check for a device setup
        it = settings_map.find(id);
        if(it==settings_map.end())
        {
            //In case there is no stored setup for a device, lets create an entry
            
            settings_map[id].id=id; //quite redundant, huh?
            settings_map[id].name=dev_name;
            #if DEBUG
            cout<<"InputHandler: There is no default setup for "<<id<<endl;
            #endif
            
            
            if(calibrate==1)
            {
                #if DEBUG
                cout<<"InputHandler: Device requested calibration"<<endl;
                #endif
                this->calibrate(address);
            }
        }
        else
        {
            #if DEBUG
            cout<<"InputHandler: Using stored calibration"<<endl;
            #endif
            dev->set_calibration(settings_map[id].calibration);
        }
    }
}

/**
* Call when a device is stoped, so uinput device can be disabled too
* @param id target device id, 0xvendorproduct
* @param address target device usb address
*/ 
void InputHandler::stop(unsigned int id,unsigned int address)
{
    #if DEBUG
    cout<<"InputHandler::stop"<<endl;
    #endif
    vector<AbsolutePointer*> backvector;
    
    for(int n=0;n<devices.size();n++)
    {
        if(devices[n]->get_address()!=address)
        {
            backvector.push_back(devices[n]);
        }
        else
        {
            devices[n]->stop();
            delete devices[n];
        }
    }
    
    devices=backvector;
}

/**
* Starts a calibration process
* @param address Target device usb address
*/ 
void InputHandler::calibrate(unsigned int address)
{
    #if DEBUG
    cout<<"[InputHandler]::calibrate()"<<endl;
    #endif
        
    //forces calibration screen creation
    CalibrationScreen::get_CalibrationScreen()->step(0);
    //NOTE: This order is important, to avoid callback event to instance a second calibration screen
    this->calibration_press=0;
    this->calibration_address=address;
    this->calibration_step=0;
}

AbsolutePointer::AbsolutePointer(string name,unsigned int id,unsigned int address,unsigned char pointer,unsigned int flags)
{
    #if DEBUG
    cout<<"AbsolutePointer()"<<endl;
    #endif
    this->name=name;
    this->id=id;
    this->address=address;
    this->pointer=pointer;
    
    this->calibration[0]=0.0f;
    this->calibration[1]=0.0f;
    
    this->calibration[2]=1.0f;
    this->calibration[3]=0.0f;
    
    this->calibration[4]=1.0f;
    this->calibration[5]=1.0f;
    
    this->calibration[6]=0.0f;
    this->calibration[7]=1.0f;
    
    this->use_calibration=false;
    
    if( (flags & PointerFlags::Pressure)==PointerFlags::Pressure)
        this->has_pressure=true;
    else
        this->has_pressure=false;
        
}

AbsolutePointer::~AbsolutePointer()
{
    #if DEBUG
    cout<<"~AbsolutePointer()"<<endl;
    #endif
}

/**
* init device
* returns: status
*/
void AbsolutePointer::start()
{

    #if DEBUG
    cout<<"AbsolutePointer start:"<<name<<endl;
    #endif
    
    fd_uinput = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    if(fd_uinput<0)
    {
        cerr<<"Failed to open uinput"<<endl;
        return;
    }
    
    //preparing iudev structure
    memset(&uidev, 0, sizeof(uidev));
    strcpy(uidev.name,name.c_str());
    uidev.id.bustype = BUS_VIRTUAL;
    uidev.id.vendor  = 0x00;
    uidev.id.product = 0x00;
    uidev.id.version = 12; /* LliureX 12.06 Nemo :) */
    
    uidev.absmin[ABS_X]=0;
    uidev.absmin[ABS_Y]=0;
    
    uidev.absmax[ABS_X]=0x0FFF;
    uidev.absmax[ABS_Y]=0x0FFF;

    set_uinput(UI_SET_EVBIT,EV_ABS);
    set_uinput(UI_SET_EVBIT,EV_KEY);
    set_uinput(UI_SET_EVBIT,EV_SYN);
    
    set_uinput(UI_SET_ABSBIT, ABS_X);
    set_uinput(UI_SET_ABSBIT, ABS_Y);
    set_uinput(UI_SET_KEYBIT, BTN_LEFT);
    set_uinput(UI_SET_KEYBIT, BTN_RIGHT);
    
    if(this->has_pressure)
    {
        uidev.absmin[ABS_Z]=0x0;
        uidev.absmax[ABS_Z]=0x0300;
        set_uinput(UI_SET_ABSBIT, ABS_Z);
    }
    
    
    //creating device
    if(write(fd_uinput, &uidev, sizeof(uidev))<0)
    {
        cerr<<"Error sending device descriptor"<<endl;
    }
    set_uinput(UI_DEV_CREATE,0);
    
    #if DEBUG
    cout<<"completed"<<endl;
    #endif
}

void AbsolutePointer::stop()
{
    #if DEBUG
    cout<<"AbsolutePointer stop()"<<endl;
    #endif
    set_uinput(UI_DEV_DESTROY,0);
    close(fd_uinput);
}

void AbsolutePointer::set_position(float x,float y)
{
    if(this->use_calibration)
    {
        Utils::inverse_interpolation(x,y,this->calibration,&this->x,&this->y);
    }
    else
    {
        this->x=x;
        this->y=y;
    }
}

void AbsolutePointer::set_button(int num,int state)
{
    this->button_state[num]=this->button[num];
    this->button[num]=state;
}

void AbsolutePointer::update()
{
    
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    gettimeofday(&ev.time, NULL);
    
    //x
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = 0x0FFF*x;
    send_uinput(&ev);
    
    //y
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = 0x0FFF*y;
    send_uinput(&ev);
    
    //z
    if(this->has_pressure)
    {
        ev.type = EV_ABS;
        ev.code = ABS_Z;
        ev.value = 0x0300*z;
        send_uinput(&ev);
    }
    
    //left button press
    if(button_state[0]==0 && button[0]==1)
    {
        ev.type = EV_KEY;
        ev.code = BTN_LEFT;
        ev.value = 1;
        send_uinput(&ev);
    }
    
    //left button release
    if(button_state[0]==1 && button[0]==0)
    {
        ev.type = EV_KEY;
        ev.code = BTN_LEFT;
        ev.value = 0;
        send_uinput(&ev);
    }
    
    //right button press
    if(button_state[1]==0 && button[1]==1)
    {
        ev.type = EV_KEY;
        ev.code = BTN_RIGHT;
        ev.value = 1;
        send_uinput(&ev);
    }
    
    //right button release
    if(button_state[1]==1 && button[1]==0)
    {
        ev.type = EV_KEY;
        ev.code = BTN_RIGHT;
        ev.value = 0;
        send_uinput(&ev);
    }
    
    //sync
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    send_uinput(&ev);
    
    
}

int AbsolutePointer::set_uinput(unsigned long int property,unsigned int value)
{
    int res;
    res=ioctl(fd_uinput, property, value);
    if(res<0)
        cerr<<"Error ioctl uinput:"<<property<<endl;
    return res;
}

int AbsolutePointer::send_uinput(input_event * ev)
{
    int res;
    res=write(fd_uinput, ev, sizeof(*ev));
    if(res<0)
        cerr<<"Error writing to uinput"<<endl;
    return res;
}

unsigned int AbsolutePointer::get_id()
{
    return this->id;
}

unsigned int AbsolutePointer::get_address()
{
    return this->address;
}

unsigned char AbsolutePointer::get_pointer()
{
    return this->pointer;
}

void AbsolutePointer::set_calibration(float * calibration)
{
    for(int n=0;n<8;n++)
    {
        this->calibration[n]=calibration[n];
    }
    
    this->use_calibration=true;
}

void AbsolutePointer::set_pressure(float z)
{
    this->z=z;
}
