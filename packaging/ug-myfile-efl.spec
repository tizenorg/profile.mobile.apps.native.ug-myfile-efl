
Name:       ug-myfile-efl
Summary:    ug
Version:    0.3.42
Release:    1
Group:      TO_BE/FILLED_IN
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

%if "%{?tizen_profile_name}" == "wearable" || "%{?tizen_profile_name}" == "tv"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(ui-gadget-1)
BuildRequires:  pkgconfig(libmedia-service)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(icu-i18n)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(capi-system-runtime-info)
BuildRequires:  pkgconfig(capi-content-media-content)
BuildRequires:  pkgconfig(notification)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(capi-content-mime-type)
BuildRequires:  pkgconfig(capi-media-metadata-extractor)
BuildRequires:  pkgconfig(capi-media-image-util)
BuildRequires:  pkgconfig(media-thumbnail)
BuildRequires:  pkgconfig(storage)
BuildRequires:  pkgconfig(capi-media-sound-manager)
BuildRequires:  pkgconfig(capi-media-player)
BuildRequires:  pkgconfig(capi-system-system-settings)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(accounts-svc)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(libtzplatform-config)

BuildRequires:  cmake
BuildRequires:  edje-bin
BuildRequires:  embryo-bin
BuildRequires:  gettext-devel

BuildRequires:  boost-devel
BuildRequires:  boost-thread
BuildRequires:  boost-system
BuildRequires:  boost-filesystem

%description
Myfile Application v1.0.
%define _smack_domain %{name}


%description
Description: myfile UG

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX="%{TZ_SYS_RO_UG}" \
	-DCMAKE_INSTALL_DATA_DIR="%{DATADIR}" \
	-DTZ_SYS_RO_PACKAGES=%{TZ_SYS_RO_PACKAGES} \
	-DTZ_SYS_RO_ICONS=%{TZ_SYS_RO_ICONS}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}

%make_install

%post
mkdir -p /usr/ug/bin/
ln -sf /usr/bin/ug-client %{TZ_SYS_RO_UG}/bin/myfile-efl

%postun

%files 
%manifest ug-myfile-efl.manifest
%defattr(-,root,root,-)
%{TZ_SYS_RO_UG}/lib/libug-myfile-efl.so*
%{TZ_SYS_RO_UG}/res/*
%{TZ_SYS_RO_PACKAGES}/ug-myfile-efl.xml
%{TZ_SYS_RO_ICONS}/default/small/ug-myfile-efl.png
