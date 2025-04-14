# Exercício 1

Imagine uma situação em que você tem um circuito capaz de ativar e desativar uma carga puramente resistiva com um microcontrolador. 
É necessário controlar a potência da carga ao longo do tempo, dependendo da situação, e a potência não pode ser alterada instantaneamente de 0% a 100%. 
Explique em palavras simples um método para obter esse efeito, considerando os aspectos de hardware e firmware do sistema. 
Se uma carga indutiva for usada em vez de uma carga resistiva, o mesmo método de disparo pode ser usado ou precauções especiais precisam ser tomadas?

## Resposta para o caso da carga resistiva
Ao utilizar uma carga resistiva, utilizaria a modulação por largura de pulso PWM. 
Para obter uma estabilização da potência o mais rápido possível, utilizaria o controlador PI por se tratar de um sistema de controle de potência de primeira ordem.

## Resposta para o caso da carga indutiva
Ao utilizar uma carga indutiva também utilizaria a modulação por largura de pulso PWM. 
Para cargas indutivas, o indutor se comporta armazenando energia e se contrapondo ao sentido da corrente, gerando um efeito chamado back-emf.
O back-emf pode causar grandes estragos, pois cria uma tensão reversa nos polos do indutor podendo comprometer todo o circuito.
Para resolver isso, é colocado um diodo em paralelo com o indutor do circuito ou em paralelo com a chave para evitar essa tensão reversa.
Esse diodo é chamado de Diodo de roda livre.
Em novos MOSFETs, o diodo de roda livre não é necessário pois o MOSFET já possui um diodo integrado, mas em grandes cargas é necessário montar um circuito snubber.
O circuito snubber serve também para atenuar as compensações da energia reativa nas chaves, o mais comum é o snubber RC.

