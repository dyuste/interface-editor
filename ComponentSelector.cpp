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



#include "ComponentSelector.h"

#include <qpainter.h>
#include <qfontmetrics.h>
#include <qapplication.h>
#include "LMComponent.h"


#ifndef max
	#define max(A,B) ((A)>(B))?(A):(B)
#endif

//////////////////////////////////////////////////////////////////////
// CSItem : Elemento de la lisa
//////////////////////////////////////////////////////////////////////

CSItem::CSItem( LMComponent * cmp )
	: QListBoxItem()
{
	this->cmpnt = cmp;
	
	// Contamos los pins
	cI=0, cO=0, cIO=0;
	QValueList<LMPinDescription>::iterator it;
	for( it = cmp->pinList().begin(); it != cmp->pinList().end(); ++it ){
		switch( (*it).accessMode() ){
		case LEPin::Input:
			cI++;
			break;
		case LEPin::Output:
			cO++;
			break;
		default:
			cIO++;
		}
	}
}

LMComponent * CSItem::component() const
{
	return cmpnt;
}

int CSItem::height( const QListBox * lb ) const
{
	int rows=0;
	if( cI )
		++rows;
	if( cO )
		++rows;
	if( cIO )
		++rows;

	return lb->fontMetrics().height()*(rows+1)+20+(rows*2);
}

int CSItem::width( const QListBox * lb ) const
{
	return max( lb->fontMetrics().width( component()->name() ), lb->fontMetrics().width( "Out pins: ##") ) + 20;
}

void CSItem::paint( QPainter * p )
{
	// Etiqueta e información
	QFont fnt;
	int yOffset=15;

	if( isSelected() ){
		float h = (float)height(listBox());
		for( int y=0; y < (int)h; y++ ){
			p->setPen( qRgb( 220- (30.*(((float)y)/h)), 230- (30.*(((float)y)/h)), 240 ) );
			p->drawLine( 0,y, listBox()->contentsWidth(),y );
		}
	}

	p->setPen( Qt::DotLine );
	p->drawLine( 0,height(listBox())-1, listBox()->contentsWidth(),height(listBox())-1 );
	
	
	// Nombre
	fnt = p->font();
	fnt.setBold( true );
	p->setFont( fnt );
	p->setPen( QApplication::palette().active().text() );
	
	p->drawText( 10, yOffset, component()->name() );
	
	// Conectividad
	QFontMetrics fm = p->fontMetrics();
	yOffset += fm.height()+5;
	
	fnt.setBold( false );
	p->setFont( fnt );
	fm = p->fontMetrics();

	if( cI ){
		p->drawText( 20, yOffset, QString("Input pins: %1").arg(cI) );
		yOffset += fm.height()+2;
	}

	if( cO ){
		p->drawText( 20, yOffset, QString("Output pins: %1").arg(cO) );
		yOffset += fm.height()+2;
	}

	if( cIO )
		p->drawText( 20, yOffset, QString("I/O pins: %1").arg(cIO) );
	
}

//////////////////////////////////////////////////////////////////////
// ComponentSelector : Lista de componentes (Widget)
//////////////////////////////////////////////////////////////////////

ComponentSelector::ComponentSelector( QWidget * parent, const char * name, WFlags f )
	: QToolBox( parent, name, f )
{
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );
}

ComponentSelector::~ComponentSelector()
{

}

void ComponentSelector::insertLibrary( LMLibrary * library )
{
	QListBox * lb = new QListBox( this );

	// Añadimos los componentes
	for( LMLibrary::iterator it = library->begin(); it != library->end(); ++it ){
		LMComponent &cmp = *it;
		CSItem * item = new CSItem( &cmp );
		lb->insertItem( item );
	}

	// Conectamos los SLOTS de la nueva lista
	connect( lb, SIGNAL(clicked(QListBoxItem*)), this, SLOT(onItemClicked(QListBoxItem*)) );

	// Añadimos la lista
	addItem( lb, library->name() );

}

void ComponentSelector::onItemClicked( QListBoxItem * item )
{
	CSItem * csItem = (CSItem*)item;
	emit componentClicked( csItem->component() );
}