#!/usr/bin/python3
 
from serial import Serial
import sys
 
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
 
pld = quadro.encode('ascii')
 
n = p.write(pld)
print('Enviou %d bytes: %s' % (n, quadro))
resp = p.readline()
print('Recebeu: %s' % str(resp))

sys.exit(0)
