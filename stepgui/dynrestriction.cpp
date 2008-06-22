#include "dynrestriction.h"
#include <limits>

namespace StepGui
{
	DynRestriction::DynRestriction( QObject* p_parent ) :
		QObject( p_parent ),
		m_restrictions( DynRestriction::WRITEABLE )
	{
		//
	}
	
	
	DynRestriction::DynRestriction( const RestrictionState p_restrictions, QObject* p_parent ) :
		QObject( p_parent ),
		m_restrictions( p_restrictions )
	{
		//
	}
	
	bool DynRestriction::isVisible() const
	{
		return !( m_restrictions.testFlag( DynRestriction::HIDDEN ) );
	}
	
	bool DynRestriction::isWriteable() const
	{
		return ( m_restrictions.testFlag( DynRestriction::WRITEABLE ) );
	}
	
	void DynRestriction::setFlag( const RestrictionState p_restrictions )
	{
		m_restrictions |= p_restrictions;
	}
	
	void DynRestriction::setHidden( const bool p_hidden )
	{
		if( p_hidden && !m_restrictions.testFlag( DynRestriction::BOUNDED ) && 
		    !m_restrictions.testFlag( DynRestriction::WRITEABLE ) )
		{
			m_restrictions |= DynRestriction::HIDDEN;
		}
		else
		{
			m_restrictions &= ~HIDDEN;
		}
		
		return;
	}
	
	void DynRestriction::setWriteable( const bool p_writeable )
	{
		if( p_writeable )
		{
			this->m_restrictions |= DynRestriction::WRITEABLE;
		}
		else
		{
			this->m_restrictions &= ~WRITEABLE;
		}
		
		return;
	}
	
	/*
	############################################################################
	############################################################################
	############################################################################
	*/
	
	DynDoubleRestriction::DynDoubleRestriction( QObject* p_parent ) :
		DynRestriction( DynRestriction::WRITEABLE | DynRestriction::BOUNDED, p_parent ), 
		m_validator()
	{
		m_validator = new QDoubleValidator( this );
	}
	
	DynDoubleRestriction::DynDoubleRestriction( 
		const double& p_bottom, const double& p_top,
		const BoundaryDomain p_domainType, 
		const int p_decimals, QObject* p_parent ) :
		
		DynRestriction( DynRestriction::WRITEABLE | DynRestriction::BOUNDED, p_parent ),
		m_validator()
	{
		double bottom( p_bottom );
		double top( p_top );
		
		if( p_domainType.testFlag( ALWAYS_POSITIVE ) )
		{
			bottom = std::max( p_bottom, (double)0.0L );
		}
		else if( p_domainType.testFlag( ALWAYS_NEGATIVE ) )
		{
			top = std::min( p_top, (double)0.0L );
		}
		
		m_validator = new QDoubleValidator( bottom, top, p_decimals, this );
	}
	
	double DynDoubleRestriction::bottom() const
	{
		return m_validator->bottom();
	}
	
	double DynDoubleRestriction::top() const
	{
		return m_validator->top();
	}
	
	int DynDoubleRestriction::decimals() const
	{
		return m_validator->decimals();
	}
	
	bool DynDoubleRestriction::isBounded() const
	{
		return ( m_restrictions & DynRestriction::BOUNDED );
	}
	
	void DynDoubleRestriction::fixup( QString& p_valuestr ) const
	{
		m_validator->fixup( p_valuestr );
	}
	
	QValidator::State DynDoubleRestriction::validate( QString& p_valuestr, int& p_position ) const
	{
		return m_validator->validate( p_valuestr, p_position );
	}
		
	void DynDoubleRestriction::fixup( double& p_value ) const
	{
		if( m_restrictions.testFlag( DynRestriction::BOUNDED ) )
		{
			if( p_value < m_validator->bottom() )
			{
				p_value = m_validator->bottom();
			}
			else if( p_value > m_validator->top() )
			{
				p_value = m_validator->top();
			}
		}
	}
	
	QValidator::State DynDoubleRestriction::validate( const double& p_value ) const
	{
		if( m_restrictions.testFlag( DynRestriction::BOUNDED ) && 
			( ( p_value > m_validator->top() ) || ( p_value < m_validator->bottom() ) ) )
		{
			return QValidator::Invalid;
		}
		
		return QValidator::Acceptable;
	}
	
	void DynDoubleRestriction::setBounded( const bool p_bounded )
	{
		if( p_bounded )
		{
			this->m_restrictions |= DynRestriction::BOUNDED;
			this->m_restrictions |= DynRestriction::WRITEABLE;
			this->m_restrictions &= ~DynRestriction::HIDDEN;
		}
		else
		{
			this->m_restrictions &= ~DynRestriction::BOUNDED;
		}
	}
	
	void DynDoubleRestriction::setBottom( const double& p_bottom )
	{
		this->_setBottom( p_bottom );
	}
	
	void DynDoubleRestriction::setTop( const double& p_top )
	{
		this->_setTop( p_top );
	}
	
	void DynDoubleRestriction::setDecimals( const int p_decimals )
	{
		m_validator->setDecimals( p_decimals );
	}
	
	void DynDoubleRestriction::setRange( 
		const double& p_bottom,
		const double& p_top,
		const BoundaryDomain p_domain,
		const int p_decimals )
	{
		m_domain = p_domain;
		
		this->_setTop( p_top );
		this->_setBottom( p_bottom );
		m_validator->setDecimals( p_decimals );
	}
	
	void DynDoubleRestriction::_setBottom( const double& p_bottom )
	{
		if( m_domain.testFlag( DynRestriction::WHOLE_DOMAIN ) )
		{
			m_validator->setBottom( p_bottom );
		}
		else if( m_domain.testFlag( DynRestriction::ALWAYS_POSITIVE ) )
		{
			m_validator->setBottom( std::max( p_bottom, (double)0.0L ) );
		}
		else
		{
			m_validator->setBottom( ( p_bottom < 0.0 ) ? p_bottom : std::numeric_limits< double >::min() );
		}
	}
	
	void DynDoubleRestriction::_setTop( const double& p_top )
	{
		if( m_domain.testFlag( DynRestriction::WHOLE_DOMAIN ) )
		{
			m_validator->setTop( p_top );
		}
		else if( m_domain.testFlag( DynRestriction::ALWAYS_POSITIVE ) )
		{
			m_validator->setTop( ( p_top > 0.0 ) ? p_top : std::numeric_limits< double >::max() );
		}
		else
		{
			m_validator->setTop( std::min( p_top, (double)0.0L ) );
		}
	}
}

//end stepgui/dynrestriction.hpp


