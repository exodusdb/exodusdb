#%define debug_package %{nil}

Summary: Exodus Multivalue Database Programming in C++
Name: exodus-c++
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: libexodus-devel
Requires: gcc-c++
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in C++

%prep
%setup -q

%build
##%configure
##{__make} build
#make make

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

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
%defattr(-,root,root)

#%doc /usr/local/info/exodus.info
#%doc %attr(0444,root,root) /usr/local/man/man1/exodus.1
#%doc COPYING AUTHORS README NEWS

%if 0%{?rhel_version}
%{_docdir}/packages/lib%{name}
%{_docdir}/packages/lib%{name}/examples
%else
%{_docdir}/lib%{name}
%{_docdir}/lib%{name}/examples
%endif
