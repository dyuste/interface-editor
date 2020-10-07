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


// InterfaceDocument.h: interface for the InterfaceDocument class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_INTERFACEDOCUMENT_H_)
#define _INTERFACEDOCUMENT_H_

#include "Document.h"
#include "IAInterface.h"

class LMComponent;
class LEDevice;
class LEConnectionPoint;

class IDInterfaceSelector;
class IDPortSelector;
class IDRegister;

class InterfaceDocument : public Document
{
public:
	InterfaceDocument( const IAInterface & iface, QWidget * parent=0, const char * name = 0 );

	void setInterfaceData( const IAInterface & iface );
	IAInterface & interfaceData();

	void createInterface();

	inline IDInterfaceSelector * interfaceSelector(){ return ifSel; }
	inline IDPortSelector * portSelector(){ return portSel; }
	inline IDRegister * registerAt( unsigned int port, unsigned int column ){ return (regs && regs[port])?regs[port][column]:0; }

protected:
	bool createControlSignals();
	
	bool createInterfaceSelector( int left, int top );
	bool createPortSelector( int left, int top );
	bool createRegisters( int left, int top );

private:
	IAInterface iface;
	
	// Buses
	LEDevice* busData[32];
	
	IDInterfaceSelector * ifSel;
	IDPortSelector * portSel;
	IDRegister *** regs;
};

#endif 