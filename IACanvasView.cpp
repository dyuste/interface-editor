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



#include "IACanvasView.h"

#include "IACanvasRectangle.h"

IACanvasView::IACanvasView( QCanvas * canvas, QWidget * parent, const char * name, WFlags f )
	: QCanvasView( canvas, parent, name, f )
{
	selItem = NULL;
	QCanvasText * te;
	for( Q_UINT32 i=0x0000; i < 0x002F; i++ ){
		te = new QCanvasText( QString::number(i, 16).upper().rightJustify( 4, '0' ), canvas );
		te->move( 0, i*this->fontMetrics().lineSpacing() );
		te->show();
	}
}	

IACanvasRectangle * IACanvasView::selectedItem()
{
	return selItem;
}

void IACanvasView::setSelectedItem( IACanvasRectangle * it )
{
	QBrush newBrush, oldBrush;

	IACanvasRectangle * old = selItem;
	
	// Selección efectiva
	selItem = it;
	
	// Resaltamos la nueva selección
	if( selItem ){
		oldBrush = selItem->brush();
		newBrush = QBrush( QColor( oldBrush.color().red()-20, oldBrush.color().green()-15, oldBrush.color().blue() ) );
		selItem->setBrush( newBrush );
	}

	// Restauramos la antigua selección
	if( old ){
		oldBrush = old->brush();
		newBrush = QBrush( QColor( oldBrush.color().red()+20, oldBrush.color().green()+15, oldBrush.color().blue() ) );
		old->setBrush( newBrush );
	}

	// Retransmisión del evento
	if( old != it )
		emit selected( it );

	canvas()->update();
}

void IACanvasView::contentsMousePressEvent( QMouseEvent * e )
{
	QCanvasItemList clist= canvas()->collisions( e->pos() );
	if( clist.begin() == clist.end() ){
		if( selectedItem() ){
			setSelectedItem( NULL );
		}
	}else{
		setSelectedItem( (IACanvasRectangle*)*clist.begin() );
	}

}
	