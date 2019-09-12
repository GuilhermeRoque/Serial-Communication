#!/usr/bin/python3

from serial import Serial
import sys

import crc

def add_escape(quadro):
    new_quadro = bytearray()
    for byte in quadro:
      if byte == 0x7e:
        new_quadro.append(0x7d)
        new_quadro.append(0x5e)
      elif byte == 0x7d:
        new_quadro.append(0x7d)
        new_quadro.append(0x5d)
      else:
        new_quadro.append(byte)

    return new_quadro

def get_controle(tipo, sequencia):
  if tipo == 'data':
    if sequencia:
      return 0b00001000
    else:
      return 0b00000000
  else:
    if sequencia:
      return 0b01001000
    else:
      return 0b01000000
  
seq_counter = 0
while (True):
    try:
      porta = sys.argv[1]
      ##quadro = sys.argv[2]
      print('----------------------------------------')
      quadro = input("Insira o quadro a ser enviado: ")
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
    msg = add_escape(msg)
    msg_send = bytearray()
    msg_send.append(0x7E)
    # msg_send.append(get_controle('data', seq_counter))
    msg_send.append(0x00)
    msg_send.extend(msg)
    msg_send.append(0x7E)
    # print('Mensagem com FCS:', msg_send)

    try:
        n = p.write(msg_send)
        msg_send_bytes = ":".join(hex(b)[2:] for b in msg_send)
        print('Enviou %d bytes: %s' % (n, msg_send_bytes))
        resp = p.readline()
        resp_bytes = ":".join(hex(b)[2:] for b in resp)
        print('Recebido: %s' % str(resp_bytes))
        seq_counter = not seq_counter
    except Exception as e:
        print(str(e))

sys.exit(0)
