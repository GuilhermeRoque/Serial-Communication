#!/usr/bin/python3
 
from serial import Serial
import sys

import crc
 
try:
  porta = sys.argv[1]
  quadro = sys.argv[2]
except:
  print('Uso: %s porta_serial quadro' % sys.argv[0])
  sys.exit(0)
 
try:
  p = Serial(porta)
except Exception as e:
  print(e)
  sys.exit(0)
 
# pld = quadro.encode('ascii')

fcs = crc.CRC16(quadro)
msg = fcs.gen_crc()
msg_send = bytearray()
msg_send.append(0x7E)
msg_send.extend(msg)
msg_send.append(0x7E)
# print('Mensagem com FCS:', msg_send)

n = p.write(msg_send)
print('Enviou %d bytes: %s' % (n, msg_send))
resp = p.readline()
print('Recebeu: %s' % str(resp))

sys.exit(0)
