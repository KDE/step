#ifndef STEPGUI_DYNAMIC_RESTRICTION_CLASS
#define STEPGUI_DYNAMIC_RESTRICTION_CLASS

#include <QObject>
#include <QFlags>
#include <QDoubleValidator>

namespace StepGui
{
	class DynRestriction : public QObject
	{
		Q_OBJECT
		
		public:

			enum RestrictionStates
			{
				HIDDEN = 0x01,
				WRITEABLE = 0x02,
				BOUNDED = 0x04
			};
			
			Q_DECLARE_FLAGS( RestrictionState, RestrictionStates )
			
			typedef enum BoundaryDomains
			{
				WHOLE_DOMAIN	= 0x0,
				ALWAYS_POSITIVE	= 0x1,
				ALWAYS_NEGATIVE = 0x2
			};
			
			Q_DECLARE_FLAGS(BoundaryDomain, BoundaryDomains)
			
			DynRestriction( QObject* =0 );
			DynRestriction( const RestrictionState, QObject* =0 );

			bool isVisible() const;
			bool isWriteable() const;
			void setFlag( const RestrictionState );

		public slots:
			
			void setHidden( const bool =true );
			void setWriteable( const bool =true);
			

		protected:
			
			RestrictionState m_restrictions;
	};
	
	Q_DECLARE_OPERATORS_FOR_FLAGS( DynRestriction::RestrictionState )
	Q_DECLARE_OPERATORS_FOR_FLAGS( DynRestriction::BoundaryDomain )	
	
	
	class DynDoubleRestriction : public DynRestriction
	{
		Q_OBJECT
		
		public:
		
		DynDoubleRestriction( QObject* = 0 );
		DynDoubleRestriction( const double&, const double&,
			const BoundaryDomain = WHOLE_DOMAIN, const int = 0,
			QObject* = 0 );
		
		double 	bottom() const;
		double 	top() 	const;
		int    	decimals() const;
		
		bool	isBounded() const;
		
		void 	fixup( QString& ) const;
		QValidator::State validate( QString&, int& ) const;
		
		void 	fixup( double& ) const;
		QValidator::State validate( const double& ) const;
		
		public slots:
		
		void setBounded( const bool = true );
		
		void setBottom( const double& );
		void setTop( const double& );
		void setDecimals( const int );
		
		void setRange( const double&, const double&, 
			const BoundaryDomain = DynDoubleRestriction::WHOLE_DOMAIN, 
			const int = 0 );
		
		private:
		
		void _setBottom( const double& );
		void _setTop( const double& );
		
		BoundaryDomain		m_domain;
		QDoubleValidator*	m_validator;
	};

}




#endif

