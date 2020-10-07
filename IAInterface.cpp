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



#include "IAInterface.h"

IAInterface::IAInterface()
{
	// Valores por defecto
	ba = 0;
	bBits = 32;
}

IAInterface::IAInterface( const IAInterface & iface )
{
	this->ba = iface.ba;
	this->nm = iface.nm;
	this->ports = iface.ports;
	this->bBits = iface.bBits;
}

void IAInterface::setName( const QString& name )
{
	this->nm = name;
}

QString IAInterface::name() const
{
	return this->nm;
}


void IAInterface::setBaseAddress( Q_UINT32 ba )
{
	this->ba = ba;
}

Q_UINT32 IAInterface::baseAddress() const
{
	return this->ba;
}

Q_UINT32 IAInterface::busBits() const
{
	return this->bBits;
}

void IAInterface::setBusBits(Q_UINT32 b)
{
	this->bBits = b;
}

Q_UINT32 IAInterface::size() const
{
	Q_UINT32 max=0;
	for( IAPortList::const_iterator it = ports.begin(); it != ports.end(); it++ )
		if( (*it).isInitialized() )
			if( (*it).baseAddress() + (*it).size() >= max )
				max = (*it).baseAddress() + (*it).size();

	// Techo del tamaño en palabras (4bytes)
	if( max%4 )
		max=max+(4-max%4);

	return max;
}

Q_UINT32 IAInterface::dataSize() const
{
	Q_UINT32 sz=0;
	for( IAPortList::const_iterator it = ports.begin(); it != ports.end(); it++ )
		if( (*it).isInitialized() ){
			if( (*it).writable() )
				sz += (*it).size();
			if( (*it).readable() )
				sz += (*it).size();
		}

	return sz;
}


//////////////////////////////////////////////////////////////////////
// Manipulación de puertos
//////////////////////////////////////////////////////////////////////
void IAInterface::insertPort( const QString& portName )
{
	IAPort p;
	p.setName( portName );
	p.setBaseAddress( size() );	// Al final del periférico
	p.setSize( 1 );				// Con 1byte de tamaño
	
	ports.append( p );
}

void IAInterface::removePort( const QString& portName )
{
	IAPort & p = findPort( portName );

	if( !p.isNull() )
		ports.remove( p );
}

void IAInterface::setPortAddress( const QString& portName, Q_UINT32 addr )
{
	Q_UINT32 sz;
	IAPort & p = findPort( portName );

	if( !p.isNull() ){
		if( !p.isInitialized() )
			sz = 1;
		else
			sz = p.size();

		if( checkFreeRange( addr, sz, &p) )
			p.setBaseAddress( addr );
	}
}

void IAInterface::setPortSize( const QString& portName, Q_UINT32 sz )
{
	Q_UINT32 addr;
	IAPort & p = findPort( portName );

	if( !p.isNull() ){
		if( !p.isInitialized() )
			addr = size(); // Final del periférico hasta el momento
		else
			addr = p.baseAddress();

		if( checkFreeRange( addr, sz, &p ) )
			p.setSize( sz );
	}
}


//////////////////////////////////////////////////////////////////////
// Búsqueda de puerto por nombre
//////////////////////////////////////////////////////////////////////
IAPort & IAInterface::findPort( const QString& portName )
{
	for( IAPortList::iterator it = ports.begin(); it != ports.end(); it++ )
		if( (*it).name() == portName )
			return *it;

	return nullPort;
}

//////////////////////////////////////////////////////////////////////
// private members
//////////////////////////////////////////////////////////////////////

// Comprueba si el rango con inicio en 'addr' y tamaño 'size' bytes está libre
bool IAInterface::checkFreeRange( Q_UINT32 addr, Q_UINT32 size, IAPort * butThisPort )
{
	for( IAPortList::iterator it = ports.begin(); it != ports.end(); it++ )
		if( (*it).isInitialized() && (*it)!=(*butThisPort))
			if( (addr >= (*it).baseAddress() && addr < (*it).baseAddress()+(*it).size()) ||
				(addr+size > (*it).baseAddress() && addr+size <= (*it).baseAddress()+(*it).size()) )
				return false;
		
	return true;
}

