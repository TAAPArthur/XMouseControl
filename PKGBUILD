# Maintainer: Arthur Williams <taaparthur@gmail.com>


pkgname='xmouse-control'
pkgver='1.2.1'
_language='en-US'
pkgrel=1
pkgdesc='Control mouse from keyboard'

arch=('any')
license=('MIT')
depends=('xorg-server' 'libx11' )
md5sums=('SKIP')

source=("git://github.com/TAAPArthur/XMouseControl.git")
_srcDir="XMouseControl"

build(){
	cd "$_srcDir"
	make
}
package() {
  cd "$_srcDir"
  make DESTDIR="$pkgdir" install
}
