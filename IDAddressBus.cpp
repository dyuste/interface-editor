//  Interface Editor: a basic circuit editor
//  Copyright (C) 2004  David Yuste Romero (david.yuste@gmail.com)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.



#include "IDAddressBus.h"

#include "LogicEditor.h"
#include "LEDevice.h"

#include "Application.h"
extern Application * app;

IDAddressBus::IDAddressBus( unsigned int size )
{
	this->sz = size;
	this->busAddr = 0;
}

// Destruye la estructuras de almacenamiento creadas, pero no los elementos almacenados
IDAddressBus::~IDAddressBus()
{
	if( this->busAddr )
		delete [] busAddr;
}

// Acceso a los puntos de conexión de la interfaz del módulo (entradas y salidas)
// NOTA: Considerar que "salida"(out) es entendida a nivel de componente.
LEConnectionPoint * IDAddressBus::out( int i )
{
	return busAddr[i]->pinList().at(0)->connectionPoint();
}

// Instanciación
bool IDAddressBus::create( LogicEditor * editor, int left, int top )
{
	// Obtención de componentes requeridos
	LMComponent * inputComponent = app->libraryManager().findComponent( "I/O:Input" );
	if( !inputComponent )
		return false;

	// Creación del Bus
	busAddr = new LEDevice*[size()];

	for( int i=0; i<size(); i++ ){
		busAddr[i] = editor->createDevice( inputComponent, false );
		busAddr[i]->setName( "A"+QString::number(i) );
		busAddr[i]->move( left+20, top+45*i );
		busAddr[i]->show();
	}

	return true;	
}
