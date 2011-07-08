#%define debug_package %{nil}

Summary: Exodus Multivalue Database
Name: exodus
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: exodus-cli
Requires: exodus-perl
Requires: exodus-python
Requires: libpgexodus
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database

%prep
%setup -q

%build
##%configure
##{__make} build
#make make

%install
#%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%post

%postun

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

%doc %attr(0444,root,root) COPYING AUTHORS README NEWS