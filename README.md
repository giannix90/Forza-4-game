Forza 4
=======

This is an simple implementation of Forza 4 game, you must run the server
that permit clients to connect and play.The server accept the request by 
the cients and create a connection between two client wich want play.


Usage
------

$ make compile //to compile

$ make install //to install

$ forza_server [my_ip_address] [my_port_number]

$ forza_client [ip_address_of_the_server] [port_number_of_the_server]


Example
-------
If i wont run the game in local machine with loopback-ip i need to open at least 3 terminal:

(Terminal 1)
$ forza_server 127.0.0.1 1234

(Terminal 2)
$ forza_client 127.0.0.1 1234

(Terminal 3)
$ forza_client 127.0.0.1 1234


(Terminal 2)

> Inserisci il tuo nome: gianni

> Inserisci la porta UDP di ascolto: 5555

> !who

> [list of player]

>!connect [player_name]

#!insert [column]


Thi is the command in the game:
 * !help --> show the help
 * !disconnect --> disconnect client
 * !quit --> close the connection to the server
 * !who --> show the list of client connected to the server
 * !insert [column] --> insert the token in the column [a,b,c.....] (valid only in your turn)
 * !show_map --> show the map
 * !exit --> exit to the game
 * !connect [player] --> connect with the player



 Developer
-----------
Gianni Pollina (gianni.pollina1@gmail.com)