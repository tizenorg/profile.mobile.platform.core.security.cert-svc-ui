%bcond_with wayland
%bcond_with x

Name:    cert-svc-ui
Summary: Certification service
Version: 1.0.1
Release: 0
Group:   System/Libraries
License: Apache-2.0
Source0: %{name}-%{version}.tar.gz
Source1001: %{name}.manifest

Requires(post):   /sbin/ldconfig
Requires(postun): /sbin/ldconfig

BuildRequires: cmake
BuildRequires: gettext-tools
BuildRequires: edje-tools
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(ail)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(appsvc)
BuildRequires: pkgconfig(cert-svc-vcore)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(efl-assist)
BuildRequires: pkgconfig(libtzplatform-config)
%if %{with x}
BuildRequires:  pkgconfig(ecore-x)
%endif

Requires: libtzplatform-config

%description
Certification service

%prep
%setup -q
cp %{SOURCE1001} .

%define _ugdir /usr/ug

%build
%{!?build_type:%define build_type "Release"}

%cmake . \
    -DCMAKE_INSTALL_PREFIX="%{_ugdir}" -DCMAKE_BUILD_TYPE=%{build_type} \
    -DPKGNAME="cert-svc1-ui" \
    -DTZ_SYS_BIN=%TZ_SYS_BIN \
%if %{with wayland}
    -DWAYLAND_SUPPORT=On \
%else
    -DWAYLAND_SUPPORT=Off \
%endif
%if %{with x}
    -DX_SUPPORT=On \
%else
    -DX_SUPPORT=Off \
%endif
    #eol

#VERBOSE=1 make
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
%make_install

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
/etc/smack/accesses2.d/ug.%{name}.include
%{_ugdir}/lib/libmgr-cert-view.so
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so.*
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so
%{_ugdir}/lib/libug-cert-selection-ug-efl.so*
%{_ugdir}/res/locale/*/LC_MESSAGES/*
%license %{_datadir}/license/%{name}
