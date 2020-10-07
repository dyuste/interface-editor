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



#if !defined(_IDINTERFACESELECTORFUNCTION_H_)
#define _IDINTERFACESELECTORFUNCTION_H_

class LogicEditor;
class LEConnectionPoint;
class LEDevice;

#include "IDInterfaceSelector.h"

class IDInterfaceSelectorFunction : public IDInterfaceSelector
{
public:
	IDInterfaceSelectorFunction( unsigned int pattern, unsigned int length );
	virtual ~IDInterfaceSelectorFunction();

	// Acceso a la interfaz del módulo
	virtual LEConnectionPoint * in( unsigned int i );
	virtual LEConnectionPoint * out( unsigned int i=0 );
	virtual unsigned int inCount() const{ return cpInput?length():0; }
	virtual unsigned int outCount() const{ return devOr?1:0; }

	// Instanciación
	virtual bool create( LogicEditor * editor, int left, int top );

	// Geometría
	virtual int width() const{ return w; }
	virtual int height() const{ return h; }
	virtual int left() const{ return l; }
	virtual int top() const{ return t; }

private:
	LEConnectionPoint **cpInput;
	LEDevice **devInv;
	LEDevice **devNand;
	LEDevice *devOr;

	// Geometría
	int l, t, w, h;
};

#endif 
