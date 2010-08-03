%define	_project	cta
%define	_name		watcher
%define	_prefix		/usr/%{_project}

Summary:	ARL1X/CTA Watcher
Name:		%{_project}-%{_name}
Version:	20100803
Release:	1
License:	Other
Group:		Applications/CTA
URL:		http://fill.me.in/
Vendor:		SPARTA
Source:		%{name}-%{version}.tar.gz

BuildRoot:	%{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Prefix:		%{_prefix}

BuildRequires:	automake
BuildRequires:	autoconf
BuildRequires:	libtool
BuildRequires:	pkgconfig
BuildRequires:  boost-devel
BuildRequires:	libconfig-devel
BuildRequires:	sqlite-devel
BuildRequires:	cta-logger-devel
BuildRequires:	libkml-devel
BuildRequires:	uriparser-devel

BuildRequires:	qt-devel

%description

%package lib
Summary:	ARL1X/CTA Watcher library files.
Group:		Libraries/CTA

%description lib

%package devel
Summary:	ARL1X/CTA Watcher development files.
Group:		Development/CTA
Requires:	%{name}-lib = %{version}

%description devel

%prep
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%setup -n %{_name}

%build
export PKG_CONFIG_PATH=%{_libdir}/pkgconfig
cd src
./autogen.sh
%configure --enable-earthWatcher
make DESTDIR=$RPM_BUILD_ROOT

%install
cd src
make DESTDIR=$RPM_BUILD_ROOT install

%clean
if [ "$RPM_BUILD_ROOT" ] && [ "$RPM_BUILD_ROOT" != "/" ]; then
	rm -rf $RPM_BUILD_ROOT
fi

%files
%defattr(-,root,root,-)

%dir %{_bindir}
%{_bindir}/connectivity2dot
%{_bindir}/earthWatcher
%{_bindir}/messageStream2Text
%{_bindir}/randomScenario
%{_bindir}/routeFeeder
%{_bindir}/sendColorMessage
%{_bindir}/sendConnectivityMessage
%{_bindir}/sendDataPointMessage
%{_bindir}/sendEdgeMessage
%{_bindir}/sendGPSMessage
%{_bindir}/sendLabelMessage
%{_bindir}/sendNodePropertiesMessage
%{_bindir}/showClock
%{_bindir}/%{_name}d

%files lib
%defattr(444,root,root,755)

%dir %{_libdir}
%{_libdir}/lib%{_name}msg.so
%{_libdir}/lib%{_name}msg.so.0
%{_libdir}/lib%{_name}msg.so.0.0.0

%files devel
%defattr(444,root,root,755)

%dir %{_includedir}
%dir %{_includedir}/lib%{_name}
%{_includedir}/lib%{_name}/client.h
%{_includedir}/lib%{_name}/colorMessage.h
%{_includedir}/lib%{_name}/colors.h
%{_includedir}/lib%{_name}/connection_fwd.h
%{_includedir}/lib%{_name}/connectivityMessage.h
%{_includedir}/lib%{_name}/dataPointMessage.h
%{_includedir}/lib%{_name}/edgeMessage.h
%{_includedir}/lib%{_name}/gpsMessage.h
%{_includedir}/lib%{_name}/initConfig.h
%{_includedir}/lib%{_name}/labelMessage.h
%{_includedir}/lib%{_name}/message.h
%{_includedir}/lib%{_name}/messageHandler.h
%{_includedir}/lib%{_name}/messageStreamFilter.h
%{_includedir}/lib%{_name}/messageStreamFilterMessage.h
%{_includedir}/lib%{_name}/messageTypesAndVersions.h
%{_includedir}/lib%{_name}/message_fwd.h
%{_includedir}/lib%{_name}/nodePropertiesMessage.h
%{_includedir}/lib%{_name}/nodeStatusMessage.h
%{_includedir}/lib%{_name}/sendMessageHandler.h
%{_includedir}/lib%{_name}/singletonConfig.h
%{_includedir}/lib%{_name}/watcherColors.h
%{_includedir}/lib%{_name}/watcherGlobalFunctions.h
%{_includedir}/lib%{_name}/watcherMessageFwd.h
%{_includedir}/lib%{_name}/watcherTypes.h

%dir %{_libdir}
%{_libdir}/lib%{_name}.a
%{_libdir}/lib%{_name}msg.a
%{_libdir}/lib%{_name}msg.la
%{_libdir}/lib%{_name}utils.a

%dir %{_libdir}/pkgconfig
%{_libdir}/pkgconfig/%{_name}.pc
%{_libdir}/pkgconfig/%{_name}msg.pc
%{_libdir}/pkgconfig/%{_name}utils.pc

%changelog
