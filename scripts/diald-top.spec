Summary: maintains top-like display of packets and TTL for diald
Name: diald-top
Version: 2.1pl5
Release: 1
Copyright: GPL
Group: Applications/System
Url: http://diald-top.sourceforge.net/
Packager: Gavin Hurlbut <gjhurlbu@users.sourceforge.net>
Source: http://prdownloads.sourceforge.net/diald-top/diald-top-%{version}.tar.gz
#Patch: diald-top-2.1pl2-rpm.patch
BuildRoot: %{_tmppath}/rpm/%{name}-%{version}
Requires: ncurses, bison, flex

%description
Maintains a 'top'-like listing of all packets and their time to live for use 
with diald (tested with version 0.16 & 0.99.[34]).  Requires ncurses, flex,
bison to compile, and uses named pipes for communication.

%prep
%setup
#%patch -p1
rm -rf ${RPM_BUILD_ROOT}
mkdir -p ${RPM_BUILD_ROOT}/var/run
mknod ${RPM_BUILD_ROOT}/var/run/diald.ctl p

%build
./configure
make squeakyclean
make

%install
make RPM_ROOT="$RPM_BUILD_ROOT" install

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%attr(-,root,root) %doc README INSTALL COPYING
%attr(500,root,root) /usr/local/bin/diald-top
%attr(500,root,root) /usr/local/bin/diald-top-server
%attr(-,root,root) /usr/local/man/man8/diald-top.8
%attr(600,root,root) /var/run/diald.ctl

