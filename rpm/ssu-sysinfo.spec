Name:       ssu-sysinfo

Summary:    Tools and libraries for getting ssu information without D-Bus IPC
Version:    1.1.3
Release:    0
Group:      System/System Control
License:    LGPLv2.1+ and BSD
URL:        https://git.merproject.org/mer-core/ssu-sysinfo
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
This package contains command line tools and libraries for
obtaining static ssu information without requiring D-Bus IPC /
SSU daemon.

%package devel
Summary:    Development files for ssu-sysinfo
Requires:   %{name} = %{version}-%{release}

%description devel
This package contains headers and pkg-config files required for
building applications that need to access static ssu related
information without using D-Bus IPC.

%prep
%setup -q -n %{name}-%{version}

%build
util/verify_version.sh
unset LD_AS_NEEDED
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/libssusysinfo.so.*
%{_bindir}/ssu-sysinfo
%doc COPYING

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/ssusysinfo
%{_includedir}/ssusysinfo/*.h
%{_libdir}/libssusysinfo.so
%dir %{_libdir}/pkgconfig
%{_libdir}/pkgconfig/*.pc
