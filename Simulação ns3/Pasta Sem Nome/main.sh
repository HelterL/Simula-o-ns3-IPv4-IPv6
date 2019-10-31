#!/bin/bash

main(){
 templete=$1
 ptc=$2
 nPacket=$3
#payloadSize=$4
 CONTADOR=0
 chavip4=IP10.1.2.1.49153
 chavip6=request

 while [  $CONTADOR -lt 30 ]; do
   
   case "$ptc" in
    IPv4|ipv4)
      ./waf --run "$templete --nPacket=$nPacket"
      taxaErro $chavip4 ping4-3-0.pcap $nPacket
      latencia ping4-0-0.pcap ping4-3-0.pcap $chavip4
    ;;
    IPv6|ipv6)
        ./waf --run "$templete --nPacket=$nPacket"
        taxaErro $chavip6 ping6-3-0.pcap $nPacket
        latencia ping6-0-0.pcap ping6-3-0.pcap $chavip6
    ;;
    *)
        echo "Opção inválida"
    ;;
    esac

   let CONTADOR=CONTADOR+1; 
 done
 
}

taxaErro(){
  echo "taxa de erro"
  #gera um arquivo organizado para o tratamento
  c=$(tcpdump -nn -tt -r $2 |
 
  # apaga todas os espaços em branco
  sed 's/ //g'  |

  #conta quantas vezes o ip aparece
  grep -c $1)
  echo "scale=1;(($3-($c/2))/$3) * 100" | bc >> tests/taxaErro$1.txt
}

latencia(){
echo "latencia"

 chav=$3
 tcpdump -nn -tt -r $1| sed 's/ //g'| grep $chav| cut -d'I' -f 1 >> p1.txt
 
 tcpdump -nn -tt -r $2|sed 's/ //g'| grep $chav|
 cut -d'I' -f 1 >> p2.txt
 
paste -d - p2.txt p1.txt | bc >> latencia.txt | sed -i 's/^[.]/0./' latencia.txt

mLatencia=$(R -e "a<-read.table('latencia.txt', header=FALSE)[,1]; media <-c(a); mean(a)")
echo $mLatencia > medialatencia.txt
mediafinal=$(cat medialatencia.txt | cut -d';' -f3 | cut -d' ' -f4)
echo $mediafinal >> tests/mediafinallatencia$3.txt


rm medialatencia.txt
rm p2.txt
rm p1.txt

}
init(){
  temple1=$1
  #temple2=$3
  ptc1=$2
  #ptc2=$4
  nPack=$3
  main $temple1 $ptc1 $nPack
  #main $temple2 $ptc2 $nPack
}

#recebe template 1 e seu protocolo tempelete 2 e seu ptocolo e o numero de pacotes
init $1 $2 $3 $4 $5
