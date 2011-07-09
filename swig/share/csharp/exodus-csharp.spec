Summary: Exodus Multivalue Database Programming in C#
Name: exodus-csharp
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
URL: http://exodusdb.googlecode.com
Group: Development/Libraries/Other

Requires: mono-core
BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRequires: swig
BuildRequires: mono-devel
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in C#

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
%{_libdir}/libexodus*
/usr/share/csharp

%if 0%{?rhel_version} || 0%{?fedora}
%{_docdir}/packages/lib%{name}
%else
%{_docdir}/lib%{name}
%endif