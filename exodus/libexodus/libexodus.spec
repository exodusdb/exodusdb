%define baseversion 11.6
%define debug_package %{nil}

Summary: Exodus Multivalue Database Programming in any language
Name: libexodus
Version: 11.6.1
Release: 1
Source0: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: postgresql-libs
Requires: boost-date_time
Requires: boost-filesystem
Requires: boost-regex
Requires: boost-system
Requires: boost-thread
Requires: libicu
BuildRequires: postgresql-devel
BuildRequires: boost-devel
BuildRequires: libicu-devel
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Pick/Multivalue database programming in any language

%prep
%setup -q

%build
%configure
%{__make}

%install
if [ "$RPM_BUILD_ROOT" != "/var/tmp/%{name}-%{version}-%{release}-root" ]
then
  echo
  echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  echo @                                                                    @
  echo @  RPM_BUILD_ROOT is not what I expected.  Please clean it yourself. @
  echo @                                                                    @
  echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  echo
else
  echo Cleaning RPM_BUILD_ROOT: "$RPM_BUILD_ROOT"
  rm -rf "$RPM_BUILD_ROOT"
fi
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
if [ "$RPM_BUILD_ROOT" != "/var/tmp/%{name}-%{version}-%{release}-root" ]
then
 echo
 echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 echo @                                                                    @
 echo @  RPM_BUILD_ROOT is not what I expected.  Please clean it yourself. @
 echo @                                                                    @
 echo @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 echo
else
 echo Cleaning RPM_BUILD_ROOT: "$RPM_BUILD_ROOT"
 rm -rf "$RPM_BUILD_ROOT"
fi

%files
%defattr(-,root,root)
/usr/include/exodus-%{baseversion}/
/usr/include/exodus
%{_libdir}/%{name}*
%{_libdir}/pkgconfig/exodus.pc

#%doc /usr/local/info/exodus.info
#%doc %attr(0444,root,root) /usr/local/man/man1/exodus.1
#%doc COPYING AUTHORS README NEWS
