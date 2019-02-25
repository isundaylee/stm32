pushd $PICOTCP_ROOT

make clean

make \
  CROSS_COMPILE=arm-none-eabi- \
  ARCH='cortexm4-softfloat' \
  \
  DHCP_SERVER=0 \
  DNS_SD=0 \
  HTTP_CLIENT=0 \
  HTTP_SERVER=0 \
  IPV6=0 \
  MCAST=0 \
  MDNS=0 \
  TFTP=0 \
  PPP=0 \
  \
  -j

popd

mkdir -p picotcp
mkdir -p picotcp/lib

rm -f picotcp/lib
rm -rf picotcp/include

cp $PICOTCP_ROOT/build/lib/libpicotcp.a picotcp/lib
cp -r $PICOTCP_ROOT/build/include picotcp/

tree picotcp
