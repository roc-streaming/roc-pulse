%undefine       _disable_source_fetch
%define         build_timestamp %(date +"%Y%m%d")
%define         pkgname pulseaudio-module-roc
%define         pkgname_git roc-pulse
%define         major 0.1
%define         minor 5
%define         branch main
%define         arch %(uname -p)
%define         pulse_version %(pulseaudio --version | egrep -o '[0-9.]*')
%define         pulse_url https://github.com/pulseaudio/pulseaudio/archive/v%{pulse_version}
Name:           %{pkgname}
Version:        %{major}.%{minor}~git_%{branch}
#Release:        1%{?dist}
Release:        %{build_timestamp}
Group:          System/Sound Daemons
Summary:        Roc module for Pulseaudio
Source:         https://github.com/roc-streaming/%{pkgname_git}/archive/refs/heads/%{branch}.zip
URL:            https://roc-streaming.org
License:        PL-2.0-or-later AND LGPL-2.1-or-later
Provides:       pulseaudio-module-%{name} = %{version}
Requires:       %{name} = %{version}
Obsoletes:      pulseaudio-module-%{name} < %{version}
BuildRequires:  libroc-devel

%description
pulseaudio is a networked sound server for Linux and other Unix like
operating systems and Microsoft Windows. It is intended to be an
improved drop-in replacement for the Enlightened Sound Daemon (ESOUND).

This package provides the Roc module for PulseAudio sound server

%prep
%setup -n %{pkgname_git}-%{branch}
mkdir -p 3rdparty
[ -f %{_sourcedir}/pulseaudio-%{pulse_version}.tar.xz ] \
  || wget -vcP %{_sourcedir}/ \
      --content-disposition \
      --trust-server-name %{pulse_url}
tar -C 3rdparty -xvf %{_sourcedir}/pulseaudio-%{pulse_version}.tar.xz \
  pulseaudio-%{pulse_version}/src/pulsecore

%build
%cmake \
      -DCMAKE_SHARED_LINKER_FLAGS="%{?build_ldflags} -Wl,--as-needed -Wl,-z,now" \
      -DDOWNLOAD_ROC=OFF \
      -DDOWNLOAD_PULSEAUDIO=OFF \
      -DDOWNLOAD_LIBTOOL=OFF \
      -DPULSEAUDIO_DIR=3rdparty/pulseaudio-%{pulse_version} \
      -DPULSEAUDIO_VERSION=%{pulse_version}
%cmake_build

%install
mkdir -p %{buildroot}/%{_libdir}/pulse-%{pulse_version}/modules
for f in bin/*.so; do
  install ${f} %{buildroot}/%{_libdir}/pulse-%{pulse_version}/modules/
done

%files -n %{pkgname}
%defattr(755,root,root)
%attr(644,root,root) %license LICENSE
%{_libdir}/pulse-%{pulse_version}/modules/*

%changelog
* Tue Jun 28 2022 twojstaryzdomu - 0.1.5_git
- Initial release
