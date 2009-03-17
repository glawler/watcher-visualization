Name:		libidmef
Version:	0.7.3_beta_McAfee20050325
Release:	1%{?dist}
Summary:	Library for IETF Intrusion Detection Exchange Format

#Group:		
License:	GPL
URL:		http://sourceforge.net/projects/libidmef/
Source0:	%{name}-%{version}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)
Packager:	Michael Elkins <michael.elkins@sparta.com>
Vendor:		Sparta, Inc

#BuildRequires:	
#Requires:	

%description
LibIDMEF is an implementation of the Internet Engineering Task Force (IETF), Intrusion Detection Exchange Format Charter Working Group (IDWG), draft standard Intrusion Detection Message Exchange Format (IDMEF) protocol.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
/usr/lib/libidmef.so
/usr/lib/libidmef.so.0
/usr/lib/libidmef.so.0.7.3
/usr/share/idmef-message.dtd
/usr/share/idmef-messages/*.xml
/usr/share/idmef-messages/vendorco.dtd

%changelog

#
# -devel subpackage
#
%package devel

Summary:	Development files for IETF Intrusion Detection Exchange Format

%description devel
LibIDMEF is an implementation of the Internet Engineering Task Force (IETF), Intrusion Detection Exchange Format Charter Working Group (IDWG), draft standard Intrusion Detection Message Exchange Format (IDMEF) protocol.

This package contains the development headers and libraries.

%files devel
%defattr(-,root,root,-)
/usr/bin/libidmef-config
/usr/include/libidmef/idmefxml.h
/usr/include/libidmef/idmefxml_gen.h
/usr/include/libidmef/idmefxml_global.h
/usr/include/libidmef/idmefxml_parse.h
/usr/include/libidmef/idmefxml_types.h
/usr/lib/libidmef.a
/usr/lib/libidmef.la
/usr/lib/pkgconfig/libidmef.pc
