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

#include "basedriver.h"

#include <iostream>

using namespace std;

const char * name="BaseDriver";
unsigned int supportedDevices [] = {0xdeadbeef,0xffffffff};/*use 0xffffffff for eof*/
const char * deviceNames [] = {"Unnamed device"};

void init(unsigned int device)
{
    cout<<"base driver init()"<<endl;
}

void run()
{
    cout<<"base driver run()"<<endl;
}

int is_running()
{
    cout<<"base driver is_running()"<<endl;
    return 0;
}

void shutdown()
{
    cout<<"base driver shutdown()"<<endl;
}
