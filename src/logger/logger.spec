%define	_project	watcher
%define	_name		logger
%define	_prefix		/usr/%{_project}

Summary:	A small wrapper around Apache's log4cxx library.
Name:		%{_project}-%{_name}
Version:	1.0.0
Release:	1
License:	GPL
Group:		Development/Libraries
URL:		http://code.google.com/p/watcher-visualization
Vendor:		SPARTA
Source:		%{name}-%{version}.tar.gz

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:		%{_prefix}

BuildRequires:	automake
BuildRequires:	autoconf
BuildRequires:  pkgconfig
BuildRequires:	log4cxx-devel
BuildRequires:	libtool

%description

%package devel
Group:		Development/Libraries
Summary:	A small wrapper around Apache's log4cxx library. Exports a standard-ish MACRO-based logging interface. 
Requires:	%{name}	= %{version}

%description devel

%prep
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%setup -n %{_name}

%build
./autogen.sh
%configure
make DESTDIR=$RPM_BUILD_ROOT

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%files
%defattr(-,root,root,-)

%{_libdir}/lib%{_name}.so
%{_libdir}/lib%{_name}.so.1
%{_libdir}/lib%{_name}.so.1.0.0

%files devel
%defattr(-,root,root,-)

%dir %{_includedir}
%{_includedir}/declareLogger.h
%{_includedir}/%{_name}.h

%dir %{_libdir}
%{_libdir}/lib%{_name}.a
%{_libdir}/lib%{_name}.la
%{_libdir}/pkgconfig/%{_name}.pc

%changelog
