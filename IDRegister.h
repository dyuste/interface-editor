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


// IDRegister.h: interface for the IDRegister class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IDREGISTER_H_
#define _IDREGISTER_H_

#include "IDComponent.h"

class IDRegister : public IDComponent
{
public:
	IDRegister( unsigned int sz=1 ){ setSize(sz); }
	virtual ~IDRegister();

	// Acceso a los puntos de conexión de la interfaz del módulo (entradas y salidas)
	virtual unsigned int inCount() const{ return 8*size(); }
	virtual unsigned int outCount() const{ return 8*size(); }

	// Señal de habitilitación (por defecto, una no implementada)
	virtual LEConnectionPoint * enable( unsigned int i ){ return 0; }
	virtual unsigned int enableCount() const{ return 1; }

	// Instanciación
	virtual bool create( LogicEditor * canvas, int left, int top )=0;

	// Acceso a atributos
	inline unsigned int size() const{ return sz; }
	inline void setSize( unsigned int sz ){ this->sz = sz; }

private:
	unsigned int sz;
};

#endif 
