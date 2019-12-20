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

#include <core.hpp>
#include <input.hpp>
#include <basedriver.hpp>
#include <log.hpp>

#include <libconfig.h++>

#include <signal.h>
#include <pwd.h>
#include <unistd.h>

#include <iostream>
#include <cstring>
#include <sstream>
#include <fstream>

#ifdef NDEBUG
    #define debug if(false)log
#else
    #define debug log
#endif

using namespace lliurex::mrpdi;
using namespace std;

bool quit_requested=false;

map<unsigned int,input::DeviceSettingsEntry> settings_map;
bool corrupted_settings=false;

Log log;

/**
 * loads calibration setup
 * */
void load_setup()
{
    struct passwd * pwd;
    string path("/etc/mrpdi/settings.conf");
    
    libconfig::Config cfg;
    
    ifstream myfile(path.c_str(),std::ifstream::in);
    
    if(myfile.good())
    {
        myfile.close();
        
        try
        {
            cfg.readFile(path.c_str());
            
            libconfig::Setting & version = cfg.lookup("version");
            std::string version_str=version;
            
            if(version_str!="2.0")
            {
                log<<"warning: unknown format version: "<<version_str<<endl;
            }
        
            libconfig::Setting & setting = cfg.lookup("mrpdi.devices");
                    
            for(int n=0;n<setting.getLength();n++)
            {
                unsigned int id;
                
                string name;
                setting[n].lookupValue("name",name);
                
                id=setting[n]["id"];
                
                settings_map[id].id=id; //and fuck that redundant shit
                settings_map[id].name=name;
                            
                for(int m=0;m<8;m++)
                {
                    settings_map[id].calibration[m]=setting[n]["calibration"][m];
                }
                
                log<<"name:"<<name<<endl;
                log<<"params: "<<setting[n]["params"].getLength()<<endl;
                for(int m=0;m<setting[n]["params"].getLength();m++)
                {
                    string name;
                    unsigned int value;
                    setting[n]["params"][m].lookupValue("name",name);
                    setting[n]["params"][m].lookupValue("value",value);
                    log<<"* "<<name<<":"<<value<<endl;
                    settings_map[id].params[name]=value;
                }
                
            }//for
            
        }//try
        catch(libconfig::ParseException &e)
        {
            log<<"error parsing config file"<<endl;
            settings_map.clear();
            corrupted_settings=true;
        }
    }//if
    
}

/**
 * saves 
 **/ 
void save_setup()
{
    string path("/etc/mrpdi/settings.conf");
    
    libconfig::Config cfg;
        
    libconfig::Setting & root = cfg.getRoot();
    libconfig::Setting & version = root.add("version",libconfig::Setting::TypeString);
    version="2.0";
    
    libconfig::Setting & mrpdi = root.add("mrpdi",libconfig::Setting::TypeGroup);
    libconfig::Setting & devices = mrpdi.add("devices",libconfig::Setting::TypeList);
    
    std::map<unsigned int,input::DeviceSettingsEntry>::iterator it;
            
    for(it=settings_map.begin();it!=settings_map.end();it++)
    {
        libconfig::Setting & device = devices.add(libconfig::Setting::TypeGroup);
        //id
        libconfig::Setting & id = device.add("id",libconfig::Setting::TypeInt);
        id.setFormat(libconfig::Setting::FormatHex);
        id=(int)it->first;
        //name
        libconfig::Setting & name = device.add("name",libconfig::Setting::TypeString);
        name=it->second.name;
        //calibration
        libconfig::Setting & calibration = device.add("calibration",libconfig::Setting::TypeArray);
        for(int n=0;n<8;n++)
        {
            libconfig::Setting & f = calibration.add(libconfig::Setting::TypeFloat);
            f=it->second.calibration[n];
        }
        
        //params	
        libconfig::Setting & params = device.add("params",libconfig::Setting::TypeList);
        std::map<string,unsigned int>::iterator pit;
        
        for(pit=it->second.params.begin();pit!=it->second.params.end();pit++)
        {
            log<<"* name:"<<pit->first<<" value:"<<pit->second<<endl;
            libconfig::Setting & param = params.add(libconfig::Setting::TypeGroup);
            libconfig::Setting & pname = param.add("name",libconfig::Setting::TypeString);
            pname=pit->first;
            libconfig::Setting & pvalue = param.add("value",libconfig::Setting::TypeInt);
            pvalue=(int)pit->second;
        }
        
    }
    
    cfg.writeFile(path.c_str());
    
}

void quit_handler(int sig)
{
    
    debug<<"signal "<<sig<<endl;
    
    quit_requested=true;
}

void display_usage()
{
    cout<<"Usage: ";
    cout<<"mrpdi-cli [OPTIONS]"<<endl;
    cout<<"\n"<<"Options:"<<endl;
    cout<<"--help \t Display this usage"<<endl;
    cout<<"--list \t List available devices"<<endl;
    cout<<"--run {address} \t Run the given device address"<<endl;
    cout<<"--auto \t Start mrpdi-cli in automathic mode"<<endl;
}

int main(int argc,char * argv[])
{
    
    Core * core;
    input::InputHandler * handler;
    vector<connected_device_info> list;
    vector<string> params;
    
    log.set_header("[mrpdi-cli]");
    
    debug<<"start"<<endl;
    
    signal(SIGABRT,&quit_handler);
    signal(SIGTERM,&quit_handler);
    signal(SIGINT,&quit_handler);
    
    core = new Core();
    core->init();
    
    if(argc>1)
    {
        string option=argv[1];
        
        if(option=="--help")
        {
            display_usage();
        }
        
        if(option=="--list")
        {
            core->update_devices(&list);
    
            cout<<"* Available devices:"<<endl;
            for(int n=0;n<list.size();n++)
            {
                cout<<"\tdev: "<<list[n].name<<"\taddress: "<<hex<<list[n].address<<"\tid: "<<list[n].id<<endl;
            }
        }
        
        if(option=="--run")
        {
            if(argc>2)
            {
                bool found=false;
                unsigned int id;
                unsigned int address;
                string name;
                
                string str(argv[2]);
                istringstream sp(str,istringstream::in);
                sp>>hex>>address;
                
                core->update_devices(&list);
                
                //device look up
                for(int n=0;n<list.size();n++)
                {
                        if(list[n].address==address)
                        {
                            id=list[n].id;
                            name=list[n].name;
                            found=true;
                            debug<<"found device"<<endl;
                        }
                }
                
                uint16_t vid,pid;
                
                vid = (id & 0xffff0000)>>16;
                pid = id & 0x0000ffff;
                
                log<<"running device "<<hex<<vid<<":"<<pid<<" "<<name<<endl;
                
                if(found)
                {
                    
                    handler = new input::InputHandler();
                    core->set_input_handler(handler);
                    core->start(id,address);
                    
                    while(!quit_requested)
                    {
                        sleep(1);
                    }
                    
                    core->stop(id,address);
                    core->set_input_handler(NULL);
                    delete handler;
                }
                else
                {
                    cout<<"* Device not found, aborting"<<endl;
                }
                
            }
        }
        
        //auto mode
        if(option=="--auto")
        {
            unsigned int id, address;
            bool quit_dev;
            bool found;
            vector<string> params;
            
            load_setup();
            
            handler = new input::InputHandler();
            core->set_input_handler(handler);
            handler->set_settings(settings_map);
                    
            while(!quit_requested)
            {
                core->update_devices(&list);
                if(list.size()>0)
                {
                    log<<"connecting to "<<list[0].name<<endl;
                    address=list[0].address;
                    id=list[0].id;
                    quit_dev=false;
                    
                    core->start(id,address);
                    
                    while(!quit_dev && !quit_requested)
                    {
                        core->update_devices(&list);
                        found=false;
                        for(int n=0;n<list.size();n++)
                        {
                            if(list[n].id==id && list[n].address==address)
                            {
                                found=true;
                                if(list[n].status==DEV_STATUS_STOP)
                                {
                                    log<<"disconnected"<<endl;
                                    quit_dev=true;
                                    break;
                                }
                            }
                        }
                        
                        if(!found)
                        {
                            log<<"device no longer connected"<<endl;
                            quit_dev=true;
                        }
                        sleep(1);
                    }
                    
                    core->stop(id,address);
                }
                sleep(1);
            }
            
            settings_map = handler->get_settings();
            if(!corrupted_settings)
                save_setup();
            
            delete handler;
        }
    }
    
    core->shutdown();
    
    debug<<"shutdown"<<endl;
    
    delete core;
     
    return 0;
}
