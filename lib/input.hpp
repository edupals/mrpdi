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

#ifndef MRPDI_INPUT
#define MRPDI_INPUT

#include "basedriver.hpp"
#include "log.hpp"

#include <linux/uinput.h>
#include <linux/input.h>

#include <string>
#include <vector>
#include <map>

namespace lliurex
{
    namespace mrpdi
    {
        namespace input
        {
            
            namespace PointerFlags
            { 
                enum e
                {
                    Simple=0,
                    Pressure=1
                };
            }
            
            class DeviceSettingsEntry
            {
                public:
                
                std::string name;
                unsigned int id;
                float calibration[8];
                std::map<std::string,unsigned int> params;
                
                DeviceSettingsEntry()
                {
                    //Safe default settings
                    name="Unknown";
                    id=0x00000000;
                    calibration[0]=0.0f;
                    calibration[1]=0.0f;
                    calibration[2]=0.0f;
                    calibration[3]=0.0f;
                    calibration[4]=0.0f;
                    calibration[5]=0.0f;
                    calibration[6]=0.0f;
                    calibration[7]=0.0f;
                }
            };
            
            class AbsolutePointer
            {
                protected:
                Log log;
                
                int fd_uinput;
                struct uinput_user_dev uidev;
                
                unsigned int id;
                unsigned int address;
                unsigned char pointer;
                
                std::string name;
                
                float x,y;
                float z;
                int button[6];
                int button_state[6];
                
                float calibration[8];
                bool use_calibration;
                bool has_pressure;
                
                int set_uinput(unsigned long int property,unsigned int value);
                int send_uinput(input_event * ev);
                
                public:
                
                AbsolutePointer(std::string name,unsigned int id,unsigned int address,unsigned char pointer,unsigned int flags);
                ~AbsolutePointer();
                
                void start();
                void stop();
                void set_position(float x,float y);
                void set_button(int num,int state);
                void set_pressure(float z);
                void update();
                unsigned int get_id();
                unsigned int get_address();
                unsigned char get_pointer();
                
                void set_calibration(float * calibration);
                
            };
            
            class BaseInputHandler
            {
                public:
                    
                virtual void pointer_callback(driver_event event) = 0;
                virtual void start(unsigned int id,unsigned int address) = 0;
                virtual void stop(unsigned int id,unsigned int address) = 0;
                
                virtual void calibrate(unsigned int address) = 0;
            };
            
            class InputHandler : public BaseInputHandler
            {
                private:
                Log log;
                
                std::vector<AbsolutePointer *> devices;
                std::map<unsigned int,DeviceSettingsEntry> settings_map;
                
                int calibration_step;
                float calibration_points[8];
                unsigned int calibration_address;
                int calibration_press;
                
                public:
                
                InputHandler();
                ~InputHandler();
                
                void set_settings(std::map<unsigned int,DeviceSettingsEntry> settings);
                std::map<unsigned int,DeviceSettingsEntry> get_settings();
                
                void pointer_callback(driver_event event);
                void start(unsigned int id,unsigned int address);
                void stop(unsigned int id,unsigned int address);
                
                void calibrate(unsigned int address);
            };
            
        }
    }
}

#endif
