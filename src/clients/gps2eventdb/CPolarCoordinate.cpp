#include "GFC.h"
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
** $Workfile: CPolarCoordinate.cpp $
** $Revision: 1.1.1.1 $
** $Modtime: 2/07/98 10:35a $
*/

CPolarCoordinate::CPolarCoordinate( void )
{
   m_UpDownAngleInDegrees        = 0.0;
   m_LeftRightAngleInDegrees     = 0.0;
   m_DistanceFromSurfaceInMeters = 0.0;
}

CPolarCoordinate::CPolarCoordinate( const CPolarCoordinate& source )
{
   Copy( source );
}

CPolarCoordinate::~CPolarCoordinate( void )
{
   m_UpDownAngleInDegrees        = 0.0;
   m_LeftRightAngleInDegrees     = 0.0;
   m_DistanceFromSurfaceInMeters = 0.0;
}

void CPolarCoordinate::Copy( const CPolarCoordinate& source )
{
   m_UpDownAngleInDegrees        = source.m_UpDownAngleInDegrees;
   m_LeftRightAngleInDegrees     = source.m_LeftRightAngleInDegrees;
   m_DistanceFromSurfaceInMeters = source.m_DistanceFromSurfaceInMeters;
}

void CPolarCoordinate::Get( double& up_down_angle, double& left_right_angle, double& length ) const
{
   up_down_angle    = m_UpDownAngleInDegrees;
   left_right_angle = m_LeftRightAngleInDegrees;
   length           = m_DistanceFromSurfaceInMeters;
}

double CPolarCoordinate::GetUpDownAngleInDegrees( void ) const
{
   return( m_UpDownAngleInDegrees );
}

double CPolarCoordinate::GetLeftRightAngleInDegrees( void ) const
{
   return( m_LeftRightAngleInDegrees );
}

double CPolarCoordinate::GetDistanceFromSurfaceInMeters( void ) const
{
   return( m_DistanceFromSurfaceInMeters );
}

void CPolarCoordinate::Set( double up_down_angle, double left_right_angle, double length )
{
   m_UpDownAngleInDegrees        = up_down_angle;
   m_LeftRightAngleInDegrees     = left_right_angle;
   m_DistanceFromSurfaceInMeters = length;
}

void CPolarCoordinate::SetUpDownAngleInDegrees( double up_down_angle )
{
   m_UpDownAngleInDegrees = up_down_angle;
}

void CPolarCoordinate::SetLeftRightAngleInDegrees( double left_right_angle )
{
   m_LeftRightAngleInDegrees = left_right_angle;
}

void CPolarCoordinate::SetDistanceFromSurfaceInMeters( double distance_from_surface )
{
   m_DistanceFromSurfaceInMeters = distance_from_surface;
}

CPolarCoordinate& CPolarCoordinate::operator=( const CPolarCoordinate& source )
{
   Copy( source );
   return( *this );
}

#if 0
<WFC_DOCUMENTATION>
<HTML>
<HEAD><TITLE>GFC - CPolarCoordinate</TITLE></HEAD>
<BODY>
<H1>CPolarCoordinate</H1>
$Revision: 1.1.1.1 $
<HR>
<H2>Description</H2>
This class encapsulates a polar coordinate. A polar coordinate is
made up of two angles and one distance. The angles originate at the
center of the Earth. One angle measures up and down (latitude) while
the other measures left and right (longitude). The distance is the
altitude above the surface of the Earth. Positive angles approach north
and east while negative values for an angle run south and west.
<H2>Constructors</H2>
<DL COMPACT>
<DT><PRE><B>CPolarCoordinate</B>()
<B>CPolarCoordinate</B>( const CPolarCoordinate& source )</PRE><DD>
Constructs an empty coordinate or copies another <B>CPolarCoordinate</B>.
</DL>
<H2>Methods</H2>
<DL COMPACT>
<DT><PRE>void <B>Copy</B>( const CPolarCoordinate& coordinate )</PRE><DD>
Copies the contents of another <B>CPolarCoordinate</B>.
<DT><PRE>void <B>Get</B>( double& up_down_angle, double& left_right_angle, double& distance )</PRE><DD>
This allows you to retrieve all the data members in one function call.
<DT><PRE>double <B>GetUpDownAngleInDegrees</B>( void ) const</PRE><DD>
This method returns the up/down angle in degrees.
<DT><PRE>double <B>GetLeftRightAngleInDegrees</B>( void ) const</PRE><DD>
This method returns the left/right angle in degrees.
<DT><PRE>double <B>GetDistanceFromSurfaceInMeters</B>( void ) const</PRE><DD>
This method returns the distance from the surface of the ellipsoid.
<DT><PRE>void <B>Set</B>( double up_down_angle, double left_right_angle, double distance )</PRE><DD>
This lets you set all of the data members in a single function call.
<DT><PRE>void <B>SetUpDownAngleInDegrees</B>( double up_down_angle )</PRE><DD>
This method sets the up/down angle.
<DT><PRE>void <B>SetLeftRightAngleInDegrees</B>( double left_right_angle )</PRE><DD>
This method sets the left/right angle.
<DT><PRE>void <B>SetDistanceFromSurfaceInMeters</B>( double distance )</PRE><DD>
This method sets the distance from the surface of the ellipsoid.
</DL>
<H2>Operators</H2>
<DL COMPACT>
<DT><PRE><B>=</B> ( const CPolarCoordinate& source )</PRE><DD>
Basically calls <B>Copy</B>().
</DL>
<H2>Example</H2>
<PRE><CODE>#include &lt;stdio.h&gt;
#include &lt;GFC.h&gt;
//#pragma hdrstop

void main( void )
{
   <B>CPolarCoordinate</B> here;
   <B>CPolarCoordinate</B> there;

   // here is 39 degrees 12.152 minutes North Latitude, 76 degrees 46.795 minutes West Longitude
   here.SetUpDownAngleInDegrees(     CMath::ConvertDegreesMinutesSecondsCoordinateToDecimalDegrees(  39.0, 12.152, 0.0 ) );
   here.SetLeftRightAngleInDegrees(  CMath::ConvertDegreesMinutesSecondsCoordinateToDecimalDegrees( -76.0, 46.795, 0.0 ) );
   here.SetDistanceFromSurfaceInMeters( 1000.0 );

   there = here;
}</CODE></PRE>
<I>Copyright, 1998, Samuel R. Blackburn</I><BR>
$Workfile: CPolarCoordinate.cpp $<BR>
$Modtime: 2/07/98 10:35a $
</BODY>
</HTML>
</WFC_DOCUMENTATION>
#endif
