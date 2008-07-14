#include "svgtheme.hpp"

#include <QDomDocument>
#include <QDomElement>

namespace StepGui
{
	SvgTheme::SvgTheme() :
		m_itemsTable(),
		m_renderer( 0 )
	{
		//
	}
	
	SvgTheme::~SvgTheme()
	{
		if( this->m_renderer )
		{
			delete this->m_renderer;
		}
	}
	
	bool SvgTheme::load( const QString& p_path2svgFile, const QString& p_itemGroupId )
	{
		QFile	svgThemeFile( p_path2svgFile );
		
		return this->_parseSvgFile( svgThemeFile, p_itemGroupId );
	}
	
	bool SvgTheme::_parseSvgFile( 
		QFile& p_themeFile, const QString& p_itemGroupId,
		const bool p_processNamespaces,
		QString* p_errorMsg, int* p_errorLine, int* p_errorCol )
	{
		//ToDo : Make the following "fit" for namespace'd svg files (e.g.
		//       provide branches for calling the xxNS variants
		//       of the DOM class member methods 
		
		if( !p_themeFile.open( QIODevice::ReadOnly ) )
		{
			return false;
		}
		
		QDomDocument	svgDoc;
		
		//Transfer the content of the given svg theme file to the DOM instance
		if( !svgDoc.setContent( &p_themeFile, p_processNamespaces, p_errorMsg, p_errorLine, p_errorCol ) )
		{
			return false;
		}
		
		//Check whether the item Group Id is an valid string
		if( p_itemGroupId.isEmpty() )
		{
			return false;
		}
		
		//using the elementById() Method, as of Qt 4.4 one gets the 
		//"elementById() is not implemented and always returns a null Node" 
		//error message. Hence, the short, oneliner from below is commented out
		//and for the time being replaced by an explicit search for the 
		//group item with the proper id :
		
		//QDomElement		itemsEnclosingElement( svgDoc.elementById( p_itemGroupId ) );
		
		QDomElement 			itemsEnclosingElement;
		static const QString	enclosingTagName( "g" );
		static const QString 	enclosingIdName( "id" );
		
		{
			QDomNodeList allGroupElements( svgDoc.elementsByTagName( enclosingTagName ) );
			
			const uint cnt( allGroupElements.length() );
			
			for( uint i = 0 ; i < cnt ; ++i )
			{
				QDomNode	node( allGroupElements.item( i ) );
				
				//That seems to be a bit redundant: If one node with the tag name
				//enclosingTagName is an element, it should hold true for all of
				//them -> probably change that !
				
				if( node.isElement() )
				{
					QString nodeId( node.toElement().attribute( enclosingIdName, QString() ) );
					
					if( nodeId == p_itemGroupId )
					{
						itemsEnclosingElement = node.toElement();
						break;
					}
				}
			}
		}
		
		if( itemsEnclosingElement.isNull() )
		{
			return false;
		}
		
		//If we are here, the following holds true : 
		// - There is a readable (XML) File
		// - There is at least one element available with id given by p_itemGroupId
		//   and tagname "g" available.
		
		static const QString itemTagName( enclosingTagName );
		static const QString itemIdName(  enclosingIdName  );
		
		// Remove all found elements from the lookup table, in case it is not
		// empty : 
		
		this->m_itemsTable.clear();
		
		// Free the currently used QSvgRenderer and allocate a new one :
		
		if( this->m_renderer )
		{
			delete	this->m_renderer;
		}
		
		this->m_renderer = new QSvgRenderer( p_themeFile.fileName() );
		
		//Check whether the allocation was successfull -> pure paranoia
		
		if( !this->m_renderer )
		{
			return false;
		}
		
		for( 
			 QDomNode next( itemsEnclosingElement.firstChild() ) ; 
			 !next.isNull() ; next = next.nextSibling())
		{
			//Next check whether the next sibling is an element itself,
			//has an group tag name and has any attribute 
			
			if( ( next.isElement() ) && ( next.nodeName() == itemTagName ) && 
			   	( next.hasAttributes() ) )
			{
				//the content of the variable next has already passed the isElement()
				//test, hence it is safe to access it via an QDomElement*. Upward
				//casting is done using static_cast.
				
				QString nextId( next.toElement().attribute( itemIdName, QString() ) );
				
				if( !nextId.isEmpty() )
				{
					//Use Qt's append rather than the STL style push_back():
					this->m_itemsTable.append( nextId );
				}
			}
		}
		
		//It is the responsibility of the calling method to close the QFile 
		//object properly !!
		
		return true;
	}
}

//end stegui/svgtheme.cpp


