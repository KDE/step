/* This file is part of Step.
   Copyright (C) 2008 Lisbeth Probst <probst.lisbeth@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _SVG_THEME_MANAGER_CLASS_HEADER_FILE_
#define _SVG_THEME_MANAGER_CLASS_HEADER_FILE_

#include <QVector>
#include <QSvgRenderer>
#include <QString>
#include <QObject>
#include <QList>
#include <QFile>

namespace StepGui
{
	/**	 	\name	SvgTheme
	  * 	\brief	contains a list of all items in a theme and a pointer to the renderer
	  *
	  *		\todo	add fields for meta information (author, licensing, description, etc.)
	  */
	  
	class SvgTheme
	{
		private:
		
		typedef QVector< QString >					svgThemeContainer;
		
		public:
		
		//STL Style Typedefs
		typedef svgThemeContainer::pointer			pointer;
		typedef svgThemeContainer::const_pointer	const_pointer;
		typedef	svgThemeContainer::iterator			iterator;
		typedef svgThemeContainer::const_iterator	const_iterator;
		typedef svgThemeContainer::reference		reference;
		typedef svgThemeContainer::const_reference	const_reference;
		typedef svgThemeContainer::difference_type	difference_type;
		typedef svgThemeContainer::size_type		size_type;
		typedef svgThemeContainer::value_type		value_type;
		
		//Qt Style Typedefs
		typedef svgThemeContainer::Iterator			Iterator;
		typedef svgThemeContainer::ConstIterator	ConstIterator;
		
		/**	\brief	Default Constructor */
		SvgTheme();
		
		/**	\brief	Destructor
		  * \note	Since SvgTheme is not a Sub Class of QObject, 
		  *			dynamically allocated parts are not handled automatically.
		  *
		  *	\todo	Handle svg namespaces properly (e.g. inkscapes extensions, etc).
		  */
		~SvgTheme();
		
		/**	\brief	STL style accessor for the begin of the themes TOC */
		iterator begin()	{	return this->m_itemsTable.begin(); }
		
		/**	\brief	STL style accessor for the end of the themes TOC */
		iterator end()		{	return this->m_itemsTable.end();   }
		
		/**	\brief	STL style accessor for the begin of the themes TOC in a const context */
		const_iterator	begin() const	{ return this->m_itemsTable.begin(); }
		
		/**	\brief	STL style accessor for the end of the themes TOC in a const context */
		const_iterator 	end() const		{ return this->m_itemsTable.end();	 }
		
		/**	\brief	Returns a pointer to the themes SVG Renderer */
		QSvgRenderer*	renderer() const	{ return this->m_renderer; }
		
		/**	\brief	Discard all items and free the SvgRenderer*/
		void	clear();
		
		/**	\brief	Check whether the theme contains no elements */
		bool	isEmpty() const	{ return ( ( !this->m_renderer ) || ( this->m_itemsTable.isEmpty() ) ); }
		
		/**	\brief	load a svg file for parsing */
		bool	load( const QString&, const QString& = QString( "items" ) );
		
		private:
		
		/**	\brief	search the contents of the svg file for theme items and store them in a list*/
		bool	_parseSvgFile( QFile&, 
				const QString& = QString( "items" ), const bool = false,
				QString* = 0, int* = 0, int* = 0 );
		
		svgThemeContainer	m_itemsTable;	///< Table for storing the id of all themeable items
		QSvgRenderer*		m_renderer;		///< pointer to associated Svg Renderer on the free-store
	};
}


#endif

//end stepgui/svgtheme.hpp


