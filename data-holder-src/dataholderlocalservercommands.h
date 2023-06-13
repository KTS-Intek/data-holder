#ifndef DATAHOLDERLOCALSERVERCOMMANDS_H
#define DATAHOLDERLOCALSERVERCOMMANDS_H

#define DATAHOLER_RESULT_OK         1
#define DATAHOLER_RESULT_ERROR       2

//RegularLocalSocket will be used on the client side, so do not change the next commands
#define DATAHOLDER_GET_INFO        1 //GET A CLIENT NAME
#define DATAHOLDER_PING             9 //FROM CLIENT TO SERVER
#define DATAHOLDER_PING_2_SERV      16 //FROM SERVER TO CLIENT


#define DATAHOLDER_ADD_POLLDATA     30 //A CLIENT SENDS DATA
#define DATAHOLDER_GET_POLLDATA     31 //A CLIENT ASKS FOR DATA
#define DATAHOLDER_GET_POLLDATA_EXT 32 //A CLIENT ASKS FOR DATA
#define DATAHOLDER_ADD_NANSWER_EVNT 33 //no answer from a meter
#define DATAHOLDER_ADD_NMODEM_EVNT  34 //no answer from a modem, bad channel
#define DATAHOLDER_RELOAD_DBFORCED  35 //data-holder asks to reload data from the database



#endif // DATAHOLDERLOCALSERVERCOMMANDS_H
