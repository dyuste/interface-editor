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



#include "LEDevice.h"

#include "LELabel.h"

#include <qpainter.h>
#include <math.h>


LEDevice::LEDevice( QCanvas * canvas, LEItem * parentItem )
	: LEItem( canvas, parentItem )
{
	label = NULL;
	cmp = NULL;
	hDir = LeftToRight;
	vDir = TopToBottom;
	setSize( 0, 0 );
	transpos = false;
}

LEDevice::~LEDevice()
{
	LEItem::hide();
}

void LEDevice::setName( const char * name )
{
	LEItem::setName( name );

	if( isExternSolving() )
		setLabel( this->name() );
}

void LEDevice::setComponentReference( const LMComponent * cmp )
{
	this->cmp = cmp;
	if( isExternSolving() )
		setLabel( name() );
}

const LMComponent * LEDevice::componentReference() const
{
	return cmp;
}

bool LEDevice::isExternSolving() const
{
	if( (const LMComponent*)componentReference() )
		return componentReference()->isExternSolving();
	else
		return false;
}


////////////////////////////////////////////////////////////////
//	Comportamiento
////////////////////////////////////////////////////////////////

int LEDevice::rtti() const
{
	return RTTI;
}

void LEDevice::hide()
{
	if( label )
		label->hide();

	for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
		aux->hide();

	LEItem::hide();
}

void LEDevice::show()
{
	if( label )
		label->show();

	for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
		aux->show();

	LEItem::show();
}


//////////////////////////////////////////////////////////////
//	Geometría
//////////////////////////////////////////////////////////////

void LEDevice::setSize ( int w, int h )
{
	if( !isResizable() ){
		qDebug( tr("LEDevice::setSize(int, int): Se ignora. Objeto no redimensionable.") );
		return;
	}
	
	// Invalidamos el area para forzar el redibujado en el próximo update()
	invalidate();

	// Actualización de atributos
	LEItem::setSize( w, h );
	

	// Redimensionamos scaledShape si se ha fijado un tamaño en size()
	float mainw = (float)mainShape.boundingRect().width()-1;
	float mainh = (float)mainShape.boundingRect().height()-1;
	if( width() != 0 && height() != 0 && mainw != .0 && mainh != .0)
	{
		float hRatio = ((float)width()) / mainw;
		float vRatio = ((float)height()) / mainh;

		for( int i=0; i < mainShape.count(); i++ )
		{
			scaledShape[i].setX( (int)( ((float)mainShape[i].x())*hRatio ) );
			scaledShape[i].setY( (int)( ((float)mainShape[i].y())*vRatio ) );
		}
	}
	
	// Reubicamos los puntos de la figura
	scaledShape.translate( this->x(), this->y() );

	// Reubicamos los pins
	for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
		placePin( aux );

	// Reubicamos la etiqueta
	if( label != NULL ){
		if( label->direction() == LELabel::TextHorizontal )
			label->move( this->x() +width()  - label->width(), this->y() + height() );
		else
			label->move( this->x() +width(), this->y() + height() - label->height() );
	}

	update();
}

void LEDevice::move( double x, double y )
{	
	
	double deltaX = x - this->x();
	double deltaY = y - this->y();

	QCanvasItem::moveBy( deltaX, deltaY );
	
	// Reubicamos la figura escalada
	invalidate();
	scaledShape.translate( deltaX, deltaY );
	update();

	// Reubicamos los pins
	for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
		placePin( aux );

	// Reubicamos la etiqueta
	if( label != NULL ){
		if( label->direction() == LELabel::TextHorizontal )
			label->move( this->x() +width()  - label->width(), this->y() + height() );
		else
			label->move( this->x() +width(), this->y() + height() - label->height() );
	}

}

void LEDevice::moveBy( double x, double y )
{
	QCanvasItem::moveBy( x, y );

	// Reubicamos la figura escalada
	invalidate();
	scaledShape.translate( x, y );
	update();

	// Reubicamos los pins
	for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
		aux->moveBy( x, y );

	// Reubicamos la etiqueta
	if( label != NULL )
		label->moveBy( x, y );

}

QPointArray LEDevice::areaPoints() const
{
	return bufferPolygon( 3 );
}

QRect LEDevice::boundingRect() const
{
	QRect rv( x()-1, y()-1, width()+2, height()+2 );
	rv.normalize();
	return  rv;
}

// Establece la forma primaria del objeto
void LEDevice::setShape( const QPointArray & shape )
{
	invalidate();

	mainShape = shape;
	scaledShape = shape.copy();
	if( width() == 0 || height() == 0 ){
		// No se ha determinado un tamaño, asignamos el de la nueva figura
		LEItem::setSize( scaledShape.boundingRect().width(), scaledShape.boundingRect().height() );
	}else
		// Existe un tamaño prefijado, escalamos la figura a este tamaño
		LEDevice::setSize( width(), height() );

	update();
}

// Devuelve la forma primaria del objeto
QPointArray & LEDevice::getPrimaryShape()
{
	return mainShape;
}

// Devuelve la versión redimensionada de la forma del objeto
QPointArray & LEDevice::getScaledShape()
{
	return scaledShape;
}


//////////////////////////////////////////////////////////////
//	Geometría: Cálculos geometrícos
//////////////////////////////////////////////////////////////

QPointArray LEDevice::bufferPolygon( double d ) const
{

	// El método empleado es el de intersectar las n rectas asociadas a los n segmentos que forman
	// el polígono 'd' unidades desplazadas. Los puntos de intersección constituyen los nuevos
	// vértices del buffer
	int i;
	QPointArray rv( ((int)scaledShape.count()) );

	double Alst, Blst, Clst, A, B, C, R, S, X, Y;
	int last = scaledShape.count()-1;
	R = scaledShape[last].x() - scaledShape[last-1].x();
	S = scaledShape[last].y() - scaledShape[last-1].y();
	Alst = S / sqrt( R*R + S*S );
	Blst = -R / sqrt( R*R + S*S );
	Clst = Alst*((double)scaledShape[last-1].x())+Blst*((double)scaledShape[last-1].y())+d;
	
	for( i=0; i<last; i++ )
	{
		R = scaledShape[i+1].x() - scaledShape[i].x();
		S = scaledShape[i+1].y() - scaledShape[i].y();
		
		// Vector (A,B) unitario normal al director (R,S) del segmento
		A = S / sqrt( R*R + S*S );
		B = -R / sqrt( R*R + S*S );
		
		// Constante de desplazamiento
		C = A*((double)scaledShape[i].x())+B*((double)scaledShape[i].y())+d;

		// Evitamos división por 0 para lineas Verticales/Horizontales 
		// NOTA: Consideramos Imposible A==B==0, ya que eso supondría haber definido dos puntos consecutivos uno
		//		sobre el otro. Asumimos corrección en la definición.
		if( A == 0 ){
			X = (Clst*B - C*Blst) / (B*Alst-A*Blst);			
			Y = (C - A*X) / B;
		}else{
			Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
			X = (C - B*Y) / A;
		}

		rv.putPoints( i, 1, X, Y );
		Alst = A;
		Blst = B;
		Clst = C;
			
	}

	rv.putPoints( i, 1, rv[0].x(), rv[0].y() );

	return rv;
}

bool LEDevice::getLeftProjection( int* x, int y )
{
	float minX = 3.402823466e+38F;
	float m, X, Y=(float)y;

	// Buscamos el punto más a la izquLErda que intersecta con la horizontal Y='y'
	for( int i=0; i<scaledShape.count()-1; i++ )
		if( (scaledShape[i].y() < y && scaledShape[i+1].y() > y) || (scaledShape[i].y() > y && scaledShape[i+1].y() < y) )
		{
			// Caso segmento: calculamos la intersección lineal
			if( scaledShape[i+1].x() == scaledShape[i].x() )
			{
				// Peculiaridad linea verteical
				if( scaledShape[i+1].x() < minX) minX = scaledShape[i+1].x();

			}
			else
			{
				m = ((float) (scaledShape[i+1].y()-scaledShape[i].y()) ) / ((float) (scaledShape[i+1].x()-scaledShape[i].x()) );
				X = ( Y + m * ((float)scaledShape[i].x())-((float)scaledShape[i].y()) ) / m;
				
				// minimización
				if( X < minX ) minX = X;
			}
		}
		else 
			if( scaledShape[i].y() == y)
			// Caso punto: la proyección coincide con un punto existente
				if( ((float)scaledShape[i].x()) < minX )
					// minimización
					minX = (float)scaledShape[i].x();

	// Caso punto para el último punto (que no es alcanzado en el bucle)
	if( scaledShape[(int)(scaledShape.count()-1)].y() == y)
		if( ((float)scaledShape[(int)(scaledShape.count()-1)].x()) < minX )
			minX = (float)scaledShape[(int)(scaledShape.count()-1)].x();


	// Validación del resultado
	if( minX < 3.402823466e+38F )
	{
		*x = (int)minX;
		return true;
	}
	else
		return false;
}

bool LEDevice::getRightProjection( int* x, int y )
{
	float maxX = -3.402823466e+38F;
	float m, X, Y=(float)y;

	// Buscamos el punto más a la derecha que intersecta con la horizontal Y='y'
	for( int i=0; i<scaledShape.count()-1; i++ )
		if( (scaledShape[i].y() < y && scaledShape[i+1].y() > y) || (scaledShape[i].y() > y && scaledShape[i+1].y() < y) )
		{
			// Caso segmento: calculamos la intersección lineal
			if( scaledShape[i+1].x() == scaledShape[i].x() )
			{
				// Peculiaridad linea verteical
				if( scaledShape[i+1].x() > maxX) maxX = scaledShape[i+1].x();

			}
			else
			{
				m = ((float) (scaledShape[i+1].y()-scaledShape[i].y()) ) / ((float) (scaledShape[i+1].x()-scaledShape[i].x()) );
				X = ( Y + m * ((float)scaledShape[i].x())-((float)scaledShape[i].y()) ) / m;
				
				// maximización
				if( X > maxX ) maxX = X;
			}
		
		}
		else 
			if( scaledShape[i].y() == y)
			// Caso punto: la proyección coincide con un punto existente
				if( ((float)scaledShape[i].x()) > maxX )
					// maximización
					maxX = (float)scaledShape[i].x();

	// Caso punto para el último punto (que no es alcanzado en el bucle)
	if( scaledShape[(int)(scaledShape.count()-1)].y() == y)
		if( ((float)scaledShape[(int)(scaledShape.count()-1)].x()) > maxX )
			maxX = (float)scaledShape[(int)(scaledShape.count()-1)].x();


	// Validación del resultado
	if( maxX > -3.402823466e+38F )
	{
		*x = (int)maxX;
		return true;
	}
	else
		return false;
}

bool LEDevice::getTopProjection( int x, int* y )
{
	float minY = 3.402823466e+38F;
	float m, Y, X=(float)x;

	// Buscamos el punto más a la izquLErda que intersecta con la vertical X='x'
	for( int i=0; i<scaledShape.count()-1; i++ )
		if( (scaledShape[i].x() < x && scaledShape[i+1].x() > x) || (scaledShape[i].x() > x && scaledShape[i+1].x() < x) )
		{
			// Caso segmento: calculamos la intersección lineal (NOTA: el caso linea vertical es ahora un caso particular del Caso punto, luego se omite)
			if( scaledShape[i+1].x() != scaledShape[i].x() )
			{
				m = ((float) (scaledShape[i+1].y()-scaledShape[i].y()) ) / ((float) (scaledShape[i+1].x()-scaledShape[i].x()) );
				Y = (X - ((float)scaledShape[i].x()) ) * m  + ((float)scaledShape[i].y()) ;
				
				// minimización
				if( Y < minY ) minY = Y;
			}
		}
		else 
			if( scaledShape[i].x() == x)
			// Caso punto: la proyección coincide con un punto existente
				if( ((float)scaledShape[i].y()) < minY )
					// minimización
					minY = (float)scaledShape[i].y();

	// Caso punto para el último punto (que no es alcanzado en el bucle)
	if( scaledShape[(int)(scaledShape.count()-1)].x() == x)
		if( ((float)scaledShape[(int)(scaledShape.count()-1)].y()) < minY )
			minY = (float)scaledShape[(int)(scaledShape.count()-1)].y();


	// Validación del resultado
	if( minY < 3.402823466e+38F )
	{
		*y = (int)minY;
		return true;
	}
	else
		return false;
}


bool LEDevice::getBottomProjection( int x, int* y )
{
	float maxY = -3.402823466e+38F;
	float m, Y, X=(float)x;

	// Buscamos el punto más a la izquLErda que intersecta con la vertical X='x'
	for( int i=0; i<scaledShape.count()-1; i++ )
		if( (scaledShape[i].x() < x && scaledShape[i+1].x() > x) || (scaledShape[i].x() > x && scaledShape[i+1].x() < x) )
		{
			// Caso segmento: calculamos la intersección lineal (NOTA: el caso linea vertical es ahora un caso particular del Caso punto, luego se omite)
			if( scaledShape[i+1].x() != scaledShape[i].x() )
			{
				m = ((float) (scaledShape[i+1].y()-scaledShape[i].y()) ) / ((float) (scaledShape[i+1].x()-scaledShape[i].x()) );
				Y = (X - ((float)scaledShape[i].x()) ) * m  + ((float)scaledShape[i].y());
				
				// maximización
				if( Y > maxY ) maxY = Y;
			}
		}
		else 
			if( scaledShape[i].x() == x)
			// Caso punto: la proyección coincide con un punto existente
				if( ((float)scaledShape[i].y()) > maxY )
					// maximización
					maxY = (float)scaledShape[i].y();

	// Caso punto para el último punto (que no es alcanzado en el bucle)
	if( scaledShape[(int)(scaledShape.count()-1)].x() == x)
		if( ((float)scaledShape[(int)(scaledShape.count()-1)].y()) > maxY )
			maxY = (float)scaledShape[(int)(scaledShape.count()-1)].y();


	// Validación del resultado
	if( maxY > -3.402823466e+38F )
	{
		*y = (int)maxY;
		return true;
	}
	else
		return false;
}

// Devuelve una copia de 'shape' a la que se le ha aplicado el efecto espejo horizontal
// NOTA: Para garantizar compatiblidad con el algoritmo bufferPolygon implementado (que
//       espera un polígono con puntos ordenados en sentido horario) se invierte el orden
//       de los puntos.
QPointArray LEDevice::horizontalMirror( const QPointArray & shape ) const
{
	QPointArray retval( shape.count() );

	// Rectángulo envolvente
	QRect bounding = shape.boundingRect();
	int left = bounding.left();
	int right = bounding.right();

	// Conversión de los puntos
	for( int i = 0; i < shape.count(); i++ )
		retval.setPoint( shape.count()-i-1, left + right - shape[i].x(), shape[i].y() );

	return retval;
}

// Devuelve una copia de 'shape' a la que se le ha aplicado el efecto espejo vertical
// NOTA: Para garantizar compatiblidad con el algoritmo bufferPolygon implementado (que
//       espera un polígono con puntos ordenados en sentido horario) se invierte el orden
//       de los puntos.
QPointArray LEDevice::verticalMirror( const QPointArray & shape ) const
{
	QPointArray retval( shape.count() );

	// Rectángulo envolvente
	QRect bounding = shape.boundingRect();
	int top = bounding.top()+1;
	int bottom = bounding.bottom()-1;

	// Conversión de los puntos
	for( int i = 0; i < shape.count(); i++ )
		retval.setPoint( shape.count()-i-1, shape[i].x(), top + bottom - shape[i].y() );

	return retval;
}

// Devuelve una copia de 'shape' en la que se han transpuesto las coordenadas x-y,
// NOTA: Para garantizar compatiblidad con el algoritmo bufferPolygon implementado (que
//       espera un polígono con puntos ordenados en sentido horario) se invierte el orden
//       de los puntos.
QPointArray LEDevice::transposePolygon( const QPointArray & shape ) const
{
	QPointArray retval( shape.count() );

	// Rectángulo envolvente
	QRect bounding = shape.boundingRect();
	int left = bounding.left();
	int top = bounding.top();

	// Conversión de los puntos
	for( int i = 0; i < shape.count(); i++ )
		retval.setPoint( shape.count()-i-1, left+shape[i].y()-top, top+shape[i].x()-left );

	return retval;
}

//////////////////////////////////////////////////////////////
// Orientación (Efecto Mirror)
//////////////////////////////////////////////////////////////
void LEDevice::setHDirection( const HDirection & direction )
{	
	
	if( hDirection() != direction ){
		invalidate();

		// Modificamos las imágenes
		mainShape = horizontalMirror( mainShape );
		scaledShape = horizontalMirror( scaledShape );
		//scaledShape.putPoints( 0, scaledShape.count(), horizontalMirror( scaledShape ) );

		// Actualizamos la alineación y posición de los pins horizontales
		for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
			if( aux->alignment() == LEPin::Left ){
				aux->setAlignment( LEPin::Right );
				placePin( aux );
			}else if( aux->alignment() == LEPin::Right ){
				aux->setAlignment( LEPin::Left );
				placePin( aux );
			}else
				aux->setPosition( 1.0 - aux->position() );

		update();		
	}

	this->hDir = direction;

	canvas()->update();
}

LEDevice::HDirection LEDevice::hDirection() const
{
	return this->hDir;
}

void LEDevice::setVDirection( const VDirection & direction )
{
	
	if( vDirection() != direction ){
		invalidate();

		// Modificamos las imágenes
		mainShape = verticalMirror( mainShape );
		scaledShape = verticalMirror( scaledShape );
		
		// Actualizamos la alineación de los pins horizontales
		for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
			if( aux->alignment() == LEPin::Top ){
				aux->setAlignment( LEPin::Bottom );
				placePin( aux );
			}else if( aux->alignment() == LEPin::Bottom ){
				aux->setAlignment( LEPin::Top );
				placePin( aux );
			}else
				aux->setPosition( 1.0 - aux->position() );
		
		update();
	}

	this->vDir = direction;
}

LEDevice::VDirection LEDevice::vDirection() const
{
	return this->vDir;
}

void LEDevice::setTransposed( bool t )
{
	if( transpos != t ){
		invalidate();

		// Intercambio de coordenadas
		mainShape = transposePolygon( mainShape );
		scaledShape = transposePolygon( scaledShape );

		// Actualizamos atributos
		LEItem::setSize( scaledShape.boundingRect().width(), scaledShape.boundingRect().height() );

		// Actualizamos la alineación de los pins
		if( t )
			for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() ){
				if( aux->alignment() == LEPin::Top )
					aux->setAlignment( LEPin::Right );	
				else if( aux->alignment() == LEPin::Right )
					aux->setAlignment( LEPin::Bottom );
				else if( aux->alignment() == LEPin::Bottom )
					aux->setAlignment( LEPin::Left );
				else if( aux->alignment() == LEPin::Left )
					aux->setAlignment( LEPin::Top );
			}
		else
			for( LEPin * aux = lsPin.first(); aux != NULL; aux = lsPin.next() ){
				if( aux->alignment() == LEPin::Top )
					aux->setAlignment( LEPin::Left );	
				else if( aux->alignment() == LEPin::Right )
					aux->setAlignment( LEPin::Top );
				else if( aux->alignment() == LEPin::Bottom )
					aux->setAlignment( LEPin::Right );
				else if( aux->alignment() == LEPin::Left )
					aux->setAlignment( LEPin::Bottom );
			}

		// Actualizamos la etiqueta
		if( label ){
			if( label->direction() == LELabel::TextHorizontal ){
				label->setDirection( LELabel::TextVertical );
				label->move( this->x() +width(), this->y() + height() - label->height() );
			}else{
				label->setDirection( LELabel::TextHorizontal );
				label->move( this->x() +width()  - label->width(), this->y() + height() );
			}
		}

		update();
	}
	
	transpos = t;
}

bool LEDevice::transposed() const
{
	return transpos;
}

//////////////////////////////////////////////////////////////
//	Manipulación de conectores
//////////////////////////////////////////////////////////////
QPtrList<LEPin>& LEDevice::pinList()
{
	return (QPtrList<LEPin>&)lsPin;
}

LEPin * LEDevice::insertPin( LEPin::Alignment align, LEPin::AccessMode access, QString label, double position, LEPin::Level l )
{
	LEPin * pin = new LEPin( align, access, label, position, l, this->canvas(), this );

	lsPin.append( pin );
	
	if( pin->position() < 0.0 ){
	// El pin debe ser auto-ubicado: esto implica la reubicación de todos los pins que compartan ubicación
		
		int count=0;
		LEPin * aux;

		// Determinamos el número de pins ubicados en la misma orLEntación
		for( aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
			if( aux->alignment() == pin->alignment() )
				count++;
		
		// Equidistribuimos los pins
		if( count == 1)
		{
			pin->setPosition( 0.5 );
			placePin( pin );
		}
		else
		{
			int i=0;
			for( aux = lsPin.first(); aux != NULL; aux = lsPin.next() )
				if( aux->alignment() == pin->alignment() )
				{
					aux->setPosition( (1.0 / ((float)(count-1)) ) * ((float)i) );
					placePin( aux );
					i++;
				}
		}

	}else
		placePin( pin );

	pin->show();
	
	return pin;
}

	

void LEDevice::removePin( LEPin * pin )
{
	if( lsPin.remove( pin ) )
		pin->remove();
}

void LEDevice::placePin(LEPin * pin )
{
	int offsetX, offsetY, desp;

	float position = pin->position();
	
	// Establece el espacio no usado en los extremos del LEDevice
	int hDifference = PIN_MARGIN_H;
	int vDifference = PIN_MARGIN_V;

	// Ajustamos la posición de los pins a la geometría de la figura en función de su alineamLEnto
	switch( pin->alignment() )
	{
	case LEPin::Left:
		offsetY = y()+vDifference + (pin->position()*((float)(height()-2*vDifference-pin->height())));
		if( getLeftProjection( &desp, offsetY ) )
			offsetX = desp - pin->width();
		else
			offsetX = x() - pin->width();
		break;
	case LEPin::Top:
		offsetX = x()+hDifference+(pin->position()*((float)(width()-2*hDifference-pin->width())));
		if( getTopProjection( offsetX, &desp ) )
			offsetY = desp - pin->height();
		else
			offsetY = y() - pin->height();
		break;
	case LEPin::Right:
		offsetY =  y()+vDifference+(pin->position()*((float)(height()-2*vDifference-pin->height())));
		if( getRightProjection( &desp, offsetY ) )
			offsetX = desp;
		else
			offsetX = x() + width();
		break;
	case LEPin::Bottom:
		offsetX = x()+hDifference+(pin->position()*((float)(width()-2*hDifference-pin->width())));
		if( getBottomProjection( offsetX, &desp ) )
			offsetY = desp;
		else
			offsetY = y() + height();
	}

	pin->move( offsetX, offsetY );

}

////////////////////////////////////////////////////////////////
//	Etiqueta
////////////////////////////////////////////////////////////////

void LEDevice::setLabel( const QString &text )
{
	if( label == NULL )
		label = new LELabel( this->canvas(), this );

	label->setText( text );
	label->move( this->x() +width()  - label->width(), this->y() + height() );
	label->show();
}

void LEDevice::showLabel()
{
	if( label != NULL )
		label->show();
}

void LEDevice::hideLabel()
{
	if( label != NULL )
		label->hide();
}

//////////////////////////////////////////////////////////////
//	Dibujado
//////////////////////////////////////////////////////////////

void LEDevice::drawShape( QPainter& p )
{
	p.setBrush( QColor(235, 240, 255) );
	p.drawPolygon( scaledShape );
}
