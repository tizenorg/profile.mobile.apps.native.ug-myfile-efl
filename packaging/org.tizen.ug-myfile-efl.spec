Name:       org.tizen.ug-myfile-efl
#VCS_FROM:   profile/mobile/apps/native/ug-myfile-efl#c263a12ba1af84cbfbad5bdc6f0d3e2c796cf3ae
#RS_Ver:    20160616_1 
Summary:    ug-myfile-efl
Version:    1.0.0
Release:    1
Group:      Applications/Multimedia Applications
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz

ExcludeArch:  aarch64 x86_64
BuildRequires:  pkgconfig(libtzplatform-config)
Requires(post):  /usr/bin/tpk-backend

%define internal_name org.tizen.ug-myfile-efl
%define preload_tpk_path %{TZ_SYS_RO_APP}/.preload-tpk 

%ifarch i386 i486 i586 i686 x86_64
%define target i386
%else
%ifarch arm armv7l aarch64
%define target arm
%else
%define target noarch
%endif
%endif

%description
#VCS_FROM:   profile/mobile/apps/native/ug-myfile-efl#c263a12ba1af84cbfbad5bdc6f0d3e2c796cf3ae
This is a container package which have preload TPK files

%prep
%setup -q

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/%{preload_tpk_path}
install %{internal_name}-%{version}-%{target}.tpk %{buildroot}/%{preload_tpk_path}/

%post

%files
%defattr(-,root,root,-)
%{preload_tpk_path}/*
