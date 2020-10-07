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



#if !defined(_LMCOMPONENT_H_)
#define _LMCOMPONENT_H_

#include <qvaluelist.h>
#include <qpointarray.h>
#include <qdom.h>

#include "LMPinDescription.h"


typedef QValueList<QPointArray> ShapeList;
typedef QValueList<LMPinDescription> PinList;

class LMLibrary;

class LMComponent  
{
public:
	LMComponent();
	LMComponent( LMLibrary * parent );

	// Enlace con la librería padre
	LMLibrary * parentLibrary() const;
	void setParentLibrary( LMLibrary * parent );

	// Determina si el componente se resuelve interna o externamente
	// recurriendo a su librería asociada
	bool isExternSolving() const;

	// Acceso a atributos
	void setName( const QString &name );
	QString name() const;
	ShapeList& shapeList();
	const ShapeList& shapeList() const;
	PinList& pinList();
	const PinList& pinList() const;

	// Construye el componente a partir de una entrada XML
	bool parse(const QDomElement &element);

	// Transforma una secuencia de números separados por espacios
	// en un point array
	static bool parseShapeString(const QString &shape, QPointArray & points );

private:
	

	// Extrae la descripción de Pin del tag node
	bool parsePinDescription( QDomNode &node, LMPinDescription & pin ) const;

	QString nm;
	ShapeList shps;
	PinList pins;
	LMLibrary * lib;
};

#endif
