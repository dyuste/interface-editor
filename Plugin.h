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



#if !defined(_PLUGIN_H_)
#define _PLUGIN_H_

#include <qvariant.h>

class Plugin;

/* Indicadores de Estado */
enum PluginFlags{
	InvalidProperty=0xFFFFFFFF
};

/* Tipos de Plugin */
enum PluginType{
	ptProject	// Tipo Proyecto
};

/* Clase Base para Plugins */
class Plugin
{
public:
	Plugin();

	// Nombre del Plugin
	virtual const char * name() const =0;

	// Breve descripción del Plugin
	virtual const char * description() const =0;

	// Acerca del autor del Plugin
	virtual const char * about() const =0;

	// Versión del Plugin ( 0 => 0.0, 1 => 0.1, 10 => 1.0, etc)
	virtual unsigned int version() const =0;

	// Determinación del tipo de Plugin, 
	// permite a la aplicación determinar la interfaz del Plugin
	virtual PluginType type() const =0;


/* Propiedades */
	// Número de propiedades
	virtual unsigned int properties() const =0;

	// Iterador de propiedades (devuelven NULL al alcanzar el último elemento)
	virtual const char * firstProperty();
	virtual const char * nextProperty();

	// Índice de la propiedad dado el nombre/nombre de la propiedad dado el índice
	virtual const unsigned int getPropertyIndex( const char * property ) const =0;
	virtual const char *getPropertyName( unsigned int property ) const =0;

	// Manipulación de la propiedad
	virtual const QVariant getProperty( unsigned int property ) const =0;
	virtual const QVariant getProperty( const char * property ) const
		{ return getProperty( getPropertyIndex(property) ); }

	virtual void setProperty( unsigned int property, const QVariant & value ) =0;
	virtual void setProperty( const char * property, const QVariant & value )
		{ setProperty( getPropertyIndex(property), value ); }

/* Propiedades tipo ENUM */

	// CONSIDERACIONES:
	// * Por defecto ninguna propiedad será de tipo ENUM, sólo es preciso sobreescribir
	//   estos métodos si se desea incorporar propiedades de este tipo.
	//
	// * El valor de la propiedad puede establecerse mediante el método setProperty()
	//   pasando como argumento 'value' el índice ordinal del valor (QVariant(unsigned int))
	//
	// * El valor de la propiedad puede consultarse mediante el método getProperty()
	//   siendo el valor devuelto el índice ordinal del valor

	// Determina si la propiedad 'property' es de tipo Enum
	virtual bool isEnumProperty( unsigned int property ) const
		{ return false; } 
	virtual bool isEnumProperty( const char * property ) const
		{ return isEnumProperty( getPropertyIndex(property) ); }

	// Devuelve la longitud del vector de cadenas que describe los valores que puede
	// tomar la propiedad 'property' (Por defecto 0)
	virtual const unsigned int getEnumValuesLength( unsigned int property ) const
		{ return 0; }
	virtual const unsigned int getEnumValuesLength( const char * property ) const
		{ return getEnumValuesLength( getPropertyIndex(property) ); }

	// Devuelve el vector de cadenas que describe los valores que puede
	// tomar la propiedad 'property' (Por defecto NULL)
	virtual const char** getEnumValues( unsigned int property ) const
		{ return 0; }
	virtual const char** getEnumValues( const char * property ) const
		{ return getEnumValues( getPropertyIndex(property) ); }

private:
	unsigned int it;
};

class ProjectPlugin : public Plugin
{
public:
/* Sobreescritura de métodos */
	// Determinación del tipo de Plugin, 
	// permite a la aplicación determinar la interfaz del Plugin
	PluginType type() const{ return ptProject; }

/* Métodos específicos */
	// ProjectFolder: 
	// Establece la ubicación del proyecto. 
	virtual void setProjectFolder( const char * folder ){ this->fld = folder; }
	virtual const char * projectFolder() const{ return this->fld; }

	// ProjectName:
	// Establece el nombre del projecto (nombre del fichero de proyecto sin extensión)
	virtual void setProjectName( const char * name ){ this->nm = name; }
	virtual const char * projectName() const{ return this->nm; }

	// ProjectAutor:
	// Establece el autor del projecto
	virtual void setProjectAutor( const char * autor ){ this->at = autor; }
	virtual const char * projectAutor() const{ return this->at; }

	// CreateProject (a implementar en clases derivadas)
	// Crea el proyecto en base a la configuración del plugin
	virtual bool createProject() =0;

/* Diálogo de Configuración */

	// *Si el plugin dispone de un diálogo propio de configuración devolverá true
	//  en hasExecDialog()
	//
	// *En caso de existir una implementación de execDialog(), se espera por su parte
	//  el funcionamiento como diálogo modal, y el método devolverá 'true' cuando 
	//  el usuario confirme la creación del proyecto y 'false' en caso contrario
	//
	// *La implementación por defecto indica la ausencia de diálogo
	virtual bool hasExecDialog() const
		{ return false; }
	virtual bool execDialog()
		{ return true; }

private:
	const char *fld, *nm, *at;
};

#endif 