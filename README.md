### **Projeto de Protocolos**
#### **Definição do protocolo**

- Protocolo de enlace ponto-a-ponto, para camada física do tipo UART;

- Encapsulamento de mensagens com até 1024 bytes:

			class Framming

- Recepção de mensagens livres de erros:

			*CRC
			Checksum

- Garantia de entrega:

			 ARQ (pedido de retransmissão automática):
				*Pare-espere
				Volta-N
				Retransmissão Seletiva

- Controle de acesso ao meio - MAC:

			 CSMA/CA
			 Aloha
			 Mestre/escravo
			 TDMA

- Conectado (estabelecimento de sessão):

			2 vias
			3 vias

>Esquemático do protocolo .
>
![](https://github.com/viniciusluzsouza/ptc/blob/master/Protocol.png)
