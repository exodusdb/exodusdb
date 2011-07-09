Summary: Exodus Multivalue Database Programming in C++
Name: exodus-c++
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
URL: http://exodusdb.googlecode.com/
Group: Development/Libraries/C and C++
BuildArch: noarch

Requires: libexodus-devel
Requires: gcc-c++

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in C++

%prep
%setup -q

%build
%{__make} make

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)

%if 0%{?rhel_version}
%{_docdir}/packages/lib%{name}
%else
%{_docdir}/lib%{name}
%endif
