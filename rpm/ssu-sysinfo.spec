Name:       ssu-sysinfo

Summary:    Tools and libraries for getting ssu information without D-Bus IPC
Version:    1.4.1
Release:    0
License:    LGPLv2+ and BSD
URL:        https://github.com/sailfishos/ssu-sysinfo/
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
make _LIBDIR=%{_libdir} %{_smp_mflags}

%install
make _LIBDIR=%{_libdir} install DESTDIR=%{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license COPYING LICENSE.BSD-3CLAUSE LICENSE.LGPL-v2.1
%{_libdir}/libssusysinfo.so.*
%{_bindir}/ssu-sysinfo

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/ssusysinfo
%{_includedir}/ssusysinfo/*.h
%{_libdir}/libssusysinfo.so
%dir %{_libdir}/pkgconfig
%{_libdir}/pkgconfig/*.pc
