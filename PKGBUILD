# Maintainer: Arthur Williams <taaparthur@gmail.com>


pkgname='xmouse-control'
pkgver='1.0.0'
_language='en-US'
pkgrel=1
pkgdesc='Control mouse from keyboard'

arch=('any')
license=('MIT')
depends=('xorg-server' 'libx11' )
md5sums=('SKIP')

source=("git://github.com/TAAPArthur/XMouseControl.git")
_srcDir="XMouseControl"

package() {

  cd "$_srcDir"
  mkdir -p "$pkgdir/usr/bin/"
  mkdir -p "$pkgdir/usr/lib/$pkgname/"
  install -D -m 0755 "xmouse-control" "$pkgdir/usr/bin/"
}
