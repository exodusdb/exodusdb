Summary: Exodus Multivalue Database Programming in PHP
Name: exodus-php
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
URL: http://exodusdb.googlecode.com/
Group: Development/Libraries/PHP
Requires: php
BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRequires: swig
BuildRequires: php-devel
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in PHP

%{!?_phpmoddir: %define _phpmoddir %(A=`php-config --extension-dir`; echo ${A})}

%prep
%setup -q

%build
make make

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post
test -f /etc/php.d && echo -e "; Enable exodus extension module\nextension=exo.so" > /etc/php.d/exodus.ini
test -f /etc/php5/conf.d && echo -e "; Enable exodus extension module\nextension=exo.so" > /etc/php5/conf.d/exodus.ini
/sbin/ldconfig
exit 0

%postun
test -f /etc/php.d/exodus.ini && rm /etc/php.d/exodus.ini
test -f /etc/php5/conf.d/exodus.ini && rm /etc/php5/conf.d/exodus.ini
/sbin/ldconfig
exit 0

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_phpmoddir}

%if 0%{?rhel_version}
%{_docdir}/packages/lib%{name}
/usr/share/php
%else
%{_docdir}/lib%{name}
/usr/share/php5
%endif