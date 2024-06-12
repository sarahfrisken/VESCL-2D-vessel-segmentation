//
// main.cpp
// Main entry point for the contouring application
//
// Copyright(C) 2024 Sarah F. Frisken, Brigham and Women's Hospital
// 
// This code is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later version.
// 
// This code is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE. See the GNU General Public License for more details.
// 
// You may have received a copy of the GNU General Public License along with this 
// program. If not, see < http://www.gnu.org/licenses/>.
// 

#include <QtWidgets/QApplication>
#include "Controller/Controller.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Controller mainWindow(nullptr);
    mainWindow.show();
    return a.exec();
}
