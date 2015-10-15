Name:      org.tizen.ugmyfiletestapp
Version:   0.0.1
Release:   1
License:   Apache-2.0
Summary:   Hello EFL Application
Group:     Applications
Source0:   %{name}-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(elementary)
BuildRequires: gettext-devel
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gthread-2.0)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(ecore-imf)
BuildRequires: pkgconfig(ecore-x)
BuildRequires: pkgconfig(eina)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(ecore-file)
BuildRequires: pkgconfig(ecore-input)
BuildRequires: pkgconfig(capi-system-power)

%description
Platform Project

%define _pkgdir /opt/apps/org.tizen.ugmyfiletestapp
%define _bindir %{_pkgdir}/bin
%define _resdir %{_pkgdir}/res
%define _localedir %{_resdir}/locale
%define _manifestdir /opt/share/packages
%define _desktop_icondir /opt/share/icons/default/small

%prep
%setup -q

%build
cmake \
    -DCMAKE_INSTALL_PREFIX=%{_pkgdir} \
    -DPACKAGE_NAME=org.tizen.ugmyfiletestapp \
    -DBINDIR=%{_bindir} \
    -DLOCALEDIR=%{_localedir} \
    -DMANIFESTDIR=%{_manifestdir} \
    -DDESKTOP_ICONDIR=%{_desktop_icondir} \
    -DDESKTOP_ICON=%{name}.png \
    -DVERSION=%{version}

make %{?jobs:-j%jobs}

%install
%make_install

%post
ln -sf /usr/bin/ug-client /usr/ug/bin/myfile-efl


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_bindir}/*
%{_resdir}/*
%{_manifestdir}/%{name}.xml
%{_desktop_icondir}/%{name}.png
