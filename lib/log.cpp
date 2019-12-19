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

#include "log.hpp"

#include <iostream>
#include <mutex>

using namespace lliurex::mrpdi;
using namespace std;

std::mutex io_mutex;

int LogBuffer::sync()
{
    std::lock_guard<std::mutex> lock(io_mutex);
    
    cerr<<header<<" "<<this->str()<<std::flush;
    str("");
    
    return 0;
}

Log::Log() : std::ostream(&buffer)
{
}

void Log::set_header(string header)
{
    buffer.header=header;
}
