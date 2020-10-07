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



#if !defined(_LELABEL_H_)
#define _LELABEL_H_

#include "LEItem.h"

#include "LErtti.h"

class LELabel : public LEItem  
{
	Q_OBJECT
	Q_PROPERTY( QString text READ text WRITE setText )
	Q_PROPERTY( QFont font READ font WRITE setFont )
	Q_PROPERTY( QColor fontColor READ fontColor WRITE setFontColor )
	Q_PROPERTY( QColor backgroundColor READ backgroundColor WRITE setBackgroundColor )
	Q_PROPERTY( QColor lineColor READ lineColor WRITE setLineColor )
	Q_PROPERTY( int lineWidth READ lineWidth WRITE setLineWidth )
	Q_PROPERTY( TextDirection direction READ direction WRITE setDirection )
	Q_ENUMS( TextDirection )

public:
	enum { leftMargin=5, topMargin=2, RTTI=LErttiLabel };
	enum TextDirection { TextVertical=0, TextHorizontal=1 };

	LELabel( QCanvas * canvas=0, LEItem * parentItem=0 );
	LELabel( QString lb, QCanvas * canvas=0, LEItem * parentItem=0 );
	virtual ~LELabel();

	virtual int rtti() const;

	// Texto de la etiqueta
	void setText( const QString &lb );
	QString text() const;

	// Formato del texto
	void setFont( const QFont &font );
	void setFontColor( const QColor &color );
	QFont font() const;
	QColor fontColor() const;

	// Caja envolvente
	void setBackgroundColor( const QColor & color );
	void setLineColor( const QColor & color );
	void setLineWidth( int width );
	QColor backgroundColor() const;
	QColor lineColor() const;
	int lineWidth() const;

	// Ángulo de rotación
	void setDirection( TextDirection d );
	TextDirection direction() const;
	
	// Geometría externa
	virtual QPointArray areaPoints() const;
	virtual QRect boundingRect() const;

private:
	int minWidth() const;
	int minHeight() const;
	void adjustSize();

protected:
	virtual void drawShape( QPainter& p );
	virtual void setSize( int w, int h );
	
private:
	QString label;
	QColor fntColor;
	QFont fnt;
	TextDirection txDirection;
	bool autoSize;
	bool drawBox;
};

#endif 
