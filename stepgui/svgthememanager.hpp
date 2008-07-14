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

#ifndef _SVG_THEME_MANAGER_HEADER_FILE_
#define _SVG_THEME_MANAGER_HEADER_FILE_

#include <QMultiMap>
#include <QList>
#include <QSvgRenderer>
#include <QObject>

#include "svgtheme.hpp"

namespace StepGui
{
	/**	\brief	manages multiple svg dressings / themes for step items
	  */
	  
	class SvgThemeManager : public QObject
	{
		private:
		
		//QHash would be nicer from a performanc pov, but iterators to
		//items stored in the hash would be invalidated by insertion operations.
		//Hence, we use the QMap, which is slower at lookups, but keeps 
		//iterators valid (cf. Qt 4.4 documentation, http://doc.trolltech.com/4.4/qmap-iterator.html)
		
		//Since the whole point of this class is to allow the mix'n match
		//of different themes, we use the slightly more convenient QMultiMap
		//variant:
		
		typedef QMultiMap< QString, StepGui::SvgTheme* >	itemsContainer;
		typedef QList< StepGui::SvgTheme* >					themesContainer;
		
		public:
		
		//STL style typedefs :
		
		typedef itemsContainer::iterator					items_iterator;
		typedef itemsContainer::const_iterator				const_items_iterator;
		
		typedef themesContainer::iterator					themes_iterator;
		typedef themesContainer::const_iterator				const_themes_iterator;
		
		/**	\brief	Default Constructor*/
		SvgThemeManager( QObject* = 0 );
		
		/**	\brief	Adds the content of a new SvgTheme to the list of available items*/
		bool add( SvgTheme& );
		
		/**	\brief	Retrieves the latest theme associated with a certain item*/
		SvgTheme*			theme( const QString& ) const;
		
		/**	\brief	Retrieves a list of all themes associated with a certain item */
		QList< SvgTheme* >	themes( const QString& ) const;
		
		/**	\brief	STL style begin of all item / theme pairs	*/
		const_items_iterator	itemsBegin() const;
		
		/**	\brief	STL style end of all item / theme pairs	*/
		const_items_iterator	itemsEnd() 	 const;
		
		/**	\brief	return iterator to the first theme associated with the given item  
			\note	If no theme is associated with the given item, return an iterator to 
					the end of the map*/
		const_items_iterator	itemsBegin( const QString& ) const;
		
		/**	\brief	return iterator to the (last + 1) theme associated with the given item  
			\note	If no theme is associated with the given item, return an iterator to 
					the end of the map*/
		const_items_iterator	itemsEnd( const QString& )   const;
		
		/**	\brief	STL style iterator to the first element in the themes list */
		const_themes_iterator	themesBegin() const;
		
		/**	\brief	STL style iterator to the last element in the themes list */
		const_themes_iterator	themesEnd()	  const;
		
		/**	\brief	check whether the theme manager hosts any items / themes */
		bool	isEmpty() const;
		
		/**	\brief	purge all items and themes from the ThemeManager
		  *	\note	the SvgTheme instances themselves are not scraped!
		  */
		void	clear();
		
		private:
		
		itemsContainer	m_itemsTable;	///< Contains a list of all item -> theme pairs
		themesContainer	m_themesList;	///< Contains a list of all themes
	};
}

#endif

//end stepgui/svgthememanager.hpp


