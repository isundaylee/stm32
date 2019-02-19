import os
import struct


MISO_FILE = os.path.expanduser('~/Desktop/MISO.csv')
MOSI_FILE = os.path.expanduser('~/Desktop/MOSI.csv')


def read_file(path):
    ts = []
    data = []

    with open(path) as f:
        for line in list(f)[1:]:
            _, tss, byte = line.split(',')
            ts.append(float(tss))
            data.append(int(byte, 16))

    return ts, bytes(data)


ts, miso = read_file(MISO_FILE)
_, mosi = read_file(MOSI_FILE)

n = len(ts)

i = 0

last_npp = None
rdp = None

while i < n:
    if (i >= 4) and (mosi[i-5:i] == b"\xBF\x03\x9F\x01\x19"):
        npp = struct.unpack('<H', miso[i+10:i+12])[0]
        len = struct.unpack('<H', miso[i+12:i+14])[0]

        if rdp is None:
            rdp = (0x1000 + npp - len - 6)
        else:
            # assert(rdp == last_npp)
            pass

        rdp += 6

        print("PKTCNT: %d" % miso[i])
        print("  Next Packet Pointer: 0x%04x" % npp)
        print("  Length             : 0x%04x (%d)" % (len, len))
        print("  Status             : 0x%02x 0x%02x" %
              (miso[i + 14], miso[i + 15]))

        i += 17
        start = i
        while (i + 1 < n) and (ts[i + 1] - ts[i] <= 5e-6):
            i += 1

        for c in range(start, i + 1):
            if miso[c:c+2] == b'\x0e\x02':
                print('0x%02x' % rdp)
            rdp = (rdp + 1) % 0x1000

        actual_read = i - start + 1

        # assert(actual_read == len + (len % 2))
        last_npp = npp

        i += 1
        continue

    if (i - 10 >= 0) and (mosi[i - 10] == 0x44)and (mosi[i - 8] == 0x45)and (mosi[i - 6] == 0x42)and (mosi[i - 4] == 0x43) and (mosi[i-2:i] == b'\x7A\x00'):
        txstl = mosi[i - 9]
        txsth = mosi[i - 7]
        wrptl = mosi[i - 5]
        wrpth = mosi[i - 3]

        txst = txstl + (txsth << 8)
        wrpt = wrptl + (wrpth << 8)

        assert((txst == 0x1000) and (wrpt == 0x1000))

        while (i + 1 < n) and (ts[i + 1] - ts[i] <= 5e-6):
            i += 1
        i += 1
        continue

    if last_npp is not None:
        if (mosi[i] == 0x3A):
            print(ts[i] * 1000)

    i += 1
