Summary: Exodus Multivalue Database Command Line Utilities
Name: exodus-cli
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
URL: http://exodusdb.googlecode.com
Group: Development/Libraries/Other

#BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Command Line Utilities

%prep
%setup -q

%build
%configure
%{__make}

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_bindir}/*

