#sbs-git:slp/pkgs/c/cert-svc cert-svc 1.0.1 ad7eb7efcefb37b06017c69cb2fc44e6f7b6cab7
Name:    cert-svc-ui
Summary: Certification service
Version: 1.0.1
Release: 31
Group:   System/Libraries
License: SAMSUNG
Source0: %{name}-%{version}.tar.gz

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

%package test
Summary:  Certification service (tests)
Group:    System/Misc
Requires: %{name} = %{version}-%{release}

%description test
Certification service (tests)

%description
Certification service (UI gadget)

%prep
%setup -q

%define _ugdir /opt/ug

%build
cmake . -DCMAKE_INSTALL_PREFIX="%{_ugdir}" -DCMAKE_BUILD_TYPE="Release" -DPKGNAME="cert-svc1-ui"
VERBOSE=1 make

%install
rm -rf %{buildroot}
%make_install

%clean
rm -rf %{buildroot}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_ugdir}/lib/libmgr-cert-common.so
%{_ugdir}/lib/libmgr-cert-view.so
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so.*
%{_ugdir}/lib/libug-setting-manage-certificates-efl.so
%{_ugdir}/res/locale/*/LC_MESSAGES/*
%{_ugdir}/res/edje/ug-setting-manage-certificates-efl/ug-setting-manage-certificates-efl.edj
%{_ugdir}/res/images/ug-setting-manage-certificates-efl/ManageApplications.png
%{_ugdir}/res/images/ug-setting-manage-certificates-efl/ManageApplications_default.png
/opt/ug/lib/libug-cert-selection-ug-efl.so*

%files test
%defattr(-,root,root,-)
/usr/bin/cert-selection-ug-test*
