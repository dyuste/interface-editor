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



#if !defined(_IDPORTSELECTOR_H_)
#define _IDPORTSELECTOR_H_

#include "IDComponent.h"

class IDPortSelector : public IDComponent
{
public:

	// Crea un multiplexor configurado para habilitar 1 de '2^size' salidas
	// a partir de 'size' entradas
	IDPortSelector( unsigned int size );
	virtual ~IDPortSelector();

	// Acceso a la interfaz del módulo
	virtual unsigned int inCount() const{ return sz; }
	virtual unsigned int outCount() const;
	virtual unsigned int enableCount() const{ return 1; }
	
	// Acceso a atributos
	inline unsigned int size() const{ return sz; }
	inline void setSize( unsigned int size ){ this->sz = size; }

private:
	unsigned int sz;
};

#endif 