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



#if !defined(_PROPERTIESBOX_H_)
#define _PROPERTIESBOX_H_

#include <qtable.h>
#include <qvariant.h>

class QObject;

class PropertiesBox : public QTable
{
Q_OBJECT

public:
	PropertiesBox( QWidget * parent = 0, const char * name = 0, WFlags f = 0 );
	
	// Elemento mostrado
	QObject * targetItem() const;

	// Tipo de la propiedad en la fila row
	QVariant::Type propertyType( int row );

public slots:
	// Cambia el elemento mostrado (Si se indica NULL, no se muestra ningún elemento)
	void setTargetItem( QObject* item );
	
  // Manejadores de eventos
	// Celda modificada
	void onValueChanged( int row, int col );

	// Celda doble-clicada
	void onDoubleClicked( int row, int col, int button, const QPoint & pos );

signals:
	// Se emite cuando el usuario cambia el valor de una propiedad y éste es válido y se
	// corresponde con el tipo.
	void propertieChanged( QObject * item, const QString& propertie, const QVariant& value );
	
private:
	// Actualiza la lista de propiedades (según targetItem)
	bool updateProperties();
	
	// Actualiza el valor de la propiedad en la fila row [asociada a mp]
	bool updateProperty( int row );
	bool updateProperty( int row, const QMetaProperty * mp );

	QObject * target;

};

#endif
