#!/bin/bash

# set up variable defaults
BUILD_AUTOTOOLS=0

if [ "$#" -le 0 ]; then
  echo "Usage: android_update_dependencies.sh <path>"
  exit 1
fi

START_PATH=$(pwd)
if [ -d "/Applications/Android Studio.app" ];
then
  export ANDROID_STUDIO_PATH="/Applications/Android Studio.app"
else
  echo "Android Studio not found in standard location, searching..."
  export ANDROID_STUDIO_PATH=`find -L /Applications -name "Android Studio.app"`
fi

# check arguments
for arg in "$@";
do
  if [ $arg = --build_autotools ]; then
    BUILD_AUTOTOOLS=1
  fi
done

mkdir -p $1
cd $1

# build and install autotools
if [ $BUILD_AUTOTOOLS -eq 1 ];
then
  # grab and build autoconf
  curl -O https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
  tar xzf autoconf-2.69.tar.gz
  cd autoconf-2.69
  ./configure
  make
  sudo make install
  cd ..

  # grab and build automake
  curl -O https://ftp.gnu.org/gnu/automake/automake-1.18.tar.gz
  tar xzf automake-1.18.tar.gz
  cd automake-1.18
  ./configure
  make
  sudo make install
  cd ..
fi

# dev environment
export NDK=$HOME/Library/Android/sdk/ndk/29.0.14206865
export HOST_TAG=darwin-x86_64
export MIN_SDK_VERSION=23

if [ ! -d "openssl-curl-android" ];
then
  git clone https://github.com/NewCreature/openssl-curl-android.git
fi
cd openssl-curl-android
git submodule update --init --recursive
./build.sh
