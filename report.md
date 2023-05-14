# PAGINADOR DE MEMÓRIA - RELATÓRIO

1. Termo de compromisso

Os membros do grupo afirmam que todo o código desenvolvido para este
trabalho é de autoria própria.  Exceto pelo material listado no item
3 deste relatório, os membros do grupo afirmam não ter copiado
material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço

Preencha as linhas abaixo com o nome e o e-mail dos integrantes do
grupo.  Substitua marcadores `XX` pela contribuição de cada membro
do grupo no desenvolvimento do trabalho (os valores devem somar
100%).

  * Thiago Pádua de Carvalho - paduathiago@ufmg.br 50%
  * Natan Ventura Menezes - natanvent@ufmg.br - 50%

3. Referências bibliográficas

THE OPEN GROUP. Ucontext - user context. 1997. Disponível em: https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html. Acesso em: 08 maio 2023.
UCLOUVAIN. Ucontext.h. 2003. Disponível em: https://sites.uclouvain.be/SystInfo/manpages/man7/ucontext.h.7posix.html. Acesso em: 08 maio 2023.
UNIVERSIDADE FEDERAL DE SÃO CARLOS (São Carlos). Signal.h. 2023. Disponível em: https://petbcc.ufscar.br/signal/. Acesso em: 10 maio 2023.
THE OPEN GROUP. Signal.h - signals. 2004. Disponível em: https://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html. Acesso em: 11 maio 2023.
KERRISK, Michael. Timer_create(2) — Linux manual page. 2018. Disponível em: https://man7.org/linux/man-pages/man2/timer_create.2.html. Acesso em: 13 maio 2023.
UNIVERSIDADE FEDERAL DE SÃO CARLOS (São Carlos). Time.h. 2023. Disponível em: https://petbcc.ufscar.br/time/. Acesso em: 14 maio 2023.

4. Estruturas de dados

  1. Descreva e justifique as estruturas de dados utilizadas para
 	gerência das threads de espaço do usuário (partes 1, 2 e 5).

Parte 1:
A estrutura dccthread foi criada com três atributos, sendo eles: name do tipo char, context do tipo ucontext_t e waiting_for do tipo pointeiro dccthread. Eles servem respectivamente para guardar o nome da thread, contexto dela e a thread que ela está esperando.

A thread gerente foi armazenada como uma variável do tipo ponteiro ucontext, visto que ela não necessitava de um nome ou espera de outra thread.
Foi criada uma variável em referência à thread que está sendo executada no momento, a variável ‘current_thread’ foi criada do tipo ponteiro dccthread_t.
Foi criada uma instância da variável dlist para armazenar a fila de threads prontas para execução. A estrutura foi utilizada para permitir remover threads em qualquer posição em uma boa complexidade. Além de ser rápida para acessar os elementos do começo e final da thread.

Também foi criada a estrutura timer_data que contém variáveis de sinal da biblioteca signal.h e timer_t da biblioteca timer.h.
Foi criado também uma instância preemp_timer da estrutura timer_data para guardar os dados de preempção.

Parte 2:
Para que a espera de threads fosse possível, foi necessário acrescentar um campo "waiting_for" no tipo abstrato de dados que define uma thread. Esse campo é um ponteiro para a thread a qual a atual está esperando. Dessa forma, é possível saber qual thread está aguardando por outra e, consequentemente, qual deve ser acordada quando a esperada terminar sua execução.

Parte 5:
Foi criado uma instância do tipo timer_data para suspender a thread atual. 

2. Descreva o mecanismo utilizado para sincronizar chamadas de dccthread_yield e disparos do temporizador (parte 4).

A parte 4 foca em evitar condições de corrida entre as threads do programa. Para tal, foi necessário utilizar uma instrução capaz de desabilitar e reabilitar o temporizador. O mecanismo utilizado foi a função sigprocmask, que desabilita o timer antes de executar a função dccthread_yield e o reabilita logo após. Dessa forma, é possível garantir que o ele não será disparado durante a execução, o que faria com que o contexto da thread fosse salvo de maneira incorreta durante a execução do programa, comprometendo os dados da biblioteca de threads. O funcionamento é similar ao de um lock mutex, porém, ao invés de bloquear a execução de outras threads, a função sigprocmask bloqueia o temporizador.

