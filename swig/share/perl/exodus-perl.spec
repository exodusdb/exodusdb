Summary: Exodus Multivalue Database Programming in Perl
Name: exodus-perl
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
URL: http://exodusdb.googlecode.com
Group: Development/Libraries/Perl

Requires: perl

BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRequires: swig

%if 0%{?fedora}
BuildRequires: perl-devel
%else
BuildRequires: perl
%endif

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%{!?_perlmoddir: %define _perlmoddir %(A=`perl -e 'print @INC[0]'`; echo ${A})}

%description
Exodus Multivalue Database Programming in Perl

%prep
%setup -q

%build
%{__make} make

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post -p /sbin/ldconfig
 
%postun -p /sbin/ldconfig
 
%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_perlmoddir}/*.pm
%{_libdir}/exo.so

%if 0%{?rhel_version} || 0%{?fedora}
%{_docdir}/packages/lib%{name}
%else
%{_docdir}/lib%{name}
%endif