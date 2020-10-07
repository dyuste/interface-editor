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



#include "HDLVComAssembler.h"
#include "HDLProcess.h"

#include <qprocess.h>
#include <qregexp.h>

//////////////////////////////////////////////////////////////////////
// Ensablaje de librerías
//////////////////////////////////////////////////////////////////////

bool HDLVComAssembler::createSourceLibrary( const QString& source, bool mustBuildFiles )
{
	// Creación de la librería
	HDLProcLibBuilder * vlib = new HDLProcLibBuilder( source, this, "vlib");
	vlib->setMustBuildFiles( mustBuildFiles );

	// Manejadores de mensajes
	connect( vlib, SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( vlib, SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );

	// Esta versión no chequea los errores producidos por vlib (ya que pueden
	// significar que la librería ya existe, en cuyo caso no es un error)
	connect( vlib, SIGNAL(success()), this, SLOT(vlibDone()) );
	connect( vlib, SIGNAL(failure()), this, SLOT(vlibDone()) );

	return vlib->start();
}

bool HDLVComAssembler::createSourceLibrary( const QString& source, const QStringList& specificTargets )
{
	// Creación de la librería
	HDLProcLibBuilder * vlib = new HDLProcLibBuilder( source, this, "vlib");
	vlib->setMustBuildFiles( true );
	vlib->setSpecificTargets( specificTargets );

	// Manejadores de mensajes
	connect( vlib, SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( vlib, SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );

	// Esta versión no chequea los errores producidos por vlib (ya que pueden
	// significar que la librería ya existe, en cuyo caso no es un error)
	connect( vlib, SIGNAL(success()), this, SLOT(vlibDone()) );
	connect( vlib, SIGNAL(failure()), this, SLOT(vlibDone()) );

	return vlib->start();
}

bool HDLVComAssembler::buildSourceLibrary( const QString& source )
{	
	// Argumentos del proceso:
	HDLProcCompiler * vcom = new HDLProcCompiler( this, "vcom");
	vcom->setWorkLibrary( source );

	// Manejadores de mensajes
	connect( vcom, SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( vcom, SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );

	connect( vcom, SIGNAL(success()), this, SLOT(vcomSuccess()) );
	connect( vcom, SIGNAL(failure()), this, SLOT(vcomFailure()) );

	return vcom->start();
}

// Devuelve true si almenos una librería se compila con éxito
bool HDLVComAssembler::buildSourceLibrary( const QStringList& sources )
{
	bool someRight=false, allRight=true;
	for( QStringList::const_iterator it = sources.begin(); it != sources.end(); ++it )
		if( buildSourceLibrary( *it ) )
			someRight=true;
		else
			allRight=false;
	
	if( !allRight && someRight)
		qWarning( tr("No se pudieron compilar todas las librerías.") );
	else if( !someRight )
		qWarning( tr("No se pudo compilar ninguna librería.") );

	return someRight;
}

//////////////////////////////////////////////////////////////////////
// Ensamblaje del proyecto
//////////////////////////////////////////////////////////////////////

bool HDLVComAssembler::buildProject( const QStringList& entities, const QString& projectLib )
{
	HDLProcCompiler * vcom = new HDLProcCompiler( entities, projectLib, this, "vcom");
	
	// Manejadores de mensajes
	connect( vcom, SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( vcom, SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );

	connect( vcom, SIGNAL(success()), this, SLOT(vcomSuccess()) );
	connect( vcom, SIGNAL(failure()), this, SLOT(vcomFailure()) );
	
	return vcom->start();
}

bool HDLVComAssembler::buildProject( const QString& entity, const QString& projectLib )
{
	return buildProject( QStringList( entity ), projectLib );
}

//////////////////////////////////////////////////////////////////////
// Simulación
//////////////////////////////////////////////////////////////////////
// Simula el modelo 'model', variando las señales 'inputs' usando las librerías 'dependences' [seleccionando las salidas 'outputs']
bool HDLVComAssembler::forceSignals( const QString& model, const SignalValues & inputs , const QStringList& dependences, const SignalValues & outputs )
{
	HDLProcSimulator * vsim = new HDLProcSimulator( model, dependences, inputs, outputs, this, "vsim" );
	
	// Manejadores de mensajes
	connect( vsim, SIGNAL(errorMessage(const QString&)), this, SIGNAL(errorMessage(const QString&)) );
	connect( vsim, SIGNAL(outputMessage(const QString&)), this, SIGNAL(outputMessage(const QString&)) );

	connect( vsim, SIGNAL(success()), this, SLOT(vsimSuccess()) );
	connect( vsim, SIGNAL(failure()), this, SLOT(vsimFailure()) );

	return vsim->start();
}

bool HDLVComAssembler::forceSignals( const QString& model, const SignalValues & inputs , const QStringList& dependences )
{
	SignalValues emptyList;

	return forceSignals( model, inputs, dependences, emptyList );
}


//////////////////////////////////////////////////////////////////////
// Manejadores internos
//////////////////////////////////////////////////////////////////////
// Fin de vLib, se enlaza con la compilación de la librería
void HDLVComAssembler::vlibDone()
{
	if( !sender() )
		return;
	
	if( !sender()->inherits( "HDLProcLibBuilder" ) )
		return;

	HDLProcLibBuilder * procLib = (HDLProcLibBuilder*)sender();
	
	if( procLib->arguments().count() == 0 )
		emit outputMessage( tr("> Construcción de la librería omitida (ya está actualizada)") );
	else
		emit outputMessage( tr("> %1 ha finalizado.").arg( procLib->arguments().join(" ") ) );

	if( procLib->mustBuildFiles() )
		if( procLib->hasSpecificTargets() )
			buildProject( procLib->specificTargets(), procLib->source() );
		else
			buildSourceLibrary( procLib->source() );

	delete procLib;
}

void HDLVComAssembler::vcomSuccess()
{
	if( !sender() )
		return;
	
	if( !sender()->inherits( "HDLProcCompiler" ) )
		return;

	HDLProcCompiler * procCom = (HDLProcCompiler*)sender();

	if( procCom->isLibrary() )
		emit librarySuccess( procCom->workLibrary() );
	else
		emit compilationSuccess( procCom->targetEntities(), procCom->workLibrary() );

	if( procCom->arguments().count() == 0 )
		emit outputMessage( tr("> Compilación de la librería omitida (todos los fichero están al día)") );
	else
		emit outputMessage( tr("> %1 ha finalizado con éxito.").arg( procCom->arguments().join(" ") ) );

	delete procCom;
}

void HDLVComAssembler::vcomFailure()
{
	if( !sender() )
		return;
	
	if( !sender()->inherits( "HDLProcCompiler" ) )
		return;

	HDLProcCompiler * procCom = (HDLProcCompiler*)sender();

	if( procCom->isLibrary() )
		emit libraryFailure( procCom->workLibrary() );
	else
		emit compilationFailure( procCom->targetEntities(), procCom->workLibrary() );

	emit errorMessage( tr("> %1 ha finalizado con errores.").arg( procCom->arguments().join(" ") ) );
	
	delete procCom;
}

void HDLVComAssembler::vsimSuccess()
{
	if( !sender() )
		return;
	
	if( !sender()->inherits( "HDLProcSimulator" ) )
		return;

	HDLProcSimulator * procSim = (HDLProcSimulator*)sender();

	emit simulationSuccess( procSim->model(), procSim->inputSignals(), procSim->outputSignals() );
	emit outputMessage( tr("> %1 ha finalizado con éxito.").arg( procSim->arguments().join(" ") ) );

	delete procSim;
}

void HDLVComAssembler::vsimFailure()
{
	if( !sender() )
		return;
	
	if( !sender()->inherits( "HDLProcSimulator" ) )
		return;

	HDLProcSimulator * procSim = (HDLProcSimulator*)sender();
	
	emit simulationFailure( procSim->model(), procSim->inputSignals() );
	emit errorMessage( tr("> %1 ha finalizado con errores.").arg( procSim->arguments().join(" ") ) );
	
	delete procSim;
}
