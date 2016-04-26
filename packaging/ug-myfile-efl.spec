
Name:       org.tizen.ug-myfile-efl
Summary:    ug-myfile-efl
Version:    0.3.42
Release:    1
Group:      Applications/Multimedia Applications
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

%define PREFIX    	 %{TZ_SYS_RO_APP}/%{name}
%define MANIFESTDIR      %{TZ_SYS_RO_PACKAGES}
%define ICONDIR          %{TZ_SYS_RO_ICONS}/default/small

%define RESDIR           %{PREFIX}/res
%define EDJDIR           %{RESDIR}/edje
%define IMGDIR           %{EDJDIR}/images
%define BINDIR           %{PREFIX}/bin
%define LIBDIR           %{PREFIX}/lib
%define LOCALEDIR        %{RESDIR}/locale
%define IMGICONDIR       %{EDJDIR}/icons

%prep
%setup -q

%build
%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . \
    -DPREFIX=%{PREFIX}   \
    -DPKGDIR=%{name}     \
    -DIMGDIR=%{IMGDIR}   \
    -DEDJDIR=%{EDJDIR}   \
    -DPKGNAME=%{name}    \
    -DBINDIR=%{BINDIR}   \
    -DMANIFESTDIR=%{MANIFESTDIR}   \
    -DEDJIMGDIR=%{EDJIMGDIR}   \
    -DLIBDIR=%{LIBDIR}   \
    -DICONDIR=%{ICONDIR}   \
    -DLOCALEDIR=%{LOCALEDIR} \
	-DIMGICONDIR=%{IMGICONDIR}\
	-DRESDIR=%{RESDIR}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{LIBDIR}

%post
GOPTION="-g 6514"

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%dir
%{LIBDIR}
%{BINDIR}/*
%{MANIFESTDIR}/*.xml
%{ICONDIR}/*
%{RESDIR}/*
%{IMGICONDIR}/*
%{LOCALEDIR}/*
