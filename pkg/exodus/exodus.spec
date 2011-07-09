Summary: Exodus Multivalue Database
Name: exodus
Version: 11.6.1
Release: 1
Source: %{name}-%{version}.tar.gz
License: MIT http://www.opensource.org/licenses/mit-license.php
Group: Development/Languages/Other
URL: http://devwiki.neosys.com
BuildArch: noarch

Requires: exodus-cli
Requires: exodus-perl
Requires: exodus-python

BuildRoot: /var/tmp/%{name}-%{version}-%{release}-root

%description
Multivalue database and programming.

%prep
%setup -q

%build
##%%configure
##{__make} build
#make make

%install
#%%{__make} install DESTDIR="$RPM_BUILD_ROOT"

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)

%doc COPYING AUTHORS README NEWS