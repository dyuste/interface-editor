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


// LMPinDescription.cpp: implementation of the LMPinDescription class.
//
//////////////////////////////////////////////////////////////////////

#include "LMPinDescription.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LMPinDescription::LMPinDescription()
{
	this->nm = QString::null;
	this->pos = -1;
}

LMPinDescription::LMPinDescription( const QString &name, LEPin::AccessMode am, LEPin::Alignment al, double position )
{
	this->nm = name;
	this->am = am;
	this->al = al;
	this->pos = position;
}

//////////////////////////////////////////////////////////////////////
// Acceso a los atributos
//////////////////////////////////////////////////////////////////////
double LMPinDescription::position() const
{
	return pos;
}

QString LMPinDescription::name() const
{
	return nm;
}


LEPin::Alignment LMPinDescription::alignment() const
{
	return al;
}


LEPin::AccessMode LMPinDescription::accessMode() const
{
	return am;
}

LEPin::Level LMPinDescription::activeLevel() const
{
	return l;
}

void LMPinDescription::setPosition( double pos )
{
	this->pos = pos;
}


void LMPinDescription::setName( const QString &nm )
{
	this->nm = nm;
}


void LMPinDescription::setAlignment( LEPin::Alignment al )
{
	this->al = al;
}


void LMPinDescription::setAccessMode( LEPin::AccessMode am )
{
	this->am = am;
}

void LMPinDescription::setActiveLevel( LEPin::Level l )
{
	this->l = l;
}


