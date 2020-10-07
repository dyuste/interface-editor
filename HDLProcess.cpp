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



#include "HDLProcess.h"

HDLProcess::HDLProcess( QObject * parent , const char * name )
: QProcess( parent, name )
{
	setCommunication( Stdout|Stderr );
}

void HDLProcess::afterExit()
{
	if( normalExit() )
	{
		if( !exitStatus() )
			emit success();
		else
			emit failure();
	}else
		emit failure();
	
}

bool HDLProcess::start( QStringList * env )
{
	connect( this, SIGNAL(processExited()), this, SLOT(afterExit()) );
	connect( this, SIGNAL(readyReadStderr()), this, SLOT(readFromStderr()) );
	connect( this, SIGNAL(readyReadStdout()), this, SLOT(readFromStdout()) );

	emit outputMessage( "$ " + arguments().join(" ") );
	return QProcess::start( env );
}

void HDLProcess::readFromStderr()
{
	while( canReadLineStderr() )
		emit errorMessage( readLineStderr() );
}

void HDLProcess::readFromStdout()
{
	while( canReadLineStdout() )
		emit outputMessage( readLineStdout() );
}
	
//////////////////////////////////////////////////////////////////////
// HDLProcLibBuilder
//	Invoca a vLib para construir las librerías de recursos
//////////////////////////////////////////////////////////////////////
HDLProcLibBuilder::HDLProcLibBuilder ( QObject * parent , const char * name )
: HDLProcess( parent, name )
{
}

HDLProcLibBuilder::HDLProcLibBuilder ( const QString & source, QObject * parent , const char * name )
: HDLProcess( parent, name )
{
	setSource( source );
}

void HDLProcLibBuilder::setSource( const QString& source )
{
	src = source;
}

QString HDLProcLibBuilder::source() const
{
	return src;
}

void HDLProcLibBuilder::setMustBuildFiles( bool mustBuild )
{
	this->mustBuild = mustBuild;
}

bool HDLProcLibBuilder::mustBuildFiles() const
{
	return mustBuild;
}

void HDLProcLibBuilder::setSpecificTargets( const QStringList & specificTargets )
{
	specTargets = specificTargets;
}

QStringList HDLProcLibBuilder::specificTargets() const
{
	return specTargets;
}

bool HDLProcLibBuilder::hasSpecificTargets() const
{
	return !specTargets.isEmpty();
}

bool HDLProcLibBuilder::start( QStringList * env )
{
	if( checkLibuptoDate() ){
		emit success();
		return true;
	}else{
		addArgument("vlib");
		addArgument("-archive");
		addArgument( source() );
	
		return HDLProcess::start( env );
	}
}

// Comprueba si el documento está al día
bool HDLProcLibBuilder::checkLibuptoDate()
{
	// La carpeta de la librería no existe
	QFileInfo libDir( source() );
	if( !libDir.exists() )
		return false;

	// El fichero de librería no existe
	QFileInfo libFile( source() + QDir::separator() + "_info2" );
	if( !libFile.exists() )
		return false;

	return true;
}

	
//////////////////////////////////////////////////////////////////////
// HDLProcCompiler
//	Invoca a vCom para compilar los modelos'entities' 
//  en el espacio de trabajo "workLib"
//////////////////////////////////////////////////////////////////////
HDLProcCompiler::HDLProcCompiler ( QObject * parent , const char * name )
: HDLProcess( parent, name )
{
}

HDLProcCompiler::HDLProcCompiler ( const QStringList & entities, const QString & workLib, QObject * parent , const char * name )
: HDLProcess( parent, name )
{
	setTargetEntities( entities );
	setWorkLibrary( workLib );
}

bool HDLProcCompiler::isLibrary() const
{
	return targetEntities().isEmpty();
}

void HDLProcCompiler::setTargetEntities( const QStringList& entities )
{
	this->entities = entities;
}

QStringList HDLProcCompiler::targetEntities() const
{
	return entities;
}

void HDLProcCompiler::setWorkLibrary( const QString& workLib )
{
	this->workLib = workLib;
}

QString HDLProcCompiler::workLibrary() const
{
	return workLib;
}

bool HDLProcCompiler::start( QStringList * env )
{
	QStringList targets, finalTargets;

	// Determinamos los ficheros a compilar:
	if( !isLibrary() ){
		// Estamos compilando ficheros concretos (probablemente de un proyecto)
		finalTargets = targetEntities();
	}else{
		// Estamos compilando todo un directorio (probablemente una librería)
		QDir libDir( workLibrary() );
		libDir.setFilter( QDir::Files );
		libDir.setNameFilter( "*.vhd*" );

		targets = libDir.entryList();
		
		for( QStringList::iterator it = targets.begin(); it != targets.end(); ++it )
			if( !checkComponentuptoDate( workLibrary()+QDir::separator()+*it ) )
				finalTargets.append( workLibrary()+QDir::separator()+*it );
	}

	// Si no hay objetivos (todos están al día) se emite éxito y se anula la ejecución
	if( finalTargets.isEmpty() ){
		emit success();
		return true;
	}

	// Establecemos los argumentos del compilador
	addArgument("vcom");
	addArgument("-work");
	addArgument( workLibrary() );
	for( QStringList::iterator it = finalTargets.begin(); it != finalTargets.end(); ++it )
		addArgument( *it );

	return HDLProcess::start( env );

}
	
// Comprueba si el documento está al día
bool HDLProcCompiler::checkComponentuptoDate(const QString & HDLFile )
{
	QString binary = HDLFile;
	if( HDLFile.length() > 4 )
		binary.truncate( HDLFile.length()-4 );

	// El fichero binario asociado a HDLFile todavía no existe
	QFileInfo binFile( binary );
	if( !binFile.exists() )
		return false;

	// El fichero binario no está al día respecto al fuente HDL
	QFileInfo srcFile( HDLFile );

	return (binFile.lastModified() >= srcFile.lastModified());
	
}

//////////////////////////////////////////////////////////////////////
// HDLProcSimulator
//	Invoca a vSim para simular el modelo 'model' enlazando con las
//  librerías libDependences, estableciendo las entradas 'inputs'
//  y recogiendo las salidas 'outputs'
//////////////////////////////////////////////////////////////////////
HDLProcSimulator::HDLProcSimulator ( QObject * parent , const char * name )
: HDLProcess( parent, name )
{
}

HDLProcSimulator::HDLProcSimulator ( const QString & model, const QStringList & libDependences, QObject * parent , const char * name )
: HDLProcess( parent, name )
{
	setModel( model );
	setLibraryDependences( libDependences );
}

HDLProcSimulator::HDLProcSimulator ( const QString & model, const QStringList & libDependences, const SignalValues & inputs, const SignalValues & outputs, QObject * parent , const char * name )
: HDLProcess( parent, name )
{
	setModel( model );
	setLibraryDependences( libDependences );
	setInputSignals( inputs );
	setOutputSignals( outputs );
}


void HDLProcSimulator::setModel( const QString& model )
{
	this->mod = model;
}

QString HDLProcSimulator::model() const
{
	return mod;
}

void HDLProcSimulator::setLibraryDependences( const QStringList& libDependences )
{
	this->libDependences = libDependences;
}

QStringList HDLProcSimulator::libraryDependences() const
{
	return libDependences;
}

void HDLProcSimulator::setInputSignals( const SignalValues & inputs )
{
	this->inputs = inputs;
}

SignalValues HDLProcSimulator::inputSignals() const
{
	return inputs;
}

void HDLProcSimulator::setOutputSignals( const SignalValues & outputs )
{
	this->outputs = outputs;
}

SignalValues HDLProcSimulator::outputSignals() const
{
	return outputs;
}

bool HDLProcSimulator::start( QStringList * env )
{
	// Construimos el origen de datos para el simulador
	buildSourceFiles();

	// Argumentos del programa
	addArgument("vsim");

	// Modo linea de comandos
	addArgument("-c");

	// Origen de comandos
	addArgument("-do");
	addArgument( inputFile );

	// Librerías de las que depende el modelo (incluida la del proyecto)
	for( QStringList::iterator itd = libraryDependences().begin(); itd != libraryDependences().end(); ++itd ){
		addArgument( "-lib" );
		addArgument( *itd );
	}

	// Modelo a simular
	addArgument( model() );
	
	return HDLProcess::start( env );
}

// Crea el fichero "orders#.in" que determinará el comportamiento del simulador
// indicando el fichero "signals#.out" donde se escribirán los resultados
bool HDLProcSimulator::buildSourceFiles()
{
	static int id=1;
	
	outputFile = QString( "signals%1.out" ).arg(id);
	inputFile = QString( "orders%1.in" ).arg(id);

	QFile orders( inputFile );
	if( !orders.open( IO_WriteOnly ) )
		return false;

	QTextStream ordersStream( &orders );

	// Filtro de datos de salida
	if( !outputs.isEmpty() )
		for( SignalValues::iterator ito = outputSignals().begin(); ito != outputSignals().end(); ++ito )
			ordersStream << "add list " << ito.key() << "\n";
		else
			ordersStream << "add list *\n";
	
	// Datos de entrada
	for( SignalValues::iterator iti = inputSignals().begin(); iti != inputSignals().end(); ++iti )
			ordersStream << "force " << iti.key() << " " << (char)iti.data() << "\n";
	
	ordersStream << "run\n" << "write list " << outputFile << "\n";

	orders.close();

	id++;

	return true;
}
	
