# Maintainer: Arthur Williams <taaparthur@gmail.com>


pkgname='xmouse-control'
pkgver='1.3.0'
_language='en-US'
pkgrel=1
pkgdesc='Control mouse from keyboard'

arch=('any')
license=('MIT')
depends=(libx11 libxtst libxi)
md5sums=('SKIP')

source=("git://github.com/TAAPArthur/XMouseControl.git")
_srcDir="XMouseControl"

build(){
	make -C "$_srcDir"
}

package() {
  make -C "$_srcDir" DESTDIR="$pkgdir" install
}
