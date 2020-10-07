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



#include "PropertiesBox.h"
#include <qmetaobject.h>
#include <qfontdialog.h>
#include <qcolordialog.h>

//////////////////////////////////////////////////////////////////////
// class PropertiesBox
//////////////////////////////////////////////////////////////////////
PropertiesBox::PropertiesBox( QWidget * parent, const char * name, WFlags f )
	: QTable( parent, name )
{
	target = NULL;

	// Aspecto
	setNumCols( 2 );
	horizontalHeader()->setLabel( 0, tr("Propiedad") );
	horizontalHeader()->setLabel( 1, tr("Valor") );
	verticalHeader()->hide();
	setLeftMargin(0);

	setColumnStretchable( 0, true );
	setColumnStretchable( 1, true );

	// Funcionamiento
	setColumnReadOnly( 0, true );
	connect( this, SIGNAL(valueChanged(int, int)), this, SLOT(onValueChanged(int, int)) );
	connect( this, SIGNAL(doubleClicked(int, int, int, const QPoint&)), this, SLOT(onDoubleClicked(int, int, int, const QPoint&)) );

}
	
//////////////////////////////////////////////////////////////////////
// Edición del Item
//////////////////////////////////////////////////////////////////////

// Elemento mostrado
QObject * PropertiesBox::targetItem() const
{
	return target;
}


// Cambia el elemento mostrado (Si se indica NULL, no se muestra ningún elemento)
void PropertiesBox::setTargetItem( QObject * item )
{
	target = item;
	updateProperties();
}

//////////////////////////////////////////////////////////////////////
// Información sobre las propiedades
//////////////////////////////////////////////////////////////////////
QVariant::Type PropertiesBox::propertyType( int row )
{
	// Obtenemos el nombre de la propiedad
	QString propertyName = item( row, 0 )->text();

	// Obtenemos la información del metaobjeto
	QMetaObject * metaObject = targetItem()->metaObject();
	const QMetaProperty * mp = metaObject->property( metaObject->findProperty( propertyName, true ), true );
	if( !mp )
		return QVariant::Invalid;

	return QVariant::nameToType( mp->type() );
}

//////////////////////////////////////////////////////////////////////
// Edición de las propiedades
//////////////////////////////////////////////////////////////////////

// Actualiza la lista de propiedades (según targetItem)
bool PropertiesBox::updateProperties()
{
	int i, nProps;

	// Limpiamos la lista
	int count = numRows();
	for( i=0; i < count; i++)
		removeRow(0);

	if( !targetItem() )
		return true;
	
	// Obtenemos la información para la nueva lista
	QMetaObject * metaObject= targetItem()->metaObject();
	nProps = metaObject->numProperties( true );
	
	// Dimensionamos la lista
	insertRows( 0, nProps );
	
	// Generamos los elementos de la lista
	for( i=0; i < nProps; i++ ){
		const QMetaProperty * mp = metaObject->property( i, true );

		// Nombre de la propiedad
		setText( i, 0, mp->name() );

		// Valor de la propiedad
		updateProperty( i, mp );
	}

	return true;
}

// Actualiza el valor de la propiedad ubicada en la fila 'row'
bool PropertiesBox::updateProperty( int row )
{
	// Obtenemos el nombre de la propiedad
	QString propertyName = item( row, 0 )->text();

	// Obtenemos la información del metaobjeto
	QMetaObject * metaObject = targetItem()->metaObject();
	const QMetaProperty * mp = metaObject->property( metaObject->findProperty( propertyName, true ), true );
	if( !mp )
		return false;
	
	return updateProperty( row, mp );

}

bool PropertiesBox::updateProperty( int row, const QMetaProperty * mp )
{

	// Preparamos tipos de datos
	QStringList booleanValues;
	booleanValues.append( tr("true") );
	booleanValues.append( tr("false") );


	// Valor de la propiedad
	if( mp->isEnumType() ){
		// Para enumeraciones

		QComboTableItem * it = new QComboTableItem( this, QStringList::fromStrList(mp->enumKeys()), false );
		setItem( row, 1, it );
		it->setCurrentItem( targetItem()->property( mp->name() ).toInt() );

	}else if( ! QString::compare( mp->type(), "bool" ) ){
		// Para booleanos

		QComboTableItem * it = new QComboTableItem( this, booleanValues, false );
		setItem( row, 1, it );
		if( targetItem()->property( mp->name() ).toBool() )
			it->setCurrentItem( 0 );
		else
			it->setCurrentItem( 1 );

	}else if( targetItem()->property( mp->name() ).canCast( QVariant::String ) ){
		// Para todo tipo representable como QString
		setText( row, 1, targetItem()->property( mp->name() ).toString() );

	}

	return true;
}

// Manejadores de eventos:

// Gestiona el cambio de valores en las propiedades
//   Realiza una validación del nuevo valor basada en tipo
void PropertiesBox::onValueChanged( int row, int col )
{
	QString prop;
	int propIndex;
	const QMetaProperty * mProp;
	QVariant value, oldValue;
	
 // Nombre de la propiedad
	prop = text( row, 0 );

	// Obtenemos el indice de la propiedad en el metaobjeto (y lo validamos)
	propIndex = targetItem()->metaObject()->findProperty( prop, true );
	if( propIndex < 0 )
		return;

	// Obtenemos la metapropiedad asociada (y la validamos)
	mProp = targetItem()->metaObject()->property( propIndex, true );
		if( !mProp )
			return;

	// Obtenemos el antiguo valor de la propiedad
	oldValue = targetItem()->property( prop );

 // Valor de la propiedad
	QTableItem * it = item( row, col );
	
	// Obtención del valor de la propiedad en QTableItem
	switch( it->rtti() ){
		case 1:
		{ // Se trata de un QComboTableItem (propiedad tipo enum)

			QComboTableItem * cbIt = (QComboTableItem *) it;

			value = cbIt->currentText();
			
			if( !QString::compare( value.toString(), oldValue.toString() ) ){
				cbIt->setCurrentItem( oldValue.toString() );
				return;
			}

		break;
		}
		default:
		{ // Valor manejado por QVariant
			
			// Obtenemos el valor y lo validamos
			value = it->text();

			if( !value.canCast( QVariant::nameToType(mProp->type())) ){
				it->setText( oldValue.toString() );
				return;
			}
		}
	}

	// Asignación efectiva del valor de la propiedad
	targetItem()->setProperty( prop, value );

	// Actualizamos el valor mostrado, ya que el introducido por el
	// usuario puede haber sido rechazado por el objeto.
	updateProperty( row, mProp );
	
}

void PropertiesBox::onDoubleClicked( int row, int col, int button, const QPoint & pos )
{
	if( row >= numRows() || col >= numCols() )
		return;

	QVariant::Type t = propertyType( row );
	QVariant value;
	bool ok = false;

	// Proceso de formatos especiales:
	switch( t ){
		case QVariant::Font:
			// Anula la edición normal
			endEdit( row, col, true, false );
			
			value = QFontDialog::getFont( &ok, QFont( item( row, 1 )->text() ), this, "FontDialog" );
			break;

		case QVariant::Color:
			// Anula la edición normal
			endEdit( row, col, true, false );

			value = QColor( QColorDialog::getRgba( QVariant(item(row, 1)->text()).toColor().rgb(), &ok , this, "ColorDialog" ) );
			break;
	}

	if( ok ){
		item( row, 1 )->setText( value.toString() );
		emit valueChanged( row, col );
	}

}
