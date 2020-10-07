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


// LMComponent.cpp: implementation of the LMComponent class.
//
//////////////////////////////////////////////////////////////////////

#include "LMComponent.h"
#include "LMLibrary.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LMComponent::LMComponent()
{
	nm = QString::null;
}

LMComponent::LMComponent( LMLibrary * parent )
{
	nm = QString::null;
	setParentLibrary( parent );
}


//////////////////////////////////////////////////////////////////////
// Enlace con la librería padre
//////////////////////////////////////////////////////////////////////

LMLibrary * LMComponent::parentLibrary()const
{
	return lib;
}

void LMComponent::setParentLibrary( LMLibrary * parent )
{
	this->lib = parent;
}

// Determina si el componente se resuelve interna o externamente
// recurriendo a su librería asociada
bool LMComponent::isExternSolving() const
{
	if( !parentLibrary() ){
		qWarning( "Precaución: El componente '" + name() +"' no está vinculado a ninguna librería." );
		return false;
	}

	if( parentLibrary()->sourceType() == LMLibrary::HDL )
		return false;
	else
		return true;
}
	
//////////////////////////////////////////////////////////////////////
// Acceso a los atributos
//////////////////////////////////////////////////////////////////////

void LMComponent::setName( const QString &name )
{
	nm = name;
}

QString LMComponent::name() const
{
	return nm;
}

ShapeList& LMComponent::shapeList()
{
	return (ShapeList&)shps;
}

const ShapeList& LMComponent::shapeList() const
{
	return (const ShapeList&)shps;
}

PinList& LMComponent::pinList()
{
	return (PinList&)pins;
}

const PinList& LMComponent::pinList() const
{
	return (const PinList&)pins;
}


//////////////////////////////////////////////////////////////////////
// Carga XML
//////////////////////////////////////////////////////////////////////

// Construye el componente a partir de una entrada XML
bool LMComponent::parse(const QDomElement &element)
{
	// Cargamos el atributo 'name' del componente
	QString name = element.attribute( "name" );
	if( name.isNull() ){
		qWarning( "Error cargando componente: No se ha especificado el atributo 'name' de 'component'" );
		return false;
	}

	setName( name );

	qDebug("Cargando componente %s", name.latin1());

	// Cargamos los elementos del componente
	QDomNode node = element.firstChild();
	while( !node.isNull() ){
		if( node.toElement().tagName() == "shape" ){
			
			// Cargamos la geometría
			QDomNode childNode = node.firstChild();
			while( !childNode.isNull() ){

				if( childNode.nodeType() == QDomNode::TextNode ){
					
					QPointArray points;
					if( !parseShapeString( childNode.toText().data(), points ) ){
						qWarning( "Error cargando componente: Imposible cargar <shape>");
						return false;
					}
					
					shps.append( points );
				}

				childNode = childNode.nextSibling();
			}
		
		}else if( node.toElement().tagName() == "pin" ){
			
			LMPinDescription pin;

			// Cargamos la descripción de pin
			if( !parsePinDescription( node, pin ) ){
				qWarning( "Error cargando componente: Imposible cargar la <pin>");
				return false;
			}

			// Insertamos el pin
			pins.append( pin );

		}else{
			qDebug( "LMComponent::parse: Elemento no identificado" );
		}

		node = node.nextSibling();
	}

	return true;
}


// Transforma una secuencia de números separados por espacios
// en un point array
bool LMComponent::parseShapeString(const QString &shape, QPointArray & points )
{
	QString strShape = shape.stripWhiteSpace();
	QString number;
	int i = 0;

	QStringList numbers = QStringList::split(" ", strShape);
	if (numbers.count() < 2 || numbers.count() % 2 != 0) {
		qWarning ("Definicion incorrecta de geometria");
		return true;
	}
		

	for (QStringList::Iterator it = numbers.begin(); it != numbers.end();) { 
		int x, y;
		
		x = (*it).toInt();
		++it;
		y = (*it).toInt();
		++it;

		i += 2;
		points.putPoints( (i/2)-1, 1, x, y );
	}

	return true;
}

// Extrae la descripción de Pin del tag node
bool LMComponent::parsePinDescription( QDomNode &node, LMPinDescription & pin ) const
{
	
	QString data;

	// Nombre del pin
	data = node.toElement().attribute( "name" );
	if( data.isNull() )
		return false;
	pin.setName( data );

	// Modo de acceso (I/O por omisión)
	pin.setAccessMode( LEPin::InputOutput );

	data = node.toElement().attribute( "accessmode" );
	if( !data.isNull() ){
		if( !QString::compare( data.lower(), "input" ) )
			pin.setAccessMode( LEPin::Input );
		else if( !QString::compare( data.lower(), "output" ) )
			pin.setAccessMode( LEPin::Output );
	}	
	
	// Nivel activo (HiLevel por omisión)
	pin.setActiveLevel( LEPin::HiLevel );
	data = node.toElement().attribute( "activelevel" );
	if( !data.isNull() ){
		if( !QString::compare( data.lower(), "hi" ) )
			pin.setActiveLevel( LEPin::HiLevel );
		else if( !QString::compare( data.lower(), "low" ) )
			pin.setActiveLevel( LEPin::LowLevel );
	}

	// Alineación
	pin.setAlignment( LEPin::Left );
	data = node.toElement().attribute( "alignment" );
	if( !data.isNull() ){	
		if( !QString::compare( data.lower(), "right" ) )
			pin.setAlignment( LEPin::Right );
		else if( !QString::compare( data.lower(), "top" ) )
			pin.setAlignment( LEPin::Top );
		else if( !QString::compare( data.lower(), "bottom" ) )
			pin.setAlignment( LEPin::Bottom );
	}

	// Posición relativa
	pin.setPosition( -1 );
	data = node.toElement().attribute( "position" );
	if( !data.isNull() ){
		bool ok;
		float val = data.toFloat( &ok );
		if( ok )
			pin.setPosition( val );
	}
	
	return true;
}
