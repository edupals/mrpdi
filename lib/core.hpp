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

#ifndef MRPDI_CORE
#define MRPDI_CORE

#include "input.hpp"
#include "driver.hpp"

#include <vector>
#include <string>

namespace lliurex
{
    namespace mrpdi
    {
    
        struct connected_device_info
        {
            unsigned int id;
            unsigned int address;
            unsigned char type;
            unsigned int status;
            std::string name;
        };

        struct parameter_conf_entry
        {
            std::string driver_name;
            std::string parameter_name;
            unsigned int value;
        };
        
        class Core
        {
            private:
            std::vector<Driver*> drivers;
            input::BaseInputHandler * inputhandler;
            
            void load_drivers();
            
            public:
            static Core * instance;
            static Core * getCore();
            
            Core();
            ~Core();
            
            void init();
            void shutdown();
            
            void set_input_handler(input::BaseInputHandler * handler);
            input::BaseInputHandler * get_input_handler();
            
            void update_devices(std::vector<connected_device_info> * out_list);
            void get_parameter_list(unsigned int id,std::vector<std::string> * out_list);
            
            std::string get_device_name(unsigned int id);
            
            void start(unsigned int id,unsigned int address);
            void stop(unsigned int id,unsigned int address);
            
            void set_parameter(unsigned int id,const char * key,unsigned int value);
            int get_parameter(unsigned int id,const char * key,unsigned int * value);

        };
        
    }
}

#endif
