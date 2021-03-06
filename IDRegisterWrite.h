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



#if !defined(_IDGEGISTERWRITE_H_)
#define _IDGEGISTERWRITE_H_

#include "IDRegister.h"

class QString;
class LEDevice;

class IDRegisterWrite : public IDRegister
{
public:
	IDRegisterWrite( unsigned int sz=1 );
	virtual ~IDRegisterWrite();

	// Acceso a los puntos de conexión de la interfaz del módulo (entradas y salidas)
	// NOTA: Esta implementación trabaja siempre con registros de 8 bits (ignora el valor size())
	virtual unsigned int inCount() const{ return 8; }
	virtual unsigned int outCount() const{ return 8; }
	virtual LEConnectionPoint * in( unsigned int i );
	virtual LEConnectionPoint * out( unsigned int i );

	// Señal de habitilitación
	// NOTA: Esta implementación acepta una habilitación por BE y 
	//       otra de carácter general (selección de palabra ie)
	virtual LEConnectionPoint * enable( unsigned int i );
	virtual unsigned int enableCount() const{ return 2; }

	// Instanciación
	virtual bool create( LogicEditor * canvas, int left, int top );
	
	// Geometría
	virtual int width() const{ return w; }
	virtual int height() const{ return h; }
	virtual int left() const{ return l; }
	virtual int top() const{ return t; }

private:
	LEDevice *_or, *_reg;

	// Geometría
	int l, t, w, h;
};

#endif 
