%define namebase pgexodus
%define majorver 9
%define minorver 6
%define microver 0
%define sonamever0 9
%define sonamever 9.0.6

Summary: Exodus Multivalue Database Plugin to PostgreSQL
Name: lib%{namebase}-9_6-9
Provides: lib%{namebase}
Version: %{majorver}.%{minorver}.%{microver}
%define baseversion %{majorver}.%{minorver}
Release: 1
Source: lib%{namebase}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries/C and C++
URL: http://devwiki.neosys.com

Requires: postgresql-server
BuildRequires: postgresql-devel
BuildRequires: gcc-c++

%if 0%{?suse_version} > 0
BuildRequires: pkg-config
%endif

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Pick/Multivalue database programming using PostgreSQL
This postgres library enables sort/select/index by
dictionary field name.

%{!?_pgpkglibdir: %define _pgpkglibdir %(A=`pg_config --pkglibdir`; echo ${A})}

%prep
#http://www.rpm.org/max-rpm/s1-rpm-inside-macros.html
#can cater for tar which doesnt unpack as per package name
%setup -q -n lib%{namebase}-%{version}

%build
%configure
%{__make}

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post

/sbin/ldconfig ||:

/etc/init.d/postgresql start || :

installexodus-postgresql

%postun -p /sbin/ldconfig

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_libdir}/%{namebase}*
%{_bindir}/installexodus-postgresql
%{_bindir}/installexodus-postgresql2
%{_pgpkglibdir}/%{namebase}.so
