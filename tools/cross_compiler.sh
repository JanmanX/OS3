#!/bin/bash

# Print all output
set -x
mkdir $HOME/src
export NJOBS=4

export PREFIX="$HOME/opt/cross"
export TARGET=x86_64-elf
export PATH="$PREFIX/bin:$PATH"

export GCC=gcc-7.2.0
export BINUTILS=binutils-2.29

#### BINUTILS
# Go to source directory
cd $HOME/src

# Check if archive is already downloaded
if [ ! -f ./${BINUTILS}.tar.bz2 ]
then
	wget http://ftp.gnu.org/gnu/binutils/${BINUTILS}.tar.bz2
fi

# Remove the old folder
rm ${BINUTILS} -rf

# Extract the archive
tar xf ${BINUTILS}.tar.bz2

rm build-binutils -rf
mkdir build-binutils
cd build-binutils
../${BINUTILS}/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make all -j$(NJOBS)
make install

#### GCC
# Go to ~/home/src
cd $HOME/src
# Check if archive is already downloaded
if [ ! -f ./${GCC}.tar.xz ]
then
	wget https://ftp.gnu.org/gnu/gcc/${GCC}/${GCC}.tar.xz
fi

# Remove the extracted directory
rm ${GCC} -rf

# Unzip the new archive
tar xf ${GCC}.tar.xz

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

cd ${GCC}
contrib/download_prerequisites
cd ..

rm build-gcc -rf
mkdir build-gcc
cd build-gcc
../${GCC}/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc -j$(NJOBS)
make all-target-libgcc -j$(NJOBS)
make install-gcc
make install-target-libgcc


# Messsage for user
echo 'Add to path: PATH="$HOME/opt/cross/bin:$PATH"'
