Summary: Exodus Multivalue Database Programming in Java
Name: exodus-java
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries/Java
URL: http://exodusdb.googlecode.com/

Requires: java
BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRequires: swig
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%if 0%{?rhel_version} + 0%{?fedora_version} > 0
BuildRequires: java-1.6.0-openjdk-devel
%else
BuildRequires: java-devel
%endif

%description
Exodus Multivalue Database Programming in Java

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
%{_libdir}/libjexodus*
/usr/share/java/jexodus*

%if 0%{?rhel_version} || 0%{?fedora}
%{_docdir}/packages/lib%{name}
%else
%{_docdir}/lib%{name}
%endif
