Summary: Exodus Multivalue Database Programming in Python
Name: exodus-python
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Libraries
Requires: python
BuildRequires: libexodus-devel
BuildRequires: libexodus
BuildRequires: gcc-c++
BuildRequires: python
BuildRequires: python-devel
BuildRequires: swig
BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Exodus Multivalue Database Programming in Python

%{!?_pylibdir: %define _pylibdir %(A=`python -c "import os, atexit; print os.path.dirname(atexit.__file__)"`; echo ${A})}

%prep
%setup -q

#pylib=`python -c "import os, atexit; print
#os.path.dirname(atexit.__file__)"` 
#echo "---$pylib---"
#[ -d /usr/lib/python ] || ln -s $pylib /usr/lib/python 

%build
#%configure
#{__make} build
make make

%install
%{__make} install DESTDIR="$RPM_BUILD_ROOT"

find "$RPM_BUILD_ROOT"

%post

%postun

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_pylibdir}/site-packages/

%if 0%{?rhel_version}
%{_docdir}/packages/lib%{name}
%{_docdir}/packages/lib%{name}/examples
%else
%{_docdir}/lib%{name}
%{_docdir}/lib%{name}/examples
%endif