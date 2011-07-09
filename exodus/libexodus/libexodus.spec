%define namebase exodus
%define majorver 11
%define minorver 6
%define microver 1
%define sonamever0 11
%define sonamever 11.0.6

Summary: Exodus Multivalue Database Programming in any language
Name: lib%{namebase}-11_6-11
Provides: lib%{namebase}
#version should have no dash - says suse osc
Version: %{majorver}.%{minorver}.%{microver}
%define baseversion %{majorver}.%{minorver}
Release: 1
Source: lib%{namebase}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries/C and C++
URL: http://devwiki.neosys.com

BuildRequires: postgresql-devel
BuildRequires: libicu-devel
BuildRequires: gcc-c++

%if 0%{?suse_version} > 0
BuildRequires: pkg-config
%endif

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%if ( 0%{?rhel_version} && 0%{?rhel_version} < 600 ) || ( 0%{?centos_version} && 0%{?centos_version} < 600 )
Requires: boostbase
BuildRequires: boostbase-devel
%else
#suse cant find boost so lets see if rpmbuild can automatically work out dependency
#Requires: boost
BuildRequires: boost-devel
%endif

%description
Pick/Multivalue database programming in any language


%package devel
Summary: The Exodus C++ headers and shared development libraries
Group: Development/Libraries/C and C++
Requires: %{name} = %{version}-%{release}
Provides: libexodus-devel, libexodus-dev
 
%description devel
Headers and shared object symlinks for the Exodus C++ libraries.


%prep
#http://www.rpm.org/max-rpm/s1-rpm-inside-macros.html
#can cater for tar which doesnt unpack as per package name
%setup -q -n lib%{namebase}-%{version}
#mv lib?{namebase}-?{version}/* .

%build
%configure CPPFLAGS=-I%{_includedir}/pgsql
%{__make}

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_libdir}/lib%{namebase}-%{baseversion}.so.%{sonamever}

%files devel
%defattr(-,root,root)
%{_libdir}/lib%{namebase}.so
%{_libdir}/lib%{namebase}.a
%{_libdir}/lib%{namebase}.la
%{_libdir}/lib%{namebase}-%{baseversion}.so.%{sonamever0}
%{_libdir}/pkgconfig/%{namebase}.pc
%{_includedir}/%{namebase}-%{baseversion}/
%{_includedir}/%{namebase}