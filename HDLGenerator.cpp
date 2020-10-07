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



#include "HDLGenerator.h"

#include <qcanvas.h>

#include "LEDevice.h"
#include "LEWireLine.h"
#include "LEPin.h"
#include "LEConnectionPoint.h"
#include "LEItem.h"
#include "LMComponent.h"
#include "LogicEditor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

HDLGenerator::HDLGenerator()
{
	this->dtOrgn = NULL;
	this->itms = NULL;
	this->sigsAtDate = false;
	this->instsAtDate = false;
	this->depsAtDate = false;
}

HDLGenerator::HDLGenerator( QCanvas * dataOrigin )
{
	this->dtOrgn = dataOrigin;
	this->itms = NULL;
	this->sigsAtDate = false;
	this->instsAtDate = false;
	this->depsAtDate = false;
}

void HDLGenerator::setDataOrigin( QCanvas * dataOrigin )
{
	this->dtOrgn = dataOrigin;	
	this->itms = NULL;
	this->sigsAtDate = false;
	this->instsAtDate = false;
	this->depsAtDate = false;
}

// Marca la cache como Not At Date, de forma
// que la próxima vez que sea necesaria deberá
// recalcularse
void HDLGenerator::setDataChanged()
{
	if( this->itms ){
		delete this->itms;
		this->itms = NULL;
	}
	this->sigsAtDate = false;
	this->instsAtDate = false;
	this->depsAtDate = false;
}

// Crea la lista {instances} y {extInstances}
// En caso de error devuelve FALSE e imprime el mensaje de error
// oportuno en errorOutput si ha sido establecido
bool HDLGenerator::buildInstances()
{
	if( instsAtDate )
		return true;

	insts.clear();
	extInsts.clear();

	if( !dtOrgn ){
		emit errorMessage( tr("Imposible generar HDL: El origen de datos no ha sido establecido.") );
		return false;
	}

	itms = new QCanvasItemList( dtOrgn->allItems() );
		
	QCanvasItemList::iterator it;
	for( it = itms->begin(); it != itms->end(); it++ )
		if( (*it)->rtti() == LEDevice::RTTI ){
			LEDevice * dev = (LEDevice*)*it;

			// Sólo se insertan los elementos originales (no los duplicados)
			if( LogicEditor::extractDupNameIndex( dev->name() ) == -1 ){

				if( dev->componentReference()->isExternSolving() )
					extInsts.append( dev );
				else
					insts.append( dev );
			}
		}

	instsAtDate = true;
	emit outputMessage( tr("Generada lista de instancias.") );
	return true;
}

// Crea las listas {signals} y {extSigs}
// En caso de error devuelve FALSE e imprime el mensaje de error
// oportuno en errorOutput si ha sido establecido
bool HDLGenerator::buildSignals()
{
	// Si la cache esta actualizada no recalculamos
	if( sigsAtDate ){
		emit outputMessage( tr("Generando señales... (se utilizará la caché)") );
		return true;
	}

	extSigs.clear();
	sigs.clear();
	portExtSignals.clear();

	// Requerimos el cálculo de las instancias
	if( !buildInstances() )
		return false;
 
	emit outputMessage( tr(" Generando señales...") );

	LEDevice* it;

	// Instancias externas
	for( it = extInsts.first(); it; it = extInsts.next() )
	{   // Para cada instancia de componente externa
		QPtrList<LEPin>& lsPin = it->pinList();
		for( LEPin* pin = lsPin.first(); pin; pin = lsPin.next() )
		{   // Para cada pin de la instancia de componente
			
			// Toda conexión al pin se mapeará con el propio pin
			LEItem * mappedItem = pin;
			
			// Recorremos el grafo de cables conectados a este pin (directamente o através de bifurcaciones)
			QPtrList<LEConnectionPoint> activeNodes;
			QPtrList<LEConnectionPoint> oldNodes;
			
			// Inicialmente tomamos el punto de conexión del pin y sus conexiones directas
			if( pin->connectionPoint() ){
				// Punto de conexión del pin
				activeNodes.append( pin->connectionPoint() );

				// Conexiones directas del pin (conexiones simétricas puntoDeConexión-puntoDeConexión: Se dan en los LEDevice duplicados)
				for( LEItem * item = pin->connectionPoint()->connectionList().first(); item; item=pin->connectionPoint()->connectionList().next() )
					if( item->rtti() == LEConnectionPoint::RTTI )
						activeNodes.append( (LEConnectionPoint*)item );
			}

			while( !activeNodes.isEmpty() ){
				LEConnectionPoint * cp = activeNodes.take();

				// Iteramos cada cable conectado al nodo activo CP
				for( LEItem * item = cp->connectionList().first(); item; item=cp->connectionList().next() ){  
					if( item->rtti() == LEWireLine::RTTI )
					{	
						LEWireLine *wl = (LEWireLine*)item;
						
						// Mapeamos el cable con el nombre del pin (evitando repetir entradas)
						if( !extSigs.contains( wl->name() ) )
							extSigs.insert( wl->resolvName('_'), mappedItem );

						// Exploramos el grafo en busca de nuevos nodos activos 
						if( cp == wl->leftConnection() ){
							// Insertamos en la lista de nodos activos el CP derecho (el opuesto al actual)

							if( wl->rightConnection() )
								// Evitamos procesar múltiples veces el mismo CP
								if( !oldNodes.contains( wl->rightConnection() ) )
									// Evitamos duplicar ocurrencias en la lista activeNodes
									if( !activeNodes.contains( wl->rightConnection() ) )
										activeNodes.append( wl->rightConnection() );
						}else{
							// Insertamos en la lista de nodos activos el CP izquierdo (el opuesto al acutal)
							
							if( wl->leftConnection() )
								// Evitamos procesar múltiples veces el mismo CP
								if( !oldNodes.contains( wl->leftConnection() ) )
									// Evitamos duplicar ocurrencias en la lista activeNodes
									if( !activeNodes.contains( wl->leftConnection() ) )
										activeNodes.append( wl->leftConnection() );
						}
					}
				}/*para cada LEWireLine*/
				
				// Añadimos el nodo CP a oldNodes para evitar volver a procesarlo
				oldNodes.append( cp );

			}/*mientas hayan Nodos activos*/
				
		}/*para cada LEPin*/

	}/*para cada Instancia Externa*/


	// Instancias internas
	for( it = insts.first(); it; it = insts.next() )
	{   // Para cada instancia de componente interna
	
		QPtrList<LEPin>& lsPin = it->pinList();
		for( LEPin* pin = lsPin.first(); pin; pin = lsPin.next() )
		{   // Para cada pin de la instancia de componente
			
			LEItem * mappedItem = NULL;
			
			// Recorremos el grafo de cables conectados a este pin (directamente o através de bifurcaciones)
			QPtrList<LEConnectionPoint> activeNodes;
			QPtrList<LEConnectionPoint> oldNodes;
			
			// Inicialmente tomamos el punto de conexión del pin
			if( pin->connectionPoint() )
				activeNodes.append( pin->connectionPoint() );
			while( !activeNodes.isEmpty() ){
				LEConnectionPoint * cp = activeNodes.take();
				
				// Iteramos cada cable conectado al nodo activo CP
				for( LEItem * item = cp->connectionList().first(); item; item=cp->connectionList().next() ){  
					if( item->rtti() == LEWireLine::RTTI )
					{	
						LEWireLine *wl = (LEWireLine*)item;
						
						// Comprobaciones previas
						if( extSigs.contains( wl->resolvName('_') ) )
							// Este cable ya ha sido mapeado como externo, abortamos la exploración
							break;
						else
							// Primer idenficador del cable encontrado, será su Item-alias
							if( !mappedItem )
								mappedItem = wl;


						// Mapeamos el cable con el nombre del pin (evitando repetir entradas)
						if( !sigs.contains( wl->resolvName('_') ) )
							sigs.insert( wl->resolvName('_'), mappedItem );
							
						// Exploramos el grafo en busca de nuevos nodos activos 
						if( cp == wl->leftConnection() ){
							// Insertamos en la lista de nodos activos el CP derecho (el opuesto al actual)

							if( wl->rightConnection() )
								// Evitamos procesar múltiples veces el mismo CP
								if( !oldNodes.contains( wl->rightConnection() ) )
									// Evitamos duplicar ocurrencias en la lista activeNodes
									if( !activeNodes.contains( wl->rightConnection() ) )
										activeNodes.append( wl->rightConnection() );
						}else{
							// Insertamos en la lista de nodos activos el CP izquierdo (el opuesto al acutal)
							
							if( wl->leftConnection() )
								// Evitamos procesar múltiples veces el mismo CP
								if( !oldNodes.contains( wl->leftConnection() ) )
									// Evitamos duplicar ocurrencias en la lista activeNodes
									if( !activeNodes.contains( wl->leftConnection() ) )
										activeNodes.append( wl->leftConnection() );
						}
					}
				}/*para cada LEWireLine*/
				
				// Añadimos el nodo CP a oldNodes para evitar volver a procesarlo
				oldNodes.append( cp );

			}/*mientas hayan Nodos activos*/
				
		}/*para cada LEPin*/

	}/*para cada Instancia Interna*/

	emit outputMessage( tr("    Todas las señales resueltas") );

	// Construcción del Puerto
	for( SignalMapper::iterator itSigs = extSigs.begin(); 
		 itSigs != extSigs.end();
		 ++itSigs )
		if( !portExtSignals.contains( itSigs.data() ) )
			portExtSignals.append( itSigs.data() );

	sigsAtDate = true;
	
	emit outputMessage( tr("    Generada Interfaz del componente") );
	
	return true;
}

// Crea la lista {components} de dependencias
// Requiere que {instances} haya sido construido
//   En caso contrario invoca a buildSimbols
//
// En caso de error devuelve FALSE e imprime el mensaje de error
// oportuno en errorOutput si ha sido establecido
bool HDLGenerator::buildDependences()
{
	if( depsAtDate ){
		emit outputMessage( tr("Generando dependencias... (se utlizará la caché)") );
		return true;
	}

	comps.clear();

	if( !buildSignals() )
		return false;

	// Creamos la lista conjunto de componentes referenciados
	for( LEDevice * dev = insts.first(); dev ; dev = insts.next() )
		if( !comps.contains( dev->componentReference() ) )
			comps.append( dev->componentReference() );
	
	emit outputMessage( tr("Generando dependencias... Generadas") );

	depsAtDate = true;
	return true;
}

// Construye el código HDL asociado al diseño volcándolo
// en dataOut.
//
// Requiere que todas las listas hayan sido construidos, así
// que invocará a las funciones build*
bool HDLGenerator::buildHDL(  const QString & entity, QTextOStream * dataOut )
{
	if( !dataOut )
		return false;

	// Nos aseguramos de que todas las listas hayan sido construidas
	// buildInstances( ); // La llamada de estos métodos está implícita en
	// buildSignals( );	  // buildDependences() si la cachéno está al día
	buildDependences( );

	// Verificación de condiciones
	if( portExtSignals.count() <= 0 ){
		emit outputMessage( tr("Generando fichero HDL '%1.vhd'...").arg(entity) );
		emit errorMessage( tr("    No hay señales externas, no se puede crear la interfaz del componente.") );
		return false;
	}
	
	// ENTITY
	*dataOut << "ENTITY " << entity << " IS\n";
	SignalMapper::iterator it = extSigs.begin();
	QString sigName = it.data()->resolvName('_');
	
	LEItem* portItem = portExtSignals.first();
	if( portItem ){
		*dataOut << "\tPORT( " << portItem->resolvName('_');
		if( portItem->rtti() == LEPin::RTTI )
			switch( ((LEPin*)portItem)->accessMode() ){
			case LEPin::Input:
				*dataOut << " : OUT BIT";
				break;
			case LEPin::Output:
				*dataOut << " : IN BIT";
				break;
			case LEPin::InputOutput:
				*dataOut << " : INOUT BIT";
				break;
			}
		

		for( portItem=portExtSignals.next(); portItem ; portItem=portExtSignals.next() ){
				*dataOut << "; " << portItem->resolvName('_');
				if( portItem->rtti() == LEPin::RTTI )
					switch( ((LEPin*)portItem)->accessMode() ){
					case LEPin::Input:
						*dataOut << " : OUT BIT";
						break;
					case LEPin::Output:
						*dataOut << " : IN BIT";
						break;
					case LEPin::InputOutput:
						*dataOut << " : INOUT BIT";
						break;
					}
		}			

		
		*dataOut << ");\nEND " << entity << ";\n\n";
	}

	// ARCHITECTURE
	*dataOut << "ARCHITECTURE estructural OF " << entity << " IS\n";
	
	// Enumeración de componentes requeridos
	for( LMComponent * cmp = comps.first(); cmp; cmp=comps.next() ){

		// COMPONENT
		*dataOut << "\tCOMPONENT " << cmp->name() << "\n";
		
		// PORT (...
		PinList::iterator pinIt = cmp->pinList().begin();
		
		*dataOut << "\t\tPORT( " << (*pinIt).name() << " : ";

		if( (*pinIt).accessMode() == LEPin::Input )
			*dataOut << "IN";
		else if( (*pinIt).accessMode() == LEPin::Output )
			*dataOut << "OUT";
		else
			*dataOut << "INOUT";
		*dataOut << " BIT";

		for( ++pinIt; pinIt != cmp->pinList().end(); ++pinIt ){
			*dataOut << "; " << (*pinIt).name() << " : ";

			if( (*pinIt).accessMode() == LEPin::Input )
				*dataOut << "IN";
			else if( (*pinIt).accessMode() == LEPin::Output )
				*dataOut << "OUT";
			else
				*dataOut << "INOUT";
			*dataOut << " BIT";
		}
		*dataOut << ");\n\tEND COMPONENT;\n";
	}
	*dataOut << "\n";

	// SIGNAL
	if( sigs.count() > 0 ){
		it = sigs.begin();
		sigName = it.data()->resolvName('_');
		*dataOut << "\tSIGNAL " << sigName;
		for( ++it; it != sigs.end(); ++it )
			if( it.data()->resolvName('_') != sigName ){
				sigName = it.data()->resolvName('_');
				*dataOut << ", " << sigName;
			}
		*dataOut << ": BIT;\n\n";
	}

	// BEGIN
	*dataOut << "\tBEGIN\n";

	for( LEDevice * lpDev = insts.first(); lpDev; lpDev = insts.next() ){
		*dataOut << "\t\t" << lpDev->name() << " : " << lpDev->componentReference()->name() << " PORT MAP( " ;
		LEPin * lpPin = lpDev->pinList().first();
		LEItem * lpCnnct = NULL;

		// Búsqueda de un cable conectado al pin
		for( lpCnnct = lpPin->connectionPoint()->connectionList().first(); lpCnnct; lpCnnct = lpPin->connectionPoint()->connectionList().next() )
			if( lpCnnct->rtti() == LEWireLine::RTTI )
				break;
		
		// Obtenemos el nombre mapeado del cable (primero comprobamos si está conectado a una señal externa)
		if( lpCnnct ){
			SignalMapper::iterator sigIt = extSigs.find( lpCnnct->name() );
			if( sigIt == extSigs.end() ){
				sigIt = sigs.find( lpCnnct->name() );
				if( sigIt != sigs.end() )
					*dataOut << sigs[ lpCnnct->name() ]->resolvName('_');
			}else
				*dataOut << extSigs[ lpCnnct->name() ]->resolvName('_');
		}
			

		for( lpPin = lpDev->pinList().next(); lpPin; lpPin = lpDev->pinList().next() ){
			// Búsqueda de un cable conectado al pin
			for( lpCnnct = lpPin->connectionPoint()->connectionList().first(); lpCnnct; lpCnnct = lpPin->connectionPoint()->connectionList().next() )
				if( lpCnnct->rtti() == LEWireLine::RTTI )
					break;
			
			// Obtenemos el nombre mapeado del cable (primero comprobamos si está conectado a una señal externa)
			if( lpCnnct ){
				SignalMapper::iterator sigIt = extSigs.find( lpCnnct->name() );
				if( sigIt == extSigs.end() ){
					sigIt = sigs.find( lpCnnct->name() );
					if( sigIt != sigs.end() )
						*dataOut << ", " << sigs[ lpCnnct->name() ]->resolvName('_');
				}else
					*dataOut << ", " << extSigs[ lpCnnct->name() ]->resolvName('_');
			}
		}

		*dataOut << ");\n\n";
		
	}

	// END
	*dataOut << "END estructural;";
	
	emit outputMessage( tr(" Fichero %1.vhd Generado").arg(entity) );
	return true;
}

// Construye la lista de señales asociada al modelo en XML, volcándolo
// en dataOut.
//
// Requiere que la lista {extSignals} haya sido generada, así que
// se invocarán las funciones build pertinentes
bool HDLGenerator::buildSignalsFile( const QString & entity, QTextOStream * dataOut )
{
	if( !dataOut )
		return false;

	// Nos aseguramos de que las listas {extSignals} y el mapa portExtSignals
	// hayan sido construidos
	//
	// buildInstances( ); // Se obvia, si es preciso será invocado por buildSignals
	buildSignals( );

	emit outputMessage( tr(" Generando fichero de señales '%1.sig'...").arg(entity) );

	// Verificación de condiciones
	if( portExtSignals.count() <= 0 ){
		emit errorMessage( tr("  No hay señales externas, no se puede continuar.") );
		return false;
	}
	
	// Cabecera <signals>
	*dataOut << "<signals entity=\"" << entity << "\">\n\n";

	// Entradas <signal>
	SignalMapper::iterator it = extSigs.begin();
	QString sigName = it.data()->resolvName('_');
	
	LEItem* portItem = portExtSignals.first();
	if( portItem ){
		*dataOut << "\t<signal accessMode=\"";
		if( portItem->rtti() == LEPin::RTTI )
			switch( ((LEPin*)portItem)->accessMode() ){
			case LEPin::Input:
				*dataOut << "OUT";
				break;
			case LEPin::Output:
				*dataOut << "IN";
				break;
			case LEPin::InputOutput:
				*dataOut << "INOUT";
				break;
			}
		*dataOut << "\">" << portItem->resolvName('_') << "</signal>\n";

		for( portItem=portExtSignals.next(); portItem ; portItem=portExtSignals.next() ){
			*dataOut << "\t<signal accessMode=\"";
			if( portItem->rtti() == LEPin::RTTI )
				switch( ((LEPin*)portItem)->accessMode() ){
				case LEPin::Input:
					*dataOut << "OUT";
					break;
				case LEPin::Output:
					*dataOut << "IN";
					break;
				case LEPin::InputOutput:
					*dataOut << "INOUT";
					break;
				}
			*dataOut << "\">" << portItem->resolvName('_') << "</signal>\n";
		}			

	}	

	// Cierrre </signals>
	*dataOut << "\n</signals>\n\n";

	emit outputMessage( tr("Generando fichero HDL '%1.vhd'... Generado").arg(entity) );

	return true;

}

// Devuelve la lista de dependencias (componentes de librería instanciados)
// 
// Si mustBuild vale true, se invoca buildDependences (que decidirá si
// es necesario reconstruirlas o utilizar la caché), en otro caso
// se entregará la caché (incluso estando vacío o fuera de fecha)
QPtrList<LMComponent> HDLGenerator::componentDependences( bool mustBuild )
{
	if( mustBuild )
		buildDependences();

	return comps;
}

