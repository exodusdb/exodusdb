#%define debug_package %{nil}

Summary: Exodus Multivalue Database Programming in Java
Name: exodus-java
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: libexodus
Requires: java
BuildRequires: libexodus
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
#%configure
#{__make} build
make make

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
%{_libdir}
/usr/share/java

#%doc /usr/local/info/exodus.info
#%doc %attr(0444,root,root) /usr/local/man/man1/exodus.1
#%doc COPYING AUTHORS README NEWS

%doc %{_docdir}/lib%{name}/examples