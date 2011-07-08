#%define debug_package %{nil}

Summary: Exodus Multivalue Database Programming in Perl
Name: exodus-perl
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: libexodus
Requires: perl
BuildRequires: libexodus
BuildRequires: libexodus-devel
BuildRequires: gcc-c++
BuildRequires: swig

#%if "%{_vendor}" == "redhat" && 0%{?rhel} < 6 && 0%{?fedora} < 10
%if 0%{?rhel_version}
BuildRequires: perl
%else
BuildRequires: perl
#BuildRequires: perl-devel
%endif

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in Perl

#%{!?_perlmoddir: %define _perlmoddir %(A=`perl -e 'print @INC[0]'`; echo ${A})}

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
%{_perlmoddir}/*.pm
%{_libdir}

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