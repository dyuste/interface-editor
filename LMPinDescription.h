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


// LMPinDescription.h: interface for the LMPinDescription class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LMPINDESCRIPTION_H_)
#define _LMPINDESCRIPTION_H_

#include "LEPin.h"

class LMPinDescription  
{
public:
	LMPinDescription();
	LMPinDescription( const QString &name, LEPin::AccessMode am, LEPin::Alignment al, double position=-1.0 );
	
	// Acceso a los atributos
	double position() const;
	QString name() const;
	LEPin::Alignment alignment() const;
	LEPin::AccessMode accessMode() const;
	LEPin::Level activeLevel() const;
	void setPosition( double pos );
	void setName( const QString &nm );
	void setAlignment( LEPin::Alignment al );
	void setAccessMode( LEPin::AccessMode am );
	void setActiveLevel( LEPin::Level l );

private:
	QString nm;
	LEPin::AccessMode am;
	LEPin::Alignment al;
	LEPin::Level l;
	double pos;

};

#endif
