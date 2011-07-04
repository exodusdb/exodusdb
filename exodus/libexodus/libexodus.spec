%define baseversion 11.6
%define debug_package %{nil}

Summary: Exodus Multivalue Database Programming in any language
Name: libexodus
Version: 11.6.1
Release: 1
Source0: %{name}-%{version}.tar.gz
#Patch0: boostm4.patch
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: postgresql-libs
#Requires: libicu
BuildRequires: gcc-c++
BuildRequires: postgresql-devel
BuildRequires: libicu-devel
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

#%if "%{_vendor}" == "redhat" && 0%{?rhel} < 6 && 0%{?fedora} < 10
%if ( 0%{?rhel_version} && 0%{?rhel_version} < 600 ) || ( 0%{?centos_version} && 0%{?centos_version} < 600 )
Requires: boostbase
BuildRequires: boostbase-devel
%else
Requires: boost
BuildRequires: boost-devel
%endif

%description
Pick/Multivalue database programming in any language

%prep
%setup -q

#%patch0 -p1

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
