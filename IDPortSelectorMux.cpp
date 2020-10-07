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



#include "IDPortSelectorMux.h"

#include "LogicEditor.h"
#include "LEDevice.h"
#include "Application.h"

extern Application * app;

IDPortSelectorMux::IDPortSelectorMux( int size )
	: IDPortSelector( size )
{
	mux=0;
}

IDPortSelectorMux::~IDPortSelectorMux()
{}

// Acceso a los puntos de conexión de la interfaz del módulo (entradas y salidas)
LEConnectionPoint * IDPortSelectorMux::enable( unsigned int i )
{
	// Las conexiones de "Seleccion:MUX?" se ordenan así: Enable+Salidas+Entradas
	return mux?mux->pinList().at(0)->connectionPoint():0;
}

LEConnectionPoint * IDPortSelectorMux::in( unsigned int i )
{
	// Las conexiones de "Seleccion:MUX?" se ordenan así: Enable+Salidas+Entradas
	return mux?mux->pinList().at( 1 + i + outCount() )->connectionPoint():0;
}

LEConnectionPoint * IDPortSelectorMux::out( unsigned int i )
{
	// Las conexiones de "Seleccion:MUX?" se ordenan así: Enable+Salidas+Entradas
	return mux?mux->pinList().at( 1+i )->connectionPoint():0;
}

// Instanciación
bool IDPortSelectorMux::create( LogicEditor * editor, int left, int top )
{
	// Cargamos el componente
	LMComponent * compMux = app->libraryManager().findComponent( "Seleccion:MUX"+QString::number(size()) );
	if( !compMux )
		return false;

	// Instanciación del componente
	mux = editor->createDevice( compMux, false );
	mux->setName( "PORT_MUXER" );
	mux->move( left, top );
	mux->show();

	// Actualización de Geometría
	this->l = left;
	this->t = top;
	this->w = mux->width();
	this->h = mux->height();

	return true;
}
