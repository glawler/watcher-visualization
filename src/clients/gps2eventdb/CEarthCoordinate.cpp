#include "GFC.h"
//#pragma hdrstop

//#pragma hdrstop

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
** Copyright, 1998, Samuel R. Blackburn
**
** $Workfile: CEarthCoordinate.cpp $
** $Revision: 1.1.1.1 $
** $Modtime: 2/07/98 10:34a $
*/

CEarthCoordinate::CEarthCoordinate( void )
{
   m_X_CoordinateInMeters = 0.0;
   m_Y_CoordinateInMeters = 0.0;
   m_Z_CoordinateInMeters = 0.0;
}

CEarthCoordinate::CEarthCoordinate( const CEarthCoordinate& source )
{
   Copy( source );
}

CEarthCoordinate::~CEarthCoordinate( void )
{
   m_X_CoordinateInMeters = 0.0;
   m_Y_CoordinateInMeters = 0.0;
   m_Z_CoordinateInMeters = 0.0;
}

void CEarthCoordinate::Copy( const CEarthCoordinate& source )
{
   m_X_CoordinateInMeters = source.m_X_CoordinateInMeters;
   m_Y_CoordinateInMeters = source.m_Y_CoordinateInMeters;
   m_Z_CoordinateInMeters = source.m_Z_CoordinateInMeters;
}

void CEarthCoordinate::Get( double& x_coordinate, double& y_coordinate, double& z_coordinate ) const
{
   x_coordinate = m_X_CoordinateInMeters;
   y_coordinate = m_Y_CoordinateInMeters;
   z_coordinate = m_Z_CoordinateInMeters;
}

double CEarthCoordinate::GetXCoordinateInMeters( void ) const
{
   return( m_X_CoordinateInMeters );
}

double CEarthCoordinate::GetYCoordinateInMeters( void ) const
{
   return( m_Y_CoordinateInMeters );
}

double CEarthCoordinate::GetZCoordinateInMeters( void ) const
{
   return( m_Z_CoordinateInMeters );
}

void CEarthCoordinate::Set( double x_coordinate, double y_coordinate, double z_coordinate )
{
   m_X_CoordinateInMeters = x_coordinate;
   m_Y_CoordinateInMeters = y_coordinate;
   m_Z_CoordinateInMeters = z_coordinate;
}

void CEarthCoordinate::SetXCoordinateInMeters( double x_coordinate )
{
   m_X_CoordinateInMeters = x_coordinate;
}

void CEarthCoordinate::SetYCoordinateInMeters( double y_coordinate )
{
   m_Y_CoordinateInMeters = y_coordinate;
}

void CEarthCoordinate::SetZCoordinateInMeters( double z_coordinate )
{
   m_Z_CoordinateInMeters = z_coordinate;
}

CEarthCoordinate& CEarthCoordinate::operator=( const CEarthCoordinate& source )
{
   Copy( source );
   return( *this );
}


#if 0
<WFC_DOCUMENTATION>
<HTML>
<HEAD><TITLE>GFC - CEarthCoordinate</TITLE></HEAD>
<BODY>
<H1>CEarthCoordinate</H1>
$Revision: 1.1.1.1 $
<HR>
<H2>Description</H2>
This class encapsulates an Earth-Centered-Earth-Fixed coordinate.
This is also known as a cartesian coordinate. It is made up of three
distances all originating at the center of the Earth.
<H2>Constructors</H2>
<DL COMPACT>
<DT><PRE><B>CEarthCoordinate</B>()
<B>CEarthCoordinate</B>( const CEarthCoordinate& source )</PRE><DD>
Constructs an empty coordinate or copies another <B>CEarthCoordinate</B>.
</DL>
<H2>Methods</H2>
<DL COMPACT>
<DT><PRE>void <B>Copy</B>( const CEarthCoordinate& coordinate )</PRE><DD>
Copies the contents of another <B>CEarthCoordinate</B>.
<DT><PRE>void <B>Get</B>( double& x_coordinate, double& y_coordinate, double& z_coordinate )</PRE><DD>
This allows you to retrieve all the data members in one function call.
<DT><PRE>double <B>GetXCoordinateInMeters</B>( void ) const</PRE><DD>
This method returns the X axis coordinate in meters.
Positive values point towards the intersection of the Prime Meridian and the Equator.
<DT><PRE>double <B>GetYCoordinateInMeters</B>( void ) const</PRE><DD>
This method returns the Y axis coordinate in meters.
Positive values point towards the intersection of 90 degrees east of Prime Meridian and the Equator.
<DT><PRE>double <B>GetZCoordinateInMeters</B>( void ) const</PRE><DD>
This method returns the Z axis coordinate in meters.
Positive values point towards the North Pole, negative values towards the South Pole.
<DT><PRE>void <B>Set</B>( double x_coordinate, double y_coordinate, double z_coordinate )</PRE><DD>
This lets you set all of the data members in a single function call.
<DT><PRE>void <B>SetXCoordinateInMeters</B>( double x_coordinate )</PRE><DD>
This method sets the X axis coordinate in meters.
<DT><PRE>void <B>SetYCoordinateInMeters</B>( double y_coordinate )</PRE><DD>
This method sets the Y axis coordinate in meters.
<DT><PRE>void <B>SetZCoordinateInMeters</B>( double z_coordinate )</PRE><DD>
This method sets the Z axis coordinate in meters.
</DL>
<H2>Operators</H2>
<DL COMPACT>
<DT><PRE><B>=</B> ( const CEarthCoordinate& source )</PRE><DD>
Basically calls <B>Copy</B>().
</DL>
<H2>Example</H2>
<PRE><CODE>#include &lt;stdio.h&gt;
#include &lt;GFC.h&gt;
//#pragma hdrstop

void main( void )
{
   CPolarCoordinate here;
   <B>CEarthCoordinate</B> there;

   // here is 39 degrees 12.152 minutes North Latitude, 76 degrees 46.795 minutes West Longitude
   here.SetUpDownAngleInDegrees(     CMath::ConvertDegreesMinutesSecondsCoordinateToDecimalDegrees(  39.0, 12.152, 0.0 ) );
   here.SetLeftRightAngleInDegrees(  CMath::ConvertDegreesMinutesSecondsCoordinateToDecimalDegrees( -76.0, 46.795, 0.0 ) );
   here.SetDistanceFromSurfaceInMeters( 1000.0 );

   CEarth earth;
   
   earth.Convert( here, there );
}</CODE></PRE>
<I>Copyright, 1998, Samuel R. Blackburn</I><BR>
$Workfile: CEarthCoordinate.cpp $<BR>
$Modtime: 2/07/98 10:34a $
</BODY>
</HTML>
</WFC_DOCUMENTATION>
#endif

