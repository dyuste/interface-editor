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


// IDPortSelectorMux.h: interface for the IDPortSelectorMux class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_IDPORTSELECTORMUX_H_)
#define _IDPORTSELECTORMUX_H_

#include "IDPortSelector.h"

class LEDevice;

class IDPortSelectorMux : public IDPortSelector
{
public:

	// Crea un multiplexor configurado para habilitar 1 de '2^size' salidas
	// a partir de 'size' entradas
	IDPortSelectorMux( int size );
	virtual ~IDPortSelectorMux();

	// Acceso a los puntos de conexión de la interfaz del módulo (entradas y salidas)
	virtual LEConnectionPoint * enable( unsigned int i=0 );
	virtual LEConnectionPoint * in( unsigned int i );
	virtual LEConnectionPoint * out( unsigned int i );

	// Instanciación
	virtual bool create( LogicEditor * canvas, int left, int top );

	// Geometría
	virtual int width() const{ return w; }
	virtual int height() const{ return h; }
	virtual int left() const{ return l; }
	virtual int top() const{ return t; }

private:
	LEDevice * mux;	
	
	// Geometría
	int l, t, w, h;

};

#endif 
