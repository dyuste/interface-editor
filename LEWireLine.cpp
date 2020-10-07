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



#include "LEWireLine.h"

#include <qpainter.h>
#include <math.h>
#include <stdlib.h>

#include "LEConnectionPoint.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LEWireLine::LEWireLine( QCanvas * canvas, LEItem * parentItem )
: LEItem( canvas, parentItem )
{
	lpLeftCnnct = NULL;
	lpRightCnnct = NULL;
}

LEWireLine::~LEWireLine()
{
	hide();
}

void LEWireLine::remove()
{
	if( lpLeftCnnct )
		lpLeftCnnct->removeConnection(this);
	
	if( lpRightCnnct )
		lpRightCnnct->removeConnection(this);

	LEItem::remove();
}

//////////////////////////////////////////////////////////////////////
// Identificador de la clase
//////////////////////////////////////////////////////////////////////

int LEWireLine::rtti() const
{
	return RTTI;
}

//////////////////////////////////////////////////////////////////////
// Conectividad
//////////////////////////////////////////////////////////////////////
void LEWireLine::connectLeft( LEConnectionPoint * lpCnnct )
{
	if( !lpCnnct ){
		if( lpLeftCnnct )
			lpLeftCnnct->removeConnection(this);
	}else{
		lpCnnct->setConnection( this );
		moveVertex( 0, lpCnnct->x(), lpCnnct->y() );
	}
	lpLeftCnnct = lpCnnct;
}

void LEWireLine::connectRight( LEConnectionPoint * lpCnnct )
{
	if( !lpCnnct ){
		if( lpRightCnnct )
			lpRightCnnct->removeConnection(this);
	}else{
		lpCnnct->setConnection( this );
		moveVertex( vertexCount()-1, lpCnnct->x(), lpCnnct->y() );
	}
	lpRightCnnct = lpCnnct;
}

bool LEWireLine::isLeftConnected() const
{
	return (bool)lpLeftCnnct;
}

bool LEWireLine::isRightConnected() const
{
	return (bool)lpRightCnnct;
}

LEConnectionPoint * LEWireLine::leftConnection()
{
	return lpLeftCnnct;
}

LEConnectionPoint * LEWireLine::rightConnection()
{
	return lpRightCnnct;
}

void LEWireLine::connectionMoved( LEConnectionPoint * lpCnnct )
{
	if( lpCnnct == lpLeftCnnct ){
		moveVertex( 0, lpCnnct->x(), lpCnnct->y() );
	}else if( lpCnnct == lpRightCnnct ){
		moveVertex( ((int)vertexCount()-1), lpCnnct->x(), lpCnnct->y() );
	}
}

void LEWireLine::connectionRemoved( LEConnectionPoint * lpCnnct )
{
	if( lpCnnct == lpLeftCnnct ){
		lpLeftCnnct = NULL ;
	}else if( lpCnnct == lpRightCnnct ){
		lpRightCnnct = NULL ;
	}
}

//////////////////////////////////////////////////////////////////////
// Geometría externa
//////////////////////////////////////////////////////////////////////

QPointArray LEWireLine::areaPoints() const
{
	return bufferPolygon( 2 );
}

QRect LEWireLine::boundingRect() const
{
	QRect rv = vertexList.boundingRect();
	return QRect( rv.x()-1, rv.y()-1, rv.width()+2, rv.height()+2 );
}

QPointArray LEWireLine::bufferPolygon( double d ) const
{
	// El método empleado es el de intersectar las n rectas asociadas a los n segmentos que forman
	// la linea 'd' unidades desplazadas recorriéndola en ambos sentidos (se trata de envolver una
	// linea). Los puntos de intersección constituyen los nuevos vértices del buffer
	QPointArray rv;
	if( vertexList.count() == 0 )
		return rv;
	else if( vertexList.count() == 1 ){
		int vx = vertexList[0].x();
		int vy = vertexList[0].y();
		int id = (int)d;
		rv.putPoints( 0, 5, vx-id, vy-id, vx+id,vy-id, vx+id,vy+id, vx-id,vy+id, vx-id,vy-id );
		return rv;
	}

	double Alst, Blst, Clst, A, B, C, R, S, X, Y;
	int i, last = vertexList.count()-1;
	
	R = vertexList[1].x() - vertexList[0].x();
	S = vertexList[1].y() - vertexList[0].y();
	Alst = S / sqrt( R*R + S*S );
	Blst = -R / sqrt( R*R + S*S );
	Clst = Alst*((double)vertexList[0].x())+Blst*((double)vertexList[0].y())+d;
	
	for( i=1; i<last; i++ )
	{
	if( vertexList[i] != vertexList[i+1] ){
		R = vertexList[i+1].x() - vertexList[i].x();
		S = vertexList[i+1].y() - vertexList[i].y();
		
		// Vector (A,B) unitario normal al director (R,S) del segmento
		A = S / sqrt( R*R + S*S );
		B = -R / sqrt( R*R + S*S );
		
		// Constante de desplazamiento
		C = A*((double)vertexList[i].x())+B*((double)vertexList[i].y())+d;

		// Evitamos división por 0 para lineas Verticales/Horizontales 
		// NOTA: Consideramos Imposible A==B==0, ya que eso supondría haber definido dos puntos consecutivos uno
		//		sobre el otro. Asumimos corrección en la definición.
		if( A == 0 ){
			if( Alst == 0 )
				X = vertexList[i].x();
			else
				X = (Clst*B - C*Blst) / (B*Alst);
			Y = C / B;
		}else if( B == 0){
			if( Blst == 0 )
				Y = vertexList[i].y();
			else
				Y = (Alst*C - A*Clst) / (-A*Blst);
			X = C / A;
		}else{
			Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
			X = (C - B*Y) / A;
		}

		rv.putPoints( rv.count(), 1, X, Y );
		Alst = A;
		Blst = B;
		Clst = C;
}			
	}


// Borde "derecho" de la linea
  // Punto superior
	// Recta tangente al segmento (n-1, n) : Vector Director paralelo al segmento
	A = R / sqrt( R*R + S*S );
	B = S / sqrt( R*R + S*S );
	C = A*((double)vertexList[last].x())+B*((double)vertexList[last].y())+d;

	// Evitamos división por 0 para lineas Verticales/Horizontales 
	if( A == 0 ){
		if( Alst == 0 )
			X = vertexList[i].x();
		else
			X = (Clst*B - C*Blst) / (B*Alst);
		Y = C / B;
	}else if( B == 0){
		if( Blst == 0 )
			Y = vertexList[i].y();
		else
			Y = (Alst*C - A*Clst) / (-A*Blst);
		X = C / A;
	}else{
		Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
		X = (C - B*Y) / A;
	}
	rv.putPoints( rv.count(), 1, X, Y );
	Alst = A;
	Blst = B;
	Clst = C;
	

// Recorrido en sentido contrario
	for( i = last; i>0; i-- )
	{
if( vertexList[i] != vertexList[i-1] ){
		R = vertexList[i-1].x() - vertexList[i].x();
		S = vertexList[i-1].y() - vertexList[i].y();
		
		// Vector (A,B) unitario normal al director (R,S) del segmento con sentido invertido (-S, R)
		A = S / sqrt( R*R + S*S );
		B = -R / sqrt( R*R + S*S );
			
		// Constante de desplazamiento
		C = A*((double)vertexList[i].x())+B*((double)vertexList[i].y())+d;

		// Evitamos división por 0 para lineas Verticales/Horizontales 
		// NOTA: Consideramos Imposible A==B==0, ya que eso supondría haber definido dos puntos consecutivos uno
		//		sobre el otro. Asumimos corrección en la definición.
		if( A == 0 ){
			if( Alst == 0 )
				X = vertexList[i].x();
			else
				X = (Clst*B - C*Blst) / (B*Alst);
			Y = C / B;
		}else if( B == 0){
			if( Blst == 0 )
				Y = vertexList[i].y();
			else
				Y = (Alst*C - A*Clst) / (-A*Blst);
			X = C / A;
		}else{
			Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
			X = (C - B*Y) / A;
		}

		rv.putPoints( rv.count(), 1, X, Y );
		Alst = A;
		Blst = B;
		Clst = C;
}
	}

// Penúltimo punto (borde "izquierdo inferior")
	// Recta tangente al segmento (0-1) : Vector Director paralelo al segmento
	// NOTA: (R-S) ha sido calculado como director de (1-0), así que negamos sus componentes para obtener (0-1)
	A = R / sqrt( R*R + S*S );
	B = S / sqrt( R*R + S*S );
	C = A*((double)vertexList[0].x())+B*((double)vertexList[0].y())+d;

	// Evitamos división por 0 para lineas Verticales/Horizontales 
	if( A == 0 ){
		if( Alst == 0 )
			X = vertexList[i].x();
		else
			X = (Clst*B - C*Blst) / (B*Alst);
		Y = C / B;
	}else if( B == 0){
		if( Blst == 0 )
			Y = vertexList[i].y();
		else
			Y = (Alst*C - A*Clst) / (-A*Blst);
		X = C / A;
	}else{
		Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
		X = (C - B*Y) / A;
	}

	rv.putPoints( rv.count(), 1, X, Y );
	Alst = A;
	Blst = B;
	Clst = C;

// Primer Punto (borde "izquierdo superior")
	// Vector (A,B) unitario normal al director (-R,-S) del segmento -NOTA:(R,S) está calculado para (1,0) y no (0,1)
	A = -S / sqrt( R*R + S*S );
	B = R / sqrt( R*R + S*S );
	
	// Constante de desplazamiento
	C = A*((double)vertexList[1].x())+B*((double)vertexList[1].y())+d;

	// Evitamos división por 0 para lineas Verticales/Horizontales 
	// NOTA: Consideramos Imposible A==B==0, ya que eso supondría haber definido dos puntos consecutivos uno
	//		sobre el otro. Asumimos corrección en la definición.
	if( A == 0 ){
		if( Alst == 0 )
			X = vertexList[i].x();
		else
			X = (Clst*B - C*Blst) / (B*Alst);
		Y = C / B;
	}else if( B == 0){
		if( Blst == 0 )
			Y = vertexList[i].y();
		else
			Y = (Alst*C - A*Clst) / (-A*Blst);
		X = C / A;
	}else{
		Y = (Alst*C - A*Clst) / (Alst*B - A*Blst);
		X = (C - B*Y) / A;
	}
	rv.putPoints( rv.count(), 1, X, Y );

	// Replicamos el primer punto como último punto
	rv.putPoints( rv.count(), 1, rv[0].x(), rv[0].y() );

	return rv;
}

//////////////////////////////////////////////////////////////////////
// Búsqueda por aproximación
//////////////////////////////////////////////////////////////////////

// Devuelve el índice del vértice más próximo al punto p o -1 si no existe ninguna aproximación viable
int LEWireLine::nearestVertex( const QPoint &p ) const
{
	QPoint delta;
	int minV=-1, minDist=999999999, tempDist;

	if( vertexList[((int)(vertexList.count()-1))] == p )
		return ((int)(vertexList.count()-1));

	for( int i=0; i<vertexList.count()-1; i++ ){
		if( vertexList[i] == p )
			return i;

		delta = vertexList[i+1] - vertexList[i];
		if( delta.x() == 0 ){
		// Tratamiento Vertical
			if( ( vertexList[i].y() < p.y() && vertexList[i+1].y() > p.y() ) ||
				( vertexList[i].y() > p.y() && vertexList[i+1].y() < p.y() )  ){

				// Candidato Viable (minimización):
				tempDist = abs( p.x() - vertexList[i].x() );
				if( tempDist < minDist ){
					minDist = tempDist;
					if( abs( p.y() - vertexList[i].y() ) < abs( vertexList[i+1].y() - p.y() ) )
						minV = i;
					else
						minV = i+1;
				}
			}

		}else if( delta.y() == 0 ){
		// Tratamiento horizontal
			if( ( vertexList[i].x() < p.x() && vertexList[i+1].x() > p.x() ) ||
				( vertexList[i].x() > p.x() && vertexList[i+1].x() < p.x() )  ){

				// Candidato Viable (minimización):
				tempDist = abs( p.y() - vertexList[i].y() );
				if( tempDist < minDist ){
					minDist = tempDist;
					if( abs( p.x() - vertexList[i].x() ) < abs( vertexList[i+1].x() - p.x() ) )
						minV = i;
					else
						minV = i+1;
				}
			}

		}
	}

	return minV;
}

//////////////////////////////////////////////////////////////////////
// Manipulación de vértices
//////////////////////////////////////////////////////////////////////

QPoint LEWireLine::vertex( int i ) const
{
	if( i<0 || i>=vertexList.count() )
		return QPoint( -1, -1 );

	return vertexList[i];
}

int LEWireLine::vertexCount() const
{
	return vertexList.count();
}



// Edición de vértices
void LEWireLine::moveVertex( int i, int x, int y )
{
	moveVertex( i, QPoint( x, y ) );
}

void LEWireLine::moveVertex( int i, QPoint p )
{
	int index, nVertex;
	QPoint v, delta;
	QPointArray oldVertexs = vertexList.copy();

	// Validación previa
	if( i < 0 && i >= vertexList.count() )
		return;

	invalidate();	
/*	
	if( i == vertexList.count()-1 && i > 0){
		v = p - vertexList[i-1];
		if( abs(v.x()) < abs(v.y()) )
			vertexList[i] = QPoint( vertexList[i-1].x(), p.y() );
		else
			vertexList[i] = QPoint( p.x(), vertexList[i-1].y() );
		update();
		return;
	}
*/
	// Asignación efectiva
	vertexList[i] = p;
	
	// Reajuste de la ortogonalidad
	if( i < vertexList.count()-1 ){
		index = i;
		v = oldVertexs[index+1] - oldVertexs[index];
		delta = vertexList[index+1] - vertexList[index];

		nVertex = vertexList.count()-2;
		// Reajuste en cascada ascendente
		while( delta.x() != 0 && delta.y() != 0 && index <= nVertex ){
			if( v.x() == 0 ){				
				//  i Horizontalmente paralela respecto a i-1: Actualizar X en i-1 para mantener la ortogonalidad			
				vertexList[index+1] = QPoint( vertexList[index].x(), vertexList[index+1].y() );
			}else if( v.y() == 0 ){
				//  i Verticalmente paralela respecto a index+1: Actualizar Y en index+1 para mantener la ortogonalidad
				vertexList[index+1] = QPoint( vertexList[index+1].x(), vertexList[index].y() );
			}else{
				// No existe ortogonalidad previa : Inicializar ( Se fija la ortogonalidad en función de la "intención de P" )
				v = vertexList[index+1] - vertexList[index];
				if( abs(v.x()) < abs(v.y()) )
					vertexList[index+1] = QPoint( vertexList[index+1].x(), vertexList[index].y() );
				else
					vertexList[index+1] = QPoint( vertexList[index].x(), vertexList[index+1].y() );
			}

			if( index == nVertex ){
				insertVertex( oldVertexs[index+1], index+2 );
				break;
			}

			index++;
			v = oldVertexs[index+1] - vertexList[index];
			delta = vertexList[index+1] - vertexList[index];

		}
	}

	if( i >= 1 ){
		index = i;
		v = oldVertexs[index] - oldVertexs[index-1];
		delta = vertexList[index] - vertexList[index-1];
		// Reajuste en cascada descendente
		while( delta.x() != 0 && delta.y() != 0 && index >= 1 ){
			if( v.x() == 0 ){
				//  i Horizontalmente paralela respecto a index-1: Actualizar X en index-1 para mantener la ortogonalidad			
				vertexList[index-1] = QPoint( vertexList[index].x(), vertexList[index-1].y() );

			}else if( v.y() == 0 ){
				//  i Verticalmente paralela respecto a index-1: Actualizar Y en index-1 para mantener la ortogonalidad
				vertexList[index-1] = QPoint( vertexList[index-1].x(), vertexList[index].y() );
	
			}else{
				// No existe ortogonalidad previa : Inicializar ( Se fija la ortogonalidad en función de la "intención de P" )
				v = vertexList[index] - vertexList[index-1];
				if( abs(v.x()) < abs(v.y()) )
					vertexList[index-1] = QPoint( vertexList[index-1].x(), vertexList[index].y() );
				else
					vertexList[index-1] = QPoint( vertexList[index].x(), vertexList[index-1].y() );

			}
			
			if( index == 1 ){
				insertVertex( oldVertexs[0], 0 );
				break;
			}
			index--;
			v = oldVertexs[index] - vertexList[index-1];
			delta = vertexList[index] - vertexList[index-1];
		}
		
	}

	normalize();

	update();
}

void LEWireLine::setVertexs( QPointArray & vertexs )
{
	invalidate();
	
	vertexList = vertexs;
	normalize();

	update();
}

void LEWireLine::insertVertex( int x, int y, int i )
{
	invalidate();
	int indx;
	if( i<0 || i>=vertexList.count() ){
		indx = vertexList.count();
		vertexList.putPoints( indx, 1, x, y );
	}else
	{
		QPointArray nv((int)(vertexList.count()+1));
		
		int k;
		for(k=0; k<i; k++)
			nv[k] = vertexList[k];
		
		nv[i]=QPoint(x,y);
		
		for(k=i; k<vertexList.count(); k++)
			nv[k+1] = vertexList[k];
		
		vertexList = nv;
		indx = i;
	}

	update();
}

void LEWireLine::insertVertex( const QPoint &p, int i )
{
	insertVertex( p.x(), p.y(), i );
}

void LEWireLine::removeVertex( int i )
{
	invalidate();
	if( i >= 0 && i < vertexList.count() )
	{
		QPointArray nv(vertexList.count()-1);
		
		int k;
		for(k=0; k<i; k++)
			nv[k] = vertexList[k];
				
		for(k=i+1; k<vertexList.count(); k++)
			nv[k-1] = vertexList[k];
		
		vertexList = nv;
	}
	
	update();
}

void LEWireLine::normalize()
{
	QPointArray vl;
	int i, k=0;

	vl.putPoints( k, 1, vertexList[0].x(), vertexList[0].y() );
	for( i=1; i < ((int)(vertexList.count()-1)); i++ )
		if( ( vertexList[i-1].x() != vertexList[i].x() || vertexList[i].x() != vertexList[i+1].x() ) &&
			( vertexList[i-1].y() != vertexList[i].y() || vertexList[i].y() != vertexList[i+1].y() ) )
			vl.putPoints( ++k, 1, vertexList[i].x(), vertexList[i].y() );
	
	vl.putPoints( ++k, 1, vertexList[ ((int)vertexList.count()-1) ].x(), vertexList[ ((int)vertexList.count()-1) ].y() );
	
	vertexList = vl;	
}

void LEWireLine::drawShape( QPainter & p )
{
	p.drawPolyline( vertexList );
	// DEBUG:
	//QPointArray n = bufferPolygon(2);
	//p.setPen( QColor(255,200,200) );
	//p.drawPolyline( n );

}
