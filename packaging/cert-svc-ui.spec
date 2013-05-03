#sbs-git:slp/pkgs/c/cert-svc cert-svc 1.0.1 ad7eb7efcefb37b06017c69cb2fc44e6f7b6cab7
Name:    cert-svc-ui
Summary: Certification service
Version: 1.0.1
Release: 47
Group:   System/Libraries
License: SAMSUNG
Source0: %{name}-%{version}.tar.gz
Source1: %{name}.manifest

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
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(glib-2.0)

%description
Certification service

%prep
%setup -q

%define _ugdir /usr/ug

%build
%{!?build_type:%define build_type "Release"}
cmake . -DCMAKE_INSTALL_PREFIX="%{_ugdir}" -DCMAKE_BUILD_TYPE=%{build_type} -DPKGNAME="cert-svc1-ui"
#VERBOSE=1 make
make

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/%{name}
%make_install
install -D %{SOURCE1} %{buildroot}%{_datadir}/%{name}.manifest

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%manifest %{_datadir}/%{name}.manifest
/etc/smack/accesses2.d/ug.%{name}.include
%{_ugdir}/lib/libmgr-cert-view.so
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so.*
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so
%{_ugdir}/lib/libug-cert-selection-ug-efl.so*
%{_ugdir}/res/locale/*/LC_MESSAGES/*
%{_datadir}/license/%{name}
