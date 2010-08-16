/*
** Author: Samuel R. Blackburn
** Internet: sblackbu@erols.com
**
** You can use it any way you like as long as you don't try to sell it.
**
** Any attempt to sell GFC in source code form must have the permission
** of the original author. You can produce commercial executables with
** GFC but you can't sell GFC.
**
** Copyright, 1997, Samuel R. Blackburn
**
** $Workfile: cbubble.cpp $
** $Revision: 1.1.1.1 $
** $Modtime: 3/23/97 11:39a $
*/

inline double CMath::AbsoluteValue( const double& value )
{
   return( ::fabs( value ) );
}

inline double CMath::ArcCosine( const double& value )
{
   return( ::acos( value ) );
}

inline double CMath::ArcSine( const double& value )
{
   return( ::asin( value ) );
}

inline double CMath::ArcTangent( const double& value )
{
   return( ::atan( value ) );
}

inline double CMath::ArcTangentOfYOverX( const double& y, const double& x )
{
   return( ::atan2( y, x ) );
}

inline double CMath::Ceiling( const double& value )
{
   return( ::ceil( value ) );
}

inline void CMath::ConvertDecimalDegreesToDegreesMinutesSeconds( double decimal_degrees, double& degrees, double& minutes, double& seconds )
{
   double fractional_part = 0.0;

   double integer_part = 0;

   fractional_part = ::modf( decimal_degrees, &integer_part );

   degrees = integer_part;

   if ( decimal_degrees < 0.0 )
   {
      fractional_part *= (-1.0);
   }

   minutes = fractional_part * 60.0;

   fractional_part = ::modf( minutes, &integer_part );

   minutes = integer_part;

   seconds = fractional_part * 60.0;
}

inline double CMath::ConvertDegreesMinutesSecondsCoordinateToDecimalDegrees( double degrees, double minutes, double seconds )
{
   double decimal_degrees = 0.0;

   decimal_degrees = degrees;

   if ( decimal_degrees < 0.0 )
   {
      decimal_degrees *= (-1.0);
   }

   decimal_degrees += (double) ( minutes / 60.0 );
   decimal_degrees += (double) ( seconds / 3600.0 );

   if ( degrees < 0.0 )
   {
      decimal_degrees *= (-1.0);
   }

   return( decimal_degrees );
}

inline double CMath::ConvertDegreesToRadians( const double& degrees )
{
   double radians           = 0.0;
   double pi_divided_by_180 = CMath::Pi() / 180.0;
   
   radians = degrees * pi_divided_by_180;

   return( radians );
}

inline double CMath::ConvertRadiansToDegrees( const double& radians )
{
   double degrees = 0.0;

   double conversion_factor = 180.0 / CMath::Pi();

   degrees = radians * conversion_factor;

   return( degrees );
}

inline double CMath::Cosine( const double& value )
{
   return( ::cos( value ) );
}

inline double CMath::HyperbolicCosine( const double& value )
{
   return( ::cosh( value ) );
}

inline double CMath::Pi( void )
{
   return( 3.1415926535897932384626433832795028841971693993751058209749445923078164 );
}

inline double CMath::Sine( const double& value )
{
   return( ::sin( value ) );
}

inline double CMath::SquareRoot( const double& value )
{
   return( ::sqrt( value ) );
}

