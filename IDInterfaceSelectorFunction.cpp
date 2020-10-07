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



#include "IDInterfaceSelectorFunction.h"

#include "LogicEditor.h"
#include "LEWireLine.h"
#include "LEDevice.h"

#include "Application.h"
extern Application * app;


#ifndef max
	#define max(a,b) ((a)>(b))?(a):(b)
#endif

IDInterfaceSelectorFunction::IDInterfaceSelectorFunction( unsigned int pattern, unsigned int length )
	: IDInterfaceSelector( pattern, length )
{
	this->devInv = 0;
	this->devNand = 0;
}

IDInterfaceSelectorFunction::~IDInterfaceSelectorFunction()
{
	if( this->devInv )
		delete [] devInv;
	if( this->devNand )
		delete [] devNand;
	if( this->cpInput )
		delete [] cpInput;
}

LEConnectionPoint * IDInterfaceSelectorFunction::in( unsigned int i )
{
	return this->cpInput[i];
}

LEConnectionPoint * IDInterfaceSelectorFunction::out( unsigned int i )
{
	return this->devOr->pinList().at(0)->connectionPoint();
}

bool IDInterfaceSelectorFunction::create( LogicEditor * editor, int left, int top )
{
	int i, j, k, sz, selSize = length();
	int nandTop = top;
	unsigned int addr = pattern();
	LMComponent * component;

	// Actualización de la geometría
	this->l = left;
	this->t = top;

	// Adquisición de componenentes
	LMComponent * invComponent = app->libraryManager().findComponent( "Seleccion:INV" );
	if( !invComponent )
		return false;

	// Asignación de recursos
	cpInput = new LEConnectionPoint*[length()];
	devInv = new LEDevice*[length()];
	devNand = new LEDevice*[4];

	// Creación de las puertas para la función selección
	i=0, j=0;
	while( selSize > 0 && i < 4 ){
		component = NULL;
		if( selSize >=16 ){
			component = app->libraryManager().findComponent( "Seleccion:NAND16" );
			if( !component )
				return false;
			sz=16;
			selSize -= 16;
		}else if( selSize >= 8 ){
			component = app->libraryManager().findComponent( "Seleccion:NAND8" );
			if( !component )
				return false;
			sz=8;
			selSize -= 8;
		}else if( selSize >= 4 ){
			component = app->libraryManager().findComponent( "Seleccion:NAND4" );
			if( !component )
				return false;
			sz=4;
			selSize -= 4;
		}else if( selSize >= 2 ){
			component = app->libraryManager().findComponent( "Seleccion:NAND2" );
			if( !component )
				return false;
			sz=2;
			selSize -= 2;
		}else
			break;

		// Creación del componente de selección
		if( component ){

			// Instanciación
			devNand[i] = editor->createDevice( component, false );
			devNand[i]->setName( "SEL"+QString::number(i) );
			devNand[i]->setSize( devNand[i]->width(), 45*(sz+1)+2 );
			devNand[i]->move( left+100, nandTop );
			devNand[i]->show();

			nandTop += 20 + devNand[i]->height();
			
			// Selección e interconexión interna (inversores con pins de las puertas OR)
			// a nivel de bit
			for( k=0; k<sz; k++ ){
				LEWireLine * wl;
				
				if( !(addr & 0x80000000) ){
				
					// El aparece a nivel bajo, hay que invertirlo
					devInv[j] = editor->createDevice( invComponent, false );
					devInv[j]->setName( "INV_A"+QString::number(j) );
					devInv[j]->move( left, top+((10+devInv[j]->height())*k) );
					devInv[j]->show();

					// Conexión entre el inversor y la puerta OR
					wl = editor->createWireLine( devInv[j]->pinList().at(0)->connectionPoint(),
										 devNand[i]->pinList().at(k+1)->connectionPoint() );
					wl->show();

					// La entrada del inversor para este bit forma parte de la interfaz del componente 
					// (NOTA: En esta aplicación, en una lista de pins, primero se encuentran las salidas
					//		  y después las entradas)
					cpInput[j] = devInv[j]->pinList().at(1)->connectionPoint();

				}else{

					devInv[j] = 0;
					cpInput[j] = devNand[i]->pinList().at(k+1)->connectionPoint();

				}				
				addr <<= 1;
				j++;
			}
			i++;
		}
	}

	// Conexiones finales
	if( i>0 && i<=5 ){
		int orSize;
		if( selSize>0 )orSize=i+1;else orSize=i;
		component = app->libraryManager().findComponent( "Seleccion:OR"+QString::number(orSize) );
		devOr = editor->createDevice( component, false );
		devOr->setName( "SEL" );
		devOr->move( left+200, top/2 );
		devOr->show();

		// Conexión NAND-OR
		for( k=0; k < i; k++ ){
			LEConnectionPoint *cpNAND, *cpOR;
			cpOR = devOr->pinList().at(k+1)->connectionPoint();
			cpNAND = devNand[k]->pinList().at(0)->connectionPoint();
	
			LEWireLine * wl = editor->createWireLine( cpNAND, cpOR, 0.35+0.5*((float)k)/((float)i) );
			wl->show();
		}

		// Conexión del bit de selección excedente (si lo hay)
		if( selSize > 0 ){

			if( !(addr & 0x80000000) ){	
				// El aparece a nivel bajo, hay que invertirlo
				devInv[j] = editor->createDevice( invComponent, false );
				devInv[j]->setName( "INV_A"+QString::number(j) );
				devInv[j]->move( left, top+((10+devInv[j]->height())*k) );
				devInv[j]->show();


				LEWireLine * wl = editor->createWireLine( devInv[j]->pinList().at(0)->connectionPoint(),
												  devOr->pinList().at(k+1)->connectionPoint() );
				wl->show();

				// Guardamos la referencia a la entrada
				cpInput[j] = devInv[j]->pinList().at(1)->connectionPoint();
												  
			}else{
				devInv[j] = 0;
				cpInput[j] = devOr->pinList().at(k+1)->connectionPoint();
			}
		}
	}

  // Actualización de geometría

	// El alto lo determinan las puertas NAND o el último inversor de entrada
	this->h = max( nandTop, (devInv[j]?(top+((10+devInv[j]->height())*k)):0) ) - this->t;
	
	// El ancho lo determina la puerta Or final (si existe) o cualquier puerta NAND
	this->w = (devOr?(devOr->x()+devOr->width()):(devNand[0]?(devNand[0]->x()+devNand[0]->width()):0) ) - this->l;

	return true;
}