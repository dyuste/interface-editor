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



#include "LEItem.h"

#include "LogicEditor.h"

//////////////////////////////////////////////////////////////////////
// Instanciación
//////////////////////////////////////////////////////////////////////
LEItem::LEItem( QCanvas * canvas, LEItem * parentItem )
	: QCanvasPolygonalItem( canvas )
{
	setParent( parentItem );
	setSize( 1, 1 );
	setResizable( true );
}

LEItem::~LEItem()
{
	hide();
}

void LEItem::setName( const char * name )
{
	static int nameCount=0;
	QString finalName;

	// Establece el nombre del objeto si no se vulnera la unicidad de nombres
	if( canvas() && canvas()->parent() && canvas()->parent()->inherits( "LogicEditor" ) ){
		LogicEditor * lpLE = (LogicEditor*) canvas()->parent();

		if( lpLE->findItem( this->name() ) ){
			
			// El item ha ya sido registrado, se procede a cambiar su nombre
			if( lpLE->tryUpdateItemName( this->name(), name ) )
				QObject::setName( name );
			else
				qDebug( tr("LEItem::setName: ! Imposible cambiar el nombre del objeto, violación de unicidad.") );

		}else{

			// Primera asignación de nombre para el item:
			if( lpLE->registerItem(this, name) ){
				// Se establece el nombre indicado en la llamada
				finalName = name;
			}else{
				// El nombre asignado no es posible, se consigue otro:
				finalName = name+QString::number(++nameCount);
				while( !lpLE->registerItem(this, finalName ) )
					finalName = name+QString::number(++nameCount);
			}
		
			// Establece el nombre definitivo del objeto
			QObject::setName( finalName );

		}
	}else{
		qDebug( tr("LEItem::setName: ! El canvas del objeto '%1' no tiene como padre un objeto LogicEditor. No se garantiza la unicidad de nombres.") );
		QObject::setName( name );
	}

}
void LEItem::remove()
{ 
	LEItem *item, *lastItem;
	
	// Borrado en cascada.
	// Este borrado contempla la reentrancia de la lista: las clases hijas
	//  se eliminan de la lista childList al ser eliminadas
	item = childList.first();
	while( item )
	{
		lastItem = item;

		// Recursión del borrado
		item->remove();

		// Actualizamos item tras la reentrancia
		item = childList.current();

		// No ha habido reentrancia
		if( item == lastItem )
			item = childList.next();
	}
	
	// El item se elimina de la lista de hijos de su padre
	if( parent() )
		parent()->removeChild( this );
	
	delete this;
}

int LEItem::rtti() const
{
	return RTTI;
}
	
//////////////////////////////////////////////////////////////////////
// Características de objeto
//////////////////////////////////////////////////////////////////////
bool LEItem::setProperty( const char * name, const QVariant & value )
{
	
	bool retval = QObject::setProperty( name, value );
	
	// Actualizamos el CanvasView (si es del tipo LogicEditor)
	//  NOTA: Esto solo es posible si se ha establecido como padre del canvas
	if( canvas()->parent()->inherits( "LogicEditor" ) ){
		LogicEditor * lpLE = (LogicEditor*) canvas()->parent();

		// Refrescamos los aspectos de visualización del item 
		//  (como la ubicación de los handlers)
		if( lpLE->activeItem() == this )
			lpLE->setActiveItem( this );
	}

	// Actualización del lienzo
	canvas()->update();
	
	return retval;
}

//////////////////////////////////////////////////////////////////////
// Aspecto
//////////////////////////////////////////////////////////////////////
void LEItem::show()
{
	QCanvasPolygonalItem::show();	
}

void LEItem::hide()
{
	QCanvasPolygonalItem::hide();
}

void LEItem::setResizable( bool rs )
{
	resizable = rs;
}

bool LEItem::isResizable() const
{
	return resizable;
}

void LEItem::setSize( int w, int h )
{
	this->w=w; this->h=h;
}

int LEItem::width() const
{
	return w;
}

int LEItem::height() const
{
	return h;
}

//////////////////////////////////////////////////////////////////////
// Parentesco
//////////////////////////////////////////////////////////////////////
void LEItem::setParent( LEItem * item )
{
	parentItem = item;

	if( item )
		item->addChild( this );
}

LEItem * LEItem::parent()
{
	return this->parentItem;
}

const LEItem * LEItem::parent() const
{
	return (const LEItem*)this->parentItem;
}

void LEItem::addChild( LEItem * item )
{
	childList.append( item );
}

void LEItem::removeChild( LEItem * item )
{
	childList.removeRef( item );
}

LEItemList& LEItem::childs()
{
	return childList;
}

QString LEItem::resolvName( char separator ) const
{
	if( parent() )
		return QString("%1%2%3").arg( parent()->resolvName() )
								.arg( separator )
								.arg( name() );
	else
		return name();
}

//////////////////////////////////////////////////////////////////////
// Conectividad
//////////////////////////////////////////////////////////////////////
void LEItem::connectionMoved( LEConnectionPoint * connection )
{}
void LEItem::connectionRemoved( LEConnectionPoint * connection )
{}
