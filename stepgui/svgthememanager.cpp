#include "svgthememanager.hpp"
#include <algorithm>

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

namespace StepGui
{
	SvgThemeManager::SvgThemeManager( QObject* p_parent ) :
		QObject( p_parent ),
		m_itemsTable(),
		m_themesList()
	{
		//
	}
	
	
	bool SvgThemeManager::add( StepGui::SvgTheme& p_svgTheme )
	{
		//Check if the theme with the current address &p_svgTheme
		//has already been indexed.
		//ToDo : Find some better (and more secure) way to do this, e.g.
		//		 avoid dangling pointers after deletion / copy operations
		//       by using shared pointers to the SvgTheme instead !
		
		if( p_svgTheme.isEmpty() )
		{
			return false;
		}
		
		if( ( this->m_themesList.isEmpty() ) || ( 
			( std::find( this->m_themesList.begin(), this->m_themesList.end(), &p_svgTheme ) == this->m_themesList.end() ) ) )
		{
			//Either no theme is currently handled by the manager, or
			//the current address is not already fetched, hence it is save to proceed.
			
			this->m_themesList.append( &p_svgTheme );
			
			for( StepGui::SvgTheme::iterator it( p_svgTheme.begin() ) ; 
				 it != p_svgTheme.end() ; ++it )
			{
				this->m_itemsTable.insert( *it, &p_svgTheme );
			}
			
			return true;
		}
		
		return false;
	}
	
	StepGui::SvgTheme*	SvgThemeManager::theme( const QString& p_itemId ) const
	{
		if( this->m_itemsTable.contains( p_itemId ) )
		{
			return	this->m_itemsTable.value( p_itemId );
		}
		
		return static_cast< SvgTheme* >( 0 );
	}
	
	
	QList< StepGui::SvgTheme* >	SvgThemeManager::themes( const QString& p_itemId ) const
	{
		if( this->m_itemsTable.contains( p_itemId ) )
		{
			return this->m_itemsTable.values( p_itemId );
		}
		
		return QList< StepGui::SvgTheme* >();
	}
	
	SvgThemeManager::const_items_iterator	SvgThemeManager::itemsBegin() const
	{
		return this->m_itemsTable.begin();
	}
	
	
	SvgThemeManager::const_items_iterator	SvgThemeManager::itemsEnd() const
	{
		return this->m_itemsTable.end();
	}
	
	
	SvgThemeManager::const_items_iterator	
	SvgThemeManager::itemsBegin( const QString& p_itemId ) const
	{
		if( this->m_itemsTable.contains( p_itemId )
		{
			return this->m_itemsTable.lowerBound( p_itemId );
		}
		else
		{
			return this->m_itemsTable.end();
		}
	}
	
	SvgThemeManager::const_items_iterator	
	SvgThemeManager::itemsEnd( const QString& p_itemId ) const
	{
		if( this->m_itemsTable.contains( p_itemId )
		{
			return this->m_itemsTable.upperBound( p_itemId );
		}
		else
		{
			return this->m_itemsTable.end();
		}
	}
	
	SvgThemeManager::const_themes_iterator	SvgThemeManager::themesBegin() const
	{
		return this->m_themesList.begin();
	}
	
	SvgThemeManager::const_themes_iterator	SvgThemeManager::themesEnd() const
	{
		return this->m_themesList.end();
	}
		
	bool SvgThemeManager::isEmpty() const
	{
		return ( ( this->m_themesList.isEmpty() )  || ( this->m_itemsTable.isEmpty() ) );
	}
	
	void SvgThemeManager::clear()
	{
		this->m_itemsTable.clear();
		this->m_themesList.clear();
	}
}

//


